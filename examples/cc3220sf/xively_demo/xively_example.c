/*
 *   Copyright (C) 2015-2016 Texas Instruments Incorporated
 *
 *   All rights reserved. Property of Texas Instruments Incorporated.
 *   Restricted rights to use, duplicate or disclose this code are
 *   granted through contract.
 *
 *   The program may not be used without the written permission of
 *   Texas Instruments Incorporated or against the terms and conditions
 *   stipulated in the agreement under which this program has been supplied,
 *   and under no circumstances can it be used with non-TI connectivity device.
 *
 ******************************************************************************/

/*
 *  Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 *  This is part of the Xively C Client library,
 *  it is licensed under the BSD 3-Clause license.
 *
 ******************************************************************************/

/* Example/Board Header files */
#include "xively_example.h"

#include <ti/drivers/net/wifi/simplelink.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/I2C.h>

#include <stdio.h>
#include <board.h>
#include <pin.h>
#include <gpio.h>
#include <sched.h>

/* driverlib Header files */
#include <ti/devices/cc32xx/inc/hw_types.h>
#include <ti/devices/cc32xx/inc/hw_memmap.h>
#include <ti/devices/cc32xx/driverlib/rom.h>
#include <ti/devices/cc32xx/driverlib/rom_map.h>
#include <ti/devices/cc32xx/driverlib/prcm.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>

#include <simplelinkWifi/network_if.h>

#include "xively.h"
#include "xi_bsp_rng.h"
#include "xi_bsp_time.h"
#include "config_file.h"

#ifdef DEBUG_WOLFSSL
extern void wolfSSL_Debugging_ON();
#endif /* DEBUG_WOLFSSL */

/****************************************************************************
                      LOCAL FUNCTION PROTOTYPES
                       (In order of appearance)
****************************************************************************/
static void InitializeAppVariables();
static void UpdateLEDProgressIndicators( UArg arg0 );
static int8_t validateWifiConfigurationVariables( const char* wifi_ssid,
                                                  const char* wifi_password,
                                                  const char* wifi_security_type );
static void ConnectToXively();
static xi_state_t subscribe_to_topic( xi_context_handle_t context_handle,
                                      const char* channel,
                                      xi_user_subscription_callback_t* callback,
                                      void* user_data );

static void parseCredentialsFromConfigFile();
static int8_t parseKeyValue( config_entry_t* config_file_context,
                             const char* key,
                             const uint8_t is_required,
                             char** out_value );

static int8_t validateWifiConfigurationVariables( const char* wifi_ssid,
                                                  const char* wifi_password,
                                                  const char* wifi_security_type );

static int8_t
validateXivelyCredentialBufferRequirement( const char* config_key, const char* value );

static int8_t mapWifiSecurityTypeStringToInt( const char* security_type );

/****************************************************************************
               Xively Application State Machine Callbacks
****************************************************************************/

/* Invoked when a xi_connect status change occurs. */
void on_connected( xi_context_handle_t in_context_handle, void* data, xi_state_t state );

/* device topic subscription callbacks used for LED control over the Xively service. */
void on_led_topic( xi_context_handle_t in_context_handle,
                   xi_sub_call_type_t call_type,
                   const xi_sub_call_params_t* const params,
                   xi_state_t state,
                   void* user_data );

/* interrupt handler register to be called whenever a user bushes either of the buttons on
 * the
 * sides of the CC3220 device.
 */
void button_interrupt_handler( unsigned int index );

/* periodically called by the Xively event system to publish temperature sensor data
 * to a Xively topic.
 */
void send_temperature( const xi_context_handle_t context_handle,
                       const xi_timed_task_handle_t timed_task_handle,
                       void* user_data );

static signed char hexConvertASCIINibble( signed char nibble );
static signed char
hexConvertByteASCII( signed char msb, signed char lsb, signed char* val );
static signed char FixWEPKey( char* key, unsigned char keyLen );

/****************************************************************************
                      GLOBAL VARIABLES
****************************************************************************/
pthread_t gProgressIndicatorThread = ( pthread_t )NULL;
pthread_t gProvisioningThread      = ( pthread_t )NULL;
pthread_t gSimpleLinkThread        = ( pthread_t )NULL;
Application_CB gApplicationControlBlock;


/****************************************************************************
         DEFINES FOR DYNAMIC CONFIGURATION OF XIVELY APPLICATION
****************************************************************************/

/* String template for dynamically constructing the various Xively topics
 * that are used for this application. */
#define XIVELY_TOPIC_LEN ( 128 )
#define XIVELY_TOPIC_FORMAT "xi/blue/v1/%s/d/%s/%s"

/* Names of the channels used by this application.  Channels
 * are suffixes to the full topic names. */
#define XIVELY_TEMPERATURE_CHANNEL_NAME "Temperature"
#define XIVELY_GREEN_LED_CHANNEL_NAME "Green LED"
#define XIVELY_ORANGE_LED_CHANNEL_NAME "Orange LED"
#define XIVELY_RED_LED_CHANNEL_NAME "Red LED"
#define XIVELY_BUTTON0_CHANNEL_NAME "Button SW2"
#define XIVELY_BUTTON1_CHANNEL_NAME "Button SW3"

/* Some defaults for the optional connection address configuration values */
#define XIVELY_DEFAULT_BROKER "broker.xively.com"
#define XIVELY_DEFAULT_PORT ( 8883 )

/* Xively Configuration File Key Names. These will be used to lookup key/value pairs
 * for Xively credentials and WiFI credentials from a file stored in the device's
 * flash file system */
#define WIFI_SEC_TYPE_KEY "wifi_security_type"
#define WIFI_SSID_KEY "wifi_ssid"
#define WIFI_PASSWORD_KEY "wifi_password"
#define XIVELY_HOST_KEY "hostname"
#define XIVELY_PORT_KEY "port"
#define XIVELY_ACNT_KEY "account_id"
#define XIVELY_DEV_KEY "device_id"
#define XIVELY_PWD_KEY "password"

/* Used to detect if the user has configured their xively_config.txt file
 * on the device before the execution of the application */
#define XIVELY_DEFAULT_CONFIG_FIELD_VALUE "<PASTE_XIVELY_CREDENTIAL_HERE>"
#define WIFI_DEFAULT_CONFIG_FIELD_VALUE "<ENTER_WIFI_CREDENTIAL_HERE>"

/* Required for push button interrupt handlers - there is no space for user argument so we
 * can't pass the xively's context handler as a function argument. */
xi_context_handle_t gXivelyContextHandle = -1;

/* Used to store the temperature timed task handles. */
xi_timed_task_handle_t gTemperatureTaskHandle = -1;

/* Used by onLed handler function to differentiate LED's */
const static uint32_t gGreenLed  = Board_LED2;
const static uint32_t gOrangeLed = Board_LED1;
const static uint32_t gRedLed    = Board_LED0;


/*****************************************************************************
                 Local Functions
*****************************************************************************/

/* @brief Clears out the Application Control Block which is used to track
 * configuration and connection status.  Initializes semaphores used
 * by the Provisioning System to notify the main applicatoin when a wifi
 * connection has been established.
 */
static void InitializeAppVariables()
{
    /* Clear the state of the control block */
    gApplicationControlBlock.gatewayIP               = 0;
    gApplicationControlBlock.status                  = 0;
    gApplicationControlBlock.xivelyConnectionStatus  = XI_CONNECTION_STATE_CLOSED;
    gApplicationControlBlock.button0State            = 0;
    gApplicationControlBlock.button1State            = 0;
    gApplicationControlBlock.desiredWifiSecurityType = 0;

    memset( gApplicationControlBlock.desiredWifiSSID, '\0',
            sizeof( gApplicationControlBlock.desiredWifiSSID ) );

    memset( gApplicationControlBlock.desiredWifiKey, '\0',
            sizeof( gApplicationControlBlock.desiredWifiKey ) );

    memset( gApplicationControlBlock.connectionSSID, '\0',
            sizeof( gApplicationControlBlock.connectionSSID ) );

    /* Clear the memory to store the WiFi SSID */
    memset( gApplicationControlBlock.connectionSSID, 0,
            sizeof( gApplicationControlBlock.connectionSSID ) );
    memset( gApplicationControlBlock.connectionBSSID, 0,
            sizeof( gApplicationControlBlock.connectionBSSID ) );

    sem_init( &gApplicationControlBlock.wifiConnectedSem, 0, 0 );
}

static void UpdateLEDProgressIndicators( UArg arg0 )
{
    static uint8_t toggle = Board_LED_OFF;

    if ( Board_LED_OFF == toggle )
    {
        toggle = Board_LED_ON;
    }
    else
    {
        toggle = Board_LED_OFF;
    }

    switch ( gApplicationControlBlock.initializationState )
    {
        case InitializationState_ReadingCredentials:
            GPIO_write( Board_LED2, toggle );
            break;
        case InitializationState_ConnectingToWifi:
            GPIO_write( Board_LED2, Board_LED_ON );
            GPIO_write( Board_LED1, toggle );
            break;
        case InitializationState_ConnectingToXively:
            GPIO_write( Board_LED2, Board_LED_ON );
            GPIO_write( Board_LED1, Board_LED_ON );
            GPIO_write( Board_LED0, toggle );
            break;

        case InitializationState_SubscribingToTopics:
            GPIO_write( Board_LED2, toggle );
            GPIO_write( Board_LED1, toggle );
            GPIO_write( Board_LED0, toggle );
            break;

        case InitializationState_Connected:
            /* LEDs are now controlled by topics */
            break;
    }
}


/*****************************************************************************
                 Main Functions
*****************************************************************************/

/* @brief the main thread of the application.
 *
 * 1. Configures the GPIO, console logging and I2C subsystems for LED control, temperature
 * sensor readings and button interrupt handling.
 * 2. Reads configuration properites from a file in the flash file system for the
 * configuration
 * of simplelink wifi and to parse the user's Xively credentials.
 * 3. Starts a Simplelink provisioning task using a standard TI provisioning example as a
 * basis.
 * 4. Connects to Xively.
 * 5. Upon connection, allows the control of LEDs from the Xively Product launcher,
 * publishes button state changes when buttons are pressed on the sides of the device,
 * and periodically (every 5 seconds) publishes the temperature read from the device's on
 * board temperature sensor.
 */
void* xivelyExampleThread( void* arg )
{
    pthread_attr_t pAttrs_spawn;
    struct sched_param priParam;

    /* For WLAN Access */
    SPI_init();

    /* For LED Access & Button Support */
    GPIO_init();

    /* For Temperature Sensor */
    I2C_init();

    /* init Terminal */
    InitTerm();

/* Enable WolfSSL Debugging */
#ifdef DEBUB_WOLFSSL
    wolfSSL_Debugging_ON();
#endif /* DEBUG_WOLFSSL */

    /* Init Variables and Control Blocks */
    InitializeAppVariables();

    /* Print Application Name */
    Report( "\n\n\n\r" );
    Report( "\t\t =================================================\n\r" );
    Report( "\t\t      %s Ver. %s      \n\r", APPLICATION_NAME, APPLICATION_VERSION );
    Report( "\t\t =================================================\n\r" );
    Report( "\n\n\n\r" );

    /* Switch off all LEDs on board, for a nice clean state */
    GPIO_write( Board_LED0, Board_LED_OFF );
    GPIO_write( Board_LED1, Board_LED_OFF );
    GPIO_write( Board_LED2, Board_LED_OFF );

    /* Add Interrupt Callbacks for the board's two side buttons */
    /* the application will use these interrupts to publish events */
    /* to the xively service on the corresponding button topic */
    GPIO_setCallback( Board_BUTTON0, ( GPIO_CallbackFxn )button_interrupt_handler );
    GPIO_enableInt( Board_BUTTON0 );

    GPIO_setCallback( Board_BUTTON1, ( GPIO_CallbackFxn )button_interrupt_handler );
    GPIO_enableInt( Board_BUTTON1 );

    /* create a periodic Clock for updating LED button states depending on
     * the current initialization state.  If an error occurs during
     * config file parsing, wifi provisioning or Xively Connection,
     * then the LED button states will let the user knows which stage something
     * went wrong. */
    Clock_Struct clk0Struct;
    Clock_Params clkParams;
    Clock_Params_init( &clkParams );
    clkParams.period    = 500; // 1 second
    clkParams.startFlag = TRUE;

    /* Construct a periodic Clock Instance */
    Clock_construct( &clk0Struct, ( Clock_FuncPtr )UpdateLEDProgressIndicators,
                     clkParams.period, &clkParams );

    /* create the simplelink task */
    /* Note: according to documentation, simplelink needs to run in it's own RTOS task */
    pthread_attr_init( &pAttrs_spawn );
    priParam.sched_priority = SPAWN_TASK_PRIORITY;

    long retval;
    retval = pthread_attr_setschedparam( &pAttrs_spawn, &priParam );
    retval |= pthread_attr_setstacksize( &pAttrs_spawn, TASK_STACK_SIZE );
    retval = pthread_create( &gSimpleLinkThread, &pAttrs_spawn, sl_Task, NULL );

    if ( retval )
    {
        Report( "Simplelink Task Creation pthread create returned an error: %d. ",
                retval );
        Report( "Looping forever\n\r" );
        while ( 1 )
            ;
    }

    /* start simplelink here so we can use the simplelink file API */
    uint32_t simpleLinkMode = sl_Start( NULL, NULL, NULL );

    /* Parse Xively and Wifi Credentials from config file on flash file system */
    /* This data will be stored in the Application Control Block */
    gApplicationControlBlock.initializationState = InitializationState_ReadingCredentials;

    parseCredentialsFromConfigFile();
    while ( 1 == 1 )
    {
        /* Attempt to connect to the configured WiFi network */
        gApplicationControlBlock.initializationState = InitializationState_ConnectingToWifi;

        /* Reset The state of the machine                                         */
        Network_IF_ResetMCUStateMachine();

        /* Start the driver                                                       */
        retval = Network_IF_InitDriver(simpleLinkMode, ROLE_STA,
                                       Callback_ConnectedToWiFi,
                                       Callback_LostWiFiConnection );
        if (retval < 0)
        {
           Report("Failed to start SimpleLink Device\n\r", retval);
           while( 1 )
               ;
        }

        /* Initialize AP security params                                          */
        SlWlanSecParams_t securityParams = { 0 };
        securityParams.Key = (signed char*)gApplicationControlBlock.desiredWifiKey;
        securityParams.KeyLen = strlen(gApplicationControlBlock.desiredWifiKey);
        securityParams.Type = gApplicationControlBlock.desiredWifiSecurityType;

        Report("Attempting to connect to WiFi SSID: %s\n", gApplicationControlBlock.desiredWifiSSID);
        Report("securityParams WiFi Type:    %d\n", securityParams.Type);

        /* Connect to the Access Point                                            */
        retval = Network_IF_ConnectAP(gApplicationControlBlock.desiredWifiSSID, securityParams);
        if (retval < 0)
        {
           Report("Connection to an AP failed\n\r");
           Report( "Looping forever\n\r" );
           while ( 1 )
               ;
        }

        gApplicationControlBlock.initializationState = InitializationState_ConnectingToXively;

        /* At this point the device has a wifi address. */
        /* Let's kick off our connection to Xively! */

        /* Wait for the Provisioning task to say that we have an IP address */
        if ( 0 > sem_wait( &gApplicationControlBlock.wifiConnectedSem ) )
        {
            Report( "Semaphore returned error when waiting for Wifi Provisioning Task.\n\r" );
            Report( "Looping forever\n\r" );
            while ( 1 )
                ;
        }

        /* Xively Client Connect Code */
        ConnectToXively();

        /* start processing xively events */
        xi_events_process_blocking();

        /* when the event dispatcher is stopped, destroy the context */
        xi_delete_context( gXivelyContextHandle );
        gXivelyContextHandle = XI_INVALID_CONTEXT_HANDLE;

        /* shutdown the xively library */
        xi_shutdown();
    }
}

void Callback_ConnectedToWiFi()
{
    /* At this point the device has a wifi address. */
    /* Let's kick off our connection to Xively! */
    sem_post( &gApplicationControlBlock.wifiConnectedSem );
}

void Callback_LostWiFiConnection()
{
    gApplicationControlBlock.initializationState = InitializationState_ShuttingDownConnection;
    Report( "Lost WiFi Connection.  Restarting Application!\n\r" );
    xi_shutdown_connection( gXivelyContextHandle );
}

/* @brief Starts a connection to the Xively Service.  xi_events_process_blocking
 * should be called after this function so that the connection process can move
 * forward.
 */
void ConnectToXively()
{
    Report(" Connecting to Xively: \n");
    Report( "\t- Xively Account ID: %s\n", gApplicationControlBlock.xivelyAccountId );
    Report( "\t- Xively Device ID: %s\n", gApplicationControlBlock.xivelyDeviceId );
    Report( "\t- Xively Device Password: <secret>\n" );

    xi_state_t ret_state = xi_initialize( gApplicationControlBlock.xivelyAccountId,
                                          gApplicationControlBlock.xivelyDeviceId );
    if ( XI_STATE_OK != ret_state )
    {
        Report( "xi failed to initialize\n\r" );
        return;
    }

    gXivelyContextHandle = xi_create_context();

    if ( XI_INVALID_CONTEXT_HANDLE == gXivelyContextHandle )
    {
        Report( " xi failed to create context, error: %d\n\r",
                    -gXivelyContextHandle );
        return;
    }
#if ENABLE_XIVELY_FIRMWARE_UPDATE_AND_SECURE_FILE_TRANSFER
    /* Pass list of files to be updated by the Xively Services. */
    const char** files_to_keep_updated = ( const char* [] ){"firmware.tar"};

    xi_set_updateable_files( gXivelyContextHandle, files_to_keep_updated, 1, NULL );
#endif

    xi_state_t connect_result =
        xi_connect( gXivelyContextHandle, gApplicationControlBlock.xivelyDeviceId,
                    gApplicationControlBlock.xivelyDevicePassword, 10, 0,
                    XI_SESSION_CLEAN, &on_connected );
}

/* Used by the on_connected callback to cancel tasks and stop the event loop, clean
 * up tasks, etc.
 */
void exit_xi_and_cleanup()
{
    xi_cancel_timed_task( gTemperatureTaskHandle );
    gTemperatureTaskHandle = XI_INVALID_TIMED_TASK_HANDLE;
    xi_events_stop();
    gApplicationControlBlock.initializationState = InitializationState_Restarting;
}

/* @brief Connection callback.  See xively.h for full signature information
 *
 * @param in_context_handle the xively connection context as was returned from
 * xi_create_context. This denotes which connection is calling this callback and
 * could be used to differentiate connections if you have more than one ongoing.)
 * This value must be used for all Xively C Client operations.
 * @param data an abstract data pointer that is parsed differently depending on the
 * connection state.  See the source below for example usage.
 * @param state status / error code from xi_error.h that might be applicable
 * to the connection state change.
 * */
void on_connected( xi_context_handle_t in_context_handle, void* data, xi_state_t state )
{
    xi_connection_data_t* conn_data = ( xi_connection_data_t* )data;
    gApplicationControlBlock.xivelyConnectionStatus = conn_data->connection_state;
    switch ( conn_data->connection_state )
    {
        case XI_CONNECTION_STATE_OPEN_FAILED:
            Report( "connection to %s:%d has failed reason %d\n\r", conn_data->host,
                    conn_data->port, state );

            if( InitializationState_ShuttingDownConnection == gApplicationControlBlock.initializationState )
            {
                Report( "on_connected callback noticed that the WiFi signal has been lost.  Shutting down.\n" );
                exit_xi_and_cleanup();
                return;
            }

            gApplicationControlBlock.initializationState =
                InitializationState_ConnectingToXively;

            xi_connect( in_context_handle, conn_data->username, conn_data->password,
                        conn_data->connection_timeout, conn_data->keepalive_timeout,
                        conn_data->session_type, &on_connected );

            return;
        case XI_CONNECTION_STATE_OPENED:
            Report( "Xively connected to %s:%d\n\r", conn_data->host, conn_data->port );

            gApplicationControlBlock.initializationState =
                InitializationState_SubscribingToTopics;

            /* Schedule the send_temperature function to publish a temperature reading
             * every 10 seconds */
            gTemperatureTaskHandle = xi_schedule_timed_task(
                in_context_handle, send_temperature, 10, 1, NULL );


            if ( XI_INVALID_TIMED_TASK_HANDLE == gTemperatureTaskHandle )
            {
                Report( "send_temperature_task couldn't be registered\n\r" );
            }
            else
            {
                /* Call it now so that we get a temperature reading immediately, too */
                send_temperature( in_context_handle, gTemperatureTaskHandle, NULL );
            }

            /* Green LED topic subscription */
            subscribe_to_topic( in_context_handle, XIVELY_GREEN_LED_CHANNEL_NAME,
                                on_led_topic, ( void* )&gGreenLed );

            /* Orange LED topic subscription */
            subscribe_to_topic( in_context_handle, XIVELY_ORANGE_LED_CHANNEL_NAME,
                                on_led_topic, ( void* )&gOrangeLed );

            /* Red LED topic subscription */
            subscribe_to_topic( in_context_handle, XIVELY_RED_LED_CHANNEL_NAME,
                                on_led_topic, ( void* )&gRedLed );

            return;

        case XI_CONNECTION_STATE_CLOSED:
            Report( "connection closed - reason %d!\n\r", state );

            /* shut down the periodic temperature sensor reading
             * when we lose our connection */
            exit_xi_and_cleanup();
            return;
        default:
            Report( "invalid parameter %d\n\r", conn_data->connection_state );
            return;
    }
}

/* @brief Subscribes to a topic given a channel (topic suffix) and a callback.
 * See xively.h for full signature information of callbacks.
 *
 * @param in_context_handle the Xively connection context as was returned from
 * xi_create_context, and passed in from the connection callback on_connected().
 * This value must be used in all further Xively operations, including subscription
 * requests (below.)
 * @param channel a string to append to the end of the topic base string.  General
 * topic structure is:  xi/blue/v1/{accountId}/d/{deviceId}/{channel}.
 * @param callback a pointer to the callback function that handles subscription
 * status updates and incoming messages on the topic.  For this example application
 * this is on_led_topic as specified in the invocation of this function in
 * on_connected() (above.)
 * @param user_data an abstract data pointer that can be anything, and will be
 * passed to the subscription handler on all invocations.  In this case the GPIO
 * index of the corresponding LED is provided. We use this to easily associate a topic
 * with an LED, without having to parse the full topic name.
 * @retval xi_state_t which is the success / failure code of the xi_subscription request.
 * Since the actual subscription response from the service occurs asynchronously, this
 * value represents if the subscription request was properly queued by the Xively Client,
 * and does not reflect the SUBACK value of the subscription request itself.
 */
xi_state_t subscribe_to_topic( xi_context_handle_t context_handle,
                               const char* channel,
                               xi_user_subscription_callback_t* callback,
                               void* user_data )
{
    char topic_name[XIVELY_TOPIC_LEN];
    memset( topic_name, '\0', XIVELY_TOPIC_LEN );

    snprintf( topic_name, XIVELY_TOPIC_LEN, XIVELY_TOPIC_FORMAT,
              gApplicationControlBlock.xivelyAccountId,
              gApplicationControlBlock.xivelyDeviceId, channel );

    return xi_subscribe( context_handle, topic_name, XI_MQTT_QOS_AT_MOST_ONCE, callback,
                         user_data );
}


/**
 * @brief Handles messages sent to the device on the LEDs' topics,
 * including toggle commands and subscription changes.
 *
 * @note For more details please refer to the xively.h - xi_subscribe function.
 *
 * @param in_context_handle - xively context required for calling API functions
 * @param call_type - describes if this callback was invoked for SUBACK or an
 * incoming message
 * @oaram params - structure with data depending on SUBACK or incoming message
 * @param state - state of the operation.  Please see xi_subscribe in xively.h
 * for more information
 * @oaram user_data - Pointer provided upon topic subscription in the connection handler
 * function on_connected(). For this application's use, these are the LED GPIO
 * identifiers so we can directly manipulate the LEDs from this function.
 */
void on_led_topic( xi_context_handle_t in_context_handle,
                   xi_sub_call_type_t call_type,
                   const xi_sub_call_params_t* const params,
                   xi_state_t state,
                   void* user_data )
{
    switch ( call_type )
    {
        case XI_SUB_CALL_SUBACK:
            if ( params->suback.suback_status == XI_MQTT_SUBACK_FAILED )
            {
                Report( "topic:%s. Subscription failed.\n\r", params->suback.topic );
            }
            else
            {
                Report( "topic:%s. Subscription granted %d.\n\r", params->suback.topic,
                        ( int )params->suback.suback_status );

                /* Switch all off and prepare for incoming led topic messages to control
                 * the LEDs */
                gApplicationControlBlock.initializationState =
                    InitializationState_Connected;
                GPIO_write( Board_LED2, Board_LED_OFF );
                GPIO_write( Board_LED1, Board_LED_OFF );
                GPIO_write( Board_LED0, Board_LED_OFF );
            }
            return;
        case XI_SUB_CALL_MESSAGE:
            if ( params->message.temporary_payload_data_length == 1 )
            {
                if ( InitializationState_Connected ==
                     gApplicationControlBlock.initializationState )
                {
                    uint32_t led_gpio_index = *( ( uint32_t* )user_data );
                    switch ( params->message.temporary_payload_data[0] )
                    {
                        case '0':
#ifndef RELEASE_2
                            GPIO_write( led_gpio_index, Board_LED_OFF );
#else
                            GPIO_write( led_gpio_index, Board_LED_ON );
#endif
                            break;
                        case '1':
#ifndef RELEASE_2
                            GPIO_write( led_gpio_index, Board_LED_ON );
#else
                            GPIO_write( led_gpio_index, Board_LED_OFF );
#endif
                            break;
                        default:
                            Report( "unexpected value on topic %s \n\r",
                                    params->message.topic );
                            break;
                    }
                }
            }
            else
            {
                Report( "unexpected data length on topic %s \n\r",
                        params->message.topic );
            }
            return;
        default:
            return;
    }
}

/**
 * @brief Device's Button Handler. Upon invocation from register interrupt handler,
 * this functions publishes to a specific topic depending on the button pressed.
 * The function also tracks state, toggling it's value between 0 and 1
 * on every corresponding button press.
 *
 * @param index of the interrupt.  This will be either Board_BUTTON0,
 * or Board_BUTTON1
 */
void button_interrupt_handler( unsigned int index )
{
    const char* buttonChannelName = NULL;
    int pinValue                  = 0;
    switch ( index )
    {
        case Board_BUTTON0: /* 0 */
            // Software Button 0 is hardware SW2
            Report( "Board Button 0 pressed\n\r" );
            gApplicationControlBlock.button0State =
                !gApplicationControlBlock.button0State;
            pinValue = gApplicationControlBlock.button0State;
            GPIO_clearInt( Board_BUTTON0 );
            buttonChannelName = XIVELY_BUTTON0_CHANNEL_NAME;
            break;
        case Board_BUTTON1: /* 1 */
            // Software Button 1 is hardware SW3
            Report( "Board Button 1 pressed\n\r" );
            gApplicationControlBlock.button1State =
                !gApplicationControlBlock.button1State;
            pinValue = gApplicationControlBlock.button1State;
            GPIO_clearInt( Board_BUTTON1 );
            buttonChannelName = XIVELY_BUTTON1_CHANNEL_NAME;
            break;
        default:
            Report( "Unknown GPIO device invoked button_interrupt_handler\n\r" );
            break;
    }

    const char* messageValue = NULL;
    switch ( pinValue )
    {
        case 0:
            messageValue = "0";
            break;
        case 1:
            messageValue = "1";
            break;
        default:
            Report( "Unsupported PinValue: %d\n\r", pinValue );
            break;
    }

    /* some last sanity checks, including a check to ensure that we're actively connected
     * to Xively.
     */
    if ( XI_CONNECTION_STATE_OPENED == gApplicationControlBlock.xivelyConnectionStatus &&
         NULL != buttonChannelName && NULL != messageValue )
    {
        char topic_name[XIVELY_TOPIC_LEN];
        memset( topic_name, '\0', XIVELY_TOPIC_LEN );

        snprintf( topic_name, XIVELY_TOPIC_LEN, XIVELY_TOPIC_FORMAT,
                  gApplicationControlBlock.xivelyAccountId,
                  gApplicationControlBlock.xivelyDeviceId, buttonChannelName );

        Report( "Button press publishing to topic name: %s\n\r", topic_name );
        Report( "message value: %s\n\r", messageValue );
        xi_publish( gXivelyContextHandle, topic_name, messageValue,
                    XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
    }
}

/**
 * @brief Invoked by the Xively Scheduler as setup as a period task to publish
 * temperature data from the device.  This task was created in on_connected()
 *
 * NOTE: The CC3220 I2C pins, which are required to access the temperature
 * sensor, are shared with the GPIO pins for the LED controls.
 * Therefore this function will momentarily disable the LED controls, read
 * the temperature value, and then put the GPIO controls back in place.
 *
 * @param context_handle - xively context required for calling API functions
 * @param timed_task_handle - context associated with scheduled event.
 * this can be used to cancel the periodic invocation of the task. Please
 * see xi_cancel_timed_task() in xively.h for more information.
 * @oaram user_data - application specific pointer provided upon timed task creation.
 * This is unused in this example, and is NULL.  Please see xi_schedule_timed_task()
 * in on_connected().
 */
void send_temperature( const xi_context_handle_t context_handle,
                       const xi_timed_task_handle_t timed_task_handle,
                       void* user_data )
{
    uint16_t temperature;
    uint8_t txBuffer[1];
    uint8_t rxBuffer[2];
    I2C_Handle i2c;
    I2C_Params i2cParams;
    I2C_Transaction i2cTransaction;

    /* Create I2C for usage */
    PinTypeI2C( PIN_01, PIN_MODE_1 );
    PinTypeI2C( PIN_02, PIN_MODE_1 );

    I2C_Params_init( &i2cParams );
    i2cParams.bitRate = I2C_400kHz;
    i2c               = I2C_open( Board_I2C_TMP, &i2cParams );
    if ( i2c == NULL )
    {
        Report( "Error Initializing I2C for Temperature Reading\r\n" );
        return;
    }

    /* Point to the T ambient register and read its 2 bytes */
    txBuffer[0]                 = 0x01;
    i2cTransaction.slaveAddress = Board_TMP006_ADDR;
    i2cTransaction.writeBuf     = txBuffer;
    i2cTransaction.writeCount   = 1;
    i2cTransaction.readBuf      = rxBuffer;
    i2cTransaction.readCount    = 2;

    if ( I2C_transfer( i2c, &i2cTransaction ) )
    {
        /* Extract degrees C from the received data; see TMP102 datasheet */
        temperature = ( rxBuffer[0] << 6 ) | ( rxBuffer[1] >> 2 );

        /*
         * If the MSB is set '1', then we have a 2's complement
         * negative value which needs to be sign extended */
        if ( rxBuffer[0] & 0x80 )
        {
            temperature |= 0xF000;
        }

        /*
             * For simplicity, divide the temperature value by 32 to get rid of
             * the decimal precision; see TI's TMP006 datasheet */
        temperature /= 32;

        /* Convert to F */
        temperature = ( temperature * 9 ) / 5;
        temperature += 32;
    }
    else
    {
        Report( "I2C Bus fault\n\r" );
    }

    /* Close the I2C context */
    I2C_close( i2c );

    /* Return Pins to LED control over GPIO */
    PinTypeGPIO( PIN_01, PIN_MODE_0, false );
    GPIODirModeSet( GPIOA1_BASE, 0x4, GPIO_DIR_MODE_OUT );

    PinTypeGPIO( PIN_02, PIN_MODE_0, false );
    GPIODirModeSet( GPIOA1_BASE, 0x8, GPIO_DIR_MODE_OUT );

    /* Publish the temperature to Xively */
    char msg[16] = {'\0'};
    sprintf( msg, "%d", temperature );

    char topic_name[XIVELY_TOPIC_LEN];
    memset( topic_name, '\0', XIVELY_TOPIC_LEN );

    snprintf( topic_name, XIVELY_TOPIC_LEN, XIVELY_TOPIC_FORMAT,
              gApplicationControlBlock.xivelyAccountId,
              gApplicationControlBlock.xivelyDeviceId, XIVELY_TEMPERATURE_CHANNEL_NAME );

    Report( "Publishing temperature: %s\n\r", msg );
    xi_publish( context_handle, topic_name, msg, XI_MQTT_QOS_AT_MOST_ONCE,
                XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

/* @brief Retrieves Wifi Credential and Xively Credential information from a configuration
 * file stored on the Flash File System of the device.
 *
 * Invokes the configuration file parser in config_file.h / config_file.c for file
 * operations and for parsing the key value pairs.
 *
 * Ensures that all required credentials exist in the file and that they fit into the
 * Application control block buffers.  Additionally the wifi security
 * type is mapped to an existing CC3220 Simple Link Enumeration.
 *
 * If any validation fails then this function logs an error to the console
 * and loops forever, since any configuration error will prevent the application
 * from connecting to wifi and/or the xively service.
 */
void parseCredentialsFromConfigFile()
{
    config_entry_t* config_file_context;
    int len = 0;
    int err = read_config_file( XIVELY_CFG_FILE, &config_file_context );
    if ( SL_ERROR_FS_INVALID_TOKEN_SECURITY_ALERT == err )
    {
        Report( "[SECURITY ALERT] Invalid token for secure config file %s. Error %d\n\r",
                XIVELY_CFG_FILE, err );
    }
    else if ( SL_ERROR_FS_DEVICE_NOT_SECURED == err )
    {
        Report( "[SECURITY ALERT] Reading the secure config file %s can only be done "
                "in a secure device type. Error %d\n\r",
                XIVELY_CFG_FILE, err );
    }
    else if ( err )
    {
        Report( ". Reading %s config file failed. returned %d\n\r", XIVELY_CFG_FILE,
                err );
        if ( SL_ERROR_FS_FILE_NOT_EXISTS == err )
        {
            Report( ". File does not exist on flash file system.\n\r" );
        }
        Report( ". Cannot recover from error.\n\r" );
        Report( ". Looping forever.\n\r" );
        while ( 1 )
            ;
    }

    /* parse wifi credentials from config file */
    static char *wifi_ssid = NULL, *wifi_security_type = NULL, *wifi_password = NULL;
    err |= parseKeyValue( config_file_context, WIFI_SSID_KEY, 1, &wifi_ssid );
    err |=
        parseKeyValue( config_file_context, WIFI_SEC_TYPE_KEY, 1, &wifi_security_type );
    err |= parseKeyValue( config_file_context, WIFI_PASSWORD_KEY, 1, &wifi_password );

    /* parse Xively account & device credentials from config file */
    static char *xively_account_id = NULL, *xively_device_id = NULL,
                *xively_device_password = NULL;
    err |= parseKeyValue( config_file_context, XIVELY_ACNT_KEY, 1, &xively_account_id );
    err |= parseKeyValue( config_file_context, XIVELY_DEV_KEY, 1, &xively_device_id );
    err |=
        parseKeyValue( config_file_context, XIVELY_PWD_KEY, 1, &xively_device_password );


    /* if everything is ok so far then do some sanity checks on the string sizes and the
     * parsed values. */
    if ( !err )
    {
        err |= validateWifiConfigurationVariables( wifi_ssid, wifi_password,
                                                   wifi_security_type );

        gApplicationControlBlock.desiredWifiSecurityType =
            mapWifiSecurityTypeStringToInt( ( const char* )wifi_security_type );

        if ( gApplicationControlBlock.desiredWifiSecurityType == SL_WLAN_SEC_TYPE_WEP )
        {
            len = strlen( wifi_password );

            if ( ( len == 10 ) || ( len == 26 ) )
            {
                if ( FixWEPKey( wifi_password, len ) < 0 )
                {
                    // Error Bad WEP Key
                    Report( "[POST] Bad WEP key hex!\r\n" );
                }
            }
        }


        err |= validateXivelyCredentialBufferRequirement( XIVELY_ACNT_KEY,
                                                          xively_account_id );
        err |=
            validateXivelyCredentialBufferRequirement( XIVELY_DEV_KEY, xively_device_id );
        err |= validateXivelyCredentialBufferRequirement( XIVELY_PWD_KEY,
                                                          xively_device_password );
    }

    /* one last check before we start copying memory around */
    if ( err )
    {
        Report( "\n\rCritical configuration error(s) found when parsing configuration "
                "file\n\r" );
        Report( "Looping forever.\n\r" );
        while ( 1 )
            ;
    }

    /* assign the variables into the Application Control Block. */

    memcpy( gApplicationControlBlock.desiredWifiSSID, wifi_ssid, strlen( wifi_ssid ) );
    memcpy( gApplicationControlBlock.desiredWifiKey, wifi_password,
            strlen( wifi_password ) );

    memcpy( gApplicationControlBlock.xivelyAccountId, xively_account_id,
            strlen( xively_account_id ) );
    memcpy( gApplicationControlBlock.xivelyDeviceId, xively_device_id,
            strlen( xively_device_id ) );
    memcpy( gApplicationControlBlock.xivelyDevicePassword, xively_device_password,
            strlen( xively_device_password ) );

    /* cleanup */
    free_config_entries( config_file_context );
}

/* @brief helper function that uses the config_file parser to parse a value for the
 * provided key.
 *
 * @param config_file_context this value returned from the creation of the config_file
 * parser.
 * This context is required for all config_file operations.
 * @param key a string to use as a lookup key for the key/value pairs in the config file.
 * @param is_required when set to a non-zero value, an error will be logged to the console
 * if the
 * specified key could not be found in the configuration file.  This does not effect the
 * return value
 * of this function.
 * @retval 0 on successful parse of parameter. -1 if parameter does not exist in the
 * configuration file,
 * or if another error has occurred.
 */
int8_t parseKeyValue( config_entry_t* config_file_context,
                      const char* key,
                      const uint8_t is_required,
                      char** out_value )
{
    int8_t retVal = 0;
    if ( NULL == out_value )
    {
        Report( "parseRequiredKeyValue error: out_value parameter is NULL!\n\r" );
        retVal = -1;
    }
    else if ( NULL == key )
    {
        Report( "parseRequiredKeyValue error: in_key parameter is NULL!\n\r" );
        retVal = -1;
    }
    else
    {
        *out_value = get_config_value( config_file_context, key );
        if ( NULL == *out_value )
        {
            retVal = -1;
            if ( is_required )
            {
                Report( "Could not find an entry for \"%s\" key in configuration file\n",
                        key );
            }
        }
    }

    return retVal;
}

/* @brief helper function. checks to if wifi credentials are of the right size
 * and type to match simplielink API
 *
 * @param wifi_ssid The wifi network name.
 * @param wifi_ssid The wifi password
 * @param wifi_ssid a string that represents the suffix of one of the valid
 * simplelink enumerated types. For instance "wpa" would be mapped to
 * SL_WLAN_SEC_TYPE_WPA.  This check is done in mapWifiSecurityTypeStringToInt().
 * @retval 0 if parameters are valid, -1 otherwise.
 */
int8_t validateWifiConfigurationVariables( const char* wifi_ssid,
                                           const char* wifi_password,
                                           const char* wifi_security_type )
{
    int8_t retVal = 0;

    /* SSID / wifi name checks */
    if ( strlen( wifi_ssid ) > SSID_LEN_MAX )
    {
        retVal = -1;
        Report( ". \"%s\" string is too long for CC3220 wifi configuration: \"%s\"\n\r",
                WIFI_SSID_KEY, wifi_ssid );
        Report( ". Max String Length is: %d\n\r", SSID_LEN_MAX );
    }

    /* wifi password checks */
    if ( strlen( wifi_password ) > WIFI_KEY_LEN_MAX )
    {
        retVal = -1;
        Report( ". Value for \"%s\" string is too big: \"%s\"\n\r", WIFI_SSID_KEY,
                wifi_password );
        Report( ". Please reconfigure WIFI_KEY_LEN_MAX in xively_example.h to increase "
                "the buffer size\n\r" );
        Report( ". Max String Length is: %d\n\r", WIFI_KEY_LEN_MAX );
    }

    /* Security type enum checks */
    int8_t wifi_security_type_enum =
        mapWifiSecurityTypeStringToInt( ( const char* )wifi_security_type );
    if ( wifi_security_type < 0 )
    {
        retVal = -1;
        Report( ". Could not find valid \"%s\" ", WIFI_SEC_TYPE_KEY );
        Report( " in xively configuration file \"%s\"\n\r", XIVELY_CFG_FILE );
    }

    if ( 0 == strcmp( wifi_ssid, WIFI_DEFAULT_CONFIG_FIELD_VALUE ) )
    {
        retVal = -1;
        Report( ". Default WiFi SSID field entry found.\n\r" );
        Report( "   Did you forget to edit the configuration file \"%s\"?\n\r",
                XIVELY_CFG_FILE );
    }

    if ( 0 == strcmp( wifi_password, WIFI_DEFAULT_CONFIG_FIELD_VALUE ) )
    {
        retVal = -1;
        Report( ". Default WiFi Password field entry found.\n\r" );
        Report( "   Did you forget to edit the configuration file \"%s\"?\n\r",
                XIVELY_CFG_FILE );
    }

    return retVal;
}

/* @brief helper function. checks to if key's value string will fit into our Application
 * Control Block
 *
 * @param config_key the key string that was used to lookup the value in the config file.
 * This is only used for logging purposes.
 * @param value the value string that will be stored as a Xively Credential.  This
 * function
 * checks to make sure that the value will fit in the Application Control Blocks buffers
 * for Xively Credentials before a copy is made.
 * @retval 0 if the credentials are of a valid size, -1 otherwise.
 * */
int8_t validateXivelyCredentialBufferRequirement( const char* const config_key,
                                                  const char* value )
{
    if ( NULL == value )
    {
        Report( ". Key \"%s\" is missing a value in the configuration file: \"%s\"\n\r",
                config_key, value );
        return -1;
    }

    if ( 0 == strcmp( value, XIVELY_DEFAULT_CONFIG_FIELD_VALUE ) )
    {
        Report( ". \"%s\" value's is still the default value.\n\r", config_key );
        Report( "   Did you forget to edit the configuration file \"%s\"?\n\r",
                XIVELY_CFG_FILE );
        return -1;
    }

    if ( strlen( value ) > XIVELY_CRED_LEN_MAX )
    {
        Report( ". \"%s\" value's string is too long for internal buffer configuration. "
                "value is: \"%s\"\n\r",
                config_key, value );
        Report( ". Max String Length is: %d\n\r", XIVELY_CRED_LEN_MAX );
        return -1;
    }

    return 0;
}

/* @brief Attempts to match the string from the configuration file to an enumeration
 * the platform's SDK wlan.h file.  The most common LAN types are supported
 * out of the box. The more uncommong lans haven't been tested again
 * but in theory could be enabled.
 * @retval one of the SL_WLAN_SEC_TYPE values as defined in wlan.h, or
 * -1 if the mapping failed.
 */
int8_t mapWifiSecurityTypeStringToInt( const char* security_type )
{
    if ( NULL == security_type )
    {
        Report( "Now Wifi Security Type String found in configuration file!\n\r" );
        return -1;
    }

    if ( strcmp( "open", security_type ) == 0 || strcmp( "OPEN", security_type ) == 0 )
    {
        return SL_WLAN_SEC_TYPE_OPEN;
    }

    if ( strcmp( "wep", security_type ) == 0 || strcmp( "WEP", security_type ) == 0 )
    {
        return SL_WLAN_SEC_TYPE_WEP;
    }

    if ( strcmp( "wpa", security_type ) == 0 || strcmp( "WPA", security_type ) == 0 )
    {
        return SL_WLAN_SEC_TYPE_WPA;
    }

    if ( strcmp( "wpa2", security_type ) == 0 || strcmp( "WPA2", security_type ) == 0 )
    {
        return SL_WLAN_SEC_TYPE_WPA_WPA2;
    }

    if ( strcmp( "wpa_ent", security_type ) == 0 ||
         strcmp( "WPA_ENT", security_type ) == 0 )
    {
        return SL_WLAN_SEC_TYPE_WPA_ENT;
    }

    /* potential future support based on CC3220 SDK:
     *
     * SL_WLAN_SEC_TYPE_WPS_PBC
     * SL_WLAN_SEC_TYPE_WPS_PIN
     * SL_WLAN_SEC_TYPE_P2P_PBC
     * SL_WLAN_SEC_TYPE_P2P_PIN_KEYPAD
     * SL_WLAN_SEC_TYPE_P2P_PIN_DISPLAY
     * SL_WLAN_SEC_TYPE_P2P_PIN_AUTO
     */
    Report( "Unsupported Wifi security type: \"%s\"\n\r", security_type );
    return -1;
}

#if 1
/**
 * @brief Function that is required by WolfSSL library to track the current time.
 */
long XTIME( long* timer )
{
    return xi_bsp_time_getcurrenttime_seconds();
}

/**
 * @brief Function required by the WolfSSL TLS library.
 */
uint32_t xively_ssl_rand_generate()
{
    return xi_bsp_rng_get();
}
#endif

static signed char hexConvertASCIINibble( signed char nibble )
{
    signed char val = -1;

    if ( ( nibble >= '0' ) && ( nibble <= '9' ) )
    {
        val = ( nibble - '0' );
    }
    else if ( ( nibble >= 'A' ) && ( nibble <= 'F' ) )
    {
        val = ( nibble - 'A' + 10 );
    }
    else if ( ( nibble >= 'a' ) && ( nibble <= 'f' ) )
    {
        val = ( nibble - 'a' + 10 );
    }

    return val;
}

static signed char
hexConvertByteASCII( signed char msb, signed char lsb, signed char* val )
{
    signed char msn;
    signed char lsn;

    msn = hexConvertASCIINibble( msb );
    lsn = hexConvertASCIINibble( lsb );

    if ( ( msn < 0 ) || ( lsn < 0 ) )
        return -1;

    *val = ( msn * 16 ) + lsn;

    return 0;
}

static signed char FixWEPKey( char* key, unsigned char keyLen )
{
    int i = 0;
    signed char msb;
    signed char lsb;

    while ( i < ( keyLen >> 1 ) )
    {
        msb = *( key + ( 2 * i ) );
        lsb = *( key + 1 + ( 2 * i ) );

        if ( hexConvertByteASCII( msb, lsb, ( signed char* )( key + i ) ) < 0 )
            return -1;
        i++;
    }

    while ( i < keyLen )
    {
        *( key + i ) = 0;
        i++;
    }

    return 0;
}
