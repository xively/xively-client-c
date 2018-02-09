/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

//*****************************************************************************
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************


/*****************************************************************************
 *                              Includes
 *****************************************************************************/

/* Simplelink */
#include "simplelink.h"
#include "flc.h"

/* Driverlib */
#include "hw_gpio.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include "interrupt.h"
#include "pin.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
#include "utils.h"
#include "gpio.h"
#include "stdio.h"

/* Common interfaces */
#include "button_if.h"
#include "common.h"
#include "gpio_if.h"
#include "i2c_if.h"
#include "pinmux.h"
#include "tmp006drv.h"
#include "uart_if.h"

/* Xively Application */
#include "time.h"
#include "xi_bsp_rng.h"
#include "xi_bsp_time.h"
#include "xively.h"
#include "xi_sft.h"
#include "config_file.h"


/*****************************************************************************
 *                              Defines
 *****************************************************************************/
#define XIVELY_CRED_LEN_MAX ( 64 )
#define WIFI_KEY_LEN_MAX ( 64 )
#define XIVELY_CRED_LEN_MAX ( 64 )

/* Path to load the configuration file from device's flash file system.
 * This config file contains Xively connection credentials and WiFi SSID
 * and password credentials */
#define XIVELY_CFG_FILE "/etc/xively_cfg.txt"

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

/* Used to detect if the user has configured their xively_cfg.txt file
 * on the device before the execution of the application */
#define XIVELY_DEFAULT_CONFIG_FIELD_VALUE "<PASTE_XIVELY_CREDENTIAL_HERE>"
#define WIFI_DEFAULT_CONFIG_FIELD_VALUE "<ENTER_WIFI_CREDENTIAL_HERE>"

/* String template for dynamically constructing the various Xively topics
 * that are used for this application. */
#define XIVELY_TOPIC_LEN ( 128 )
#define XIVELY_TOPIC_FORMAT "xi/blue/v1/%s/d/%s/%s"

/* Simplelink Application specific status/error codes */
typedef enum {
    /* Choosing -0x7D0 to avoid overlap w/ host-driver's error codes */
    LAN_CONNECTION_FAILED      = -0x7D0,
    DEVICE_NOT_IN_STATION_MODE = LAN_CONNECTION_FAILED - 1,
    STATUS_CODE_MAX            = -0xBB8
} e_AppStatusCodes;

/* WiFi and Xively Credentials are stored here after being parsed
 * from configuration file in FFS. */
typedef struct WifiDeviceCredentials_s
{
    int desiredWifiSecurityType;               /* see SL_WLAN_SEC_TYPE in wlan.h */
    char desiredWifiSSID[SSID_LEN_MAX + 1];    /* user configured host wifi SSID */
    char desiredWifiKey[WIFI_KEY_LEN_MAX + 1]; /* user configured wifi password */
    char xivelyAccountId[XIVELY_CRED_LEN_MAX];
    char xivelyDeviceId[XIVELY_CRED_LEN_MAX];
    char xivelyDevicePassword[XIVELY_CRED_LEN_MAX];
} WifiDeviceCredentials_t;

/*****************************************************************************
 *                           GLOBAL VARIABLES
 *****************************************************************************/
volatile unsigned long g_ulStatus = 0; /* SimpleLink Status */

/* Stores Provisioned Network information after acquiring WiFi network connection */
unsigned long g_ulGatewayIP = 0;                    /* Network Gateway IP address */
unsigned char g_ucConnectionSSID[SSID_LEN_MAX + 1]; /* Connection SSID */
unsigned char g_ucConnectionBSSID[BSSID_LEN_MAX];   /* Connection BSSID */

/* Struct which stores device configuration for desired WiFi connection parameters and
 * for Xively connection parameters. */
WifiDeviceCredentials_t g_wifi_device_credentials;

/* Context for the Xively Connection */
xi_context_handle_t gXivelyContextHandle = -1;


/****************************************************************************
 *                      FUNCTION PROTOTYPES
 *****************************************************************************/

/* Xively Functions */
void ConnectToXively();
void on_connected( xi_context_handle_t in_context_handle, void* data, xi_state_t state );
void send_fileinfo( const xi_context_handle_t context_handle,
                    const xi_timed_task_handle_t timed_task_handle,
                    void* user_data );
void pushButtonInterruptHandlerSW2();
void pushButtonInterruptHandlerSW3();

/* Xively and Wifi Config File Parsing functions */
static void parseCredentialsFromConfigFile();
static int8_t parseKeyValue( config_entry_t* config_file_context,
                             const char* key,
                             const uint8_t is_required,
                             char** out_value );
static int8_t mapWifiSecurityTypeStringToInt( const char* security_type );

/* Xively and Wifi Config value verification functions */
static int8_t validateWifiConfigurationVariables( const char* wifi_ssid,
                                                  const char* wifi_password,
                                                  const char* wifi_security_type );

static int8_t
validateXivelyCredentialBufferRequirement( const char* config_key, const char* value );

/* CCS requirement */
extern void ( *const g_pfnVectors[] )( void );

/* Device and Simplelink WiFi Provisioning Functions */
long MainLogic();
void WaitForWlanEvent();
static void BoardInit( void );
static void InitializeAppVariables();
static long ConfigureSimpleLinkToDefaultState();

/****************************************************************************
 *                               Source!
 *****************************************************************************/

/**
 * @brief Main Application entry point.
 *
 * Initializes board and then calls MainLogic
 * for Wifi Provisioning and connecting to Xively
 */
int main()
{
    long lRetVal = -1;

    /* Initialize Board configurations */
    BoardInit();
    PinMuxConfig();

    /* configure RED and Green LED */
    GPIO_IF_LedConfigure( LED1 | LED2 | LED3 );

#ifndef NOTERM
    InitTerm();
    ClearTerm();
#endif

    lRetVal = MainLogic();

    if ( lRetVal < 0 )
    {
        ERR_PRINT( lRetVal );
    }

    LOOP_FOREVER();
}

/**
 * @brief Driver for the xively library logic
 *
 * This function creates a xively context and starts the internal event
 * dispatcher and event processing of Xively library.
 *
 */
void ConnectToXively()
{
    /* Register Push Button Handlers */
    Button_IF_Init( pushButtonInterruptHandlerSW2, pushButtonInterruptHandlerSW3 );

    /* Make the buttons to react on both press and release events */
    MAP_GPIOIntTypeSet( GPIOA1_BASE, GPIO_PIN_5, GPIO_BOTH_EDGES );
    MAP_GPIOIntTypeSet( GPIOA2_BASE, GPIO_PIN_6, GPIO_BOTH_EDGES );

    /* Disable the interrupt for now */
    Button_IF_DisableInterrupt( SW2 | SW3 );

    xi_state_t ret_state = xi_initialize( g_wifi_device_credentials.xivelyAccountId );

    if ( XI_STATE_OK != ret_state )
    {
        printf( "xi failed to initialise\n" );
        return;
    }

    gXivelyContextHandle = xi_create_context();

    if ( XI_INVALID_CONTEXT_HANDLE == gXivelyContextHandle )
    {
        printf( " xi failed to create context, error: %d\n", -gXivelyContextHandle );
        return;
    }

    xi_state_t connect_result =
        xi_connect( gXivelyContextHandle, g_wifi_device_credentials.xivelyDeviceId,
                    g_wifi_device_credentials.xivelyDevicePassword, 10, 0,
                    XI_SESSION_CLEAN, &on_connected );

    /* start processing xively library events */
    xi_events_process_blocking();

    /* when the event dispatcher is stop destroy the context */
    xi_delete_context( gXivelyContextHandle );

    /* shutdown the xively library */
    xi_shutdown();
}


void send_fileinfo( const xi_context_handle_t context_handle,
                    const xi_timed_task_handle_t timed_task_handle,
                    void* user_data )
{
    /* guard against multiple sends */
    static int sent = 0;
    if ( !sent )
    {
        xi_publish_file_info( context_handle );
        sent = 1;
    }
}


/**
* @brief Event handler for SW2 button press/release interrupts.
 *
 * The pinValue will be 1 on a button press event, 0 on button up..
 */
void pushButtonInterruptHandlerSW2()
{
    if ( GPIOPinRead( GPIOA2_BASE, GPIO_PIN_6 ) != 0 )
    {
        if ( XI_INVALID_TIMED_TASK_HANDLE ==
             xi_schedule_timed_task( gXivelyContextHandle, send_fileinfo, 1, 0, NULL ) )
        {
            printf( "send_file_info couldn't be registered\n" );
        }
    }

    Button_IF_EnableInterrupt( SW2 );
}

/**
 * @brief Event handler for SW2 button press/release interrupts.
 *
 * The pinValue will be 1 on a button press event, 0 on button up.
 */
void pushButtonInterruptHandlerSW3()
{
    Button_IF_EnableInterrupt( SW3 );
}

/**
 * @brief Function that handles messages sent to the device on the `blink` topic.
 *
 * @note For more details please refer to the xively.h - xi_subscribe function.
 *
 * @param in_context_handle - xively library context required for calling API functions
 * @param call_type - describes if the handler was called carrying the subscription
 * operation result or the incoming message
 * @oaram params - structure with data
 * @param state - state of the operation
 * @oaram user_data - optional data, in this case nothing.
 */
void on_blink_topic( xi_context_handle_t in_context_handle,
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
                printf( "topic:%s. Subscription failed.\n", params->suback.topic );
            }
            else
            {
                printf( "topic:%s. Subscription granted %d.\n", params->suback.topic,
                        ( int )params->suback.suback_status );
            }
            return;
        case XI_SUB_CALL_MESSAGE:
        {
            static int toggle_lights = 0;
            toggle_lights            = ( toggle_lights + 1 ) % 2;

            if ( toggle_lights )
            {
                GPIO_IF_LedOn( MCU_GREEN_LED_GPIO );
                GPIO_IF_LedOn( MCU_ORANGE_LED_GPIO );
            }
            else
            {
                GPIO_IF_LedOff( MCU_GREEN_LED_GPIO );
                GPIO_IF_LedOff( MCU_ORANGE_LED_GPIO );
            }
            return;
        }
    }
}

/*
 * @brief Subscribes to a topic given a channel (topic suffix) and a callback.
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
 * this is on_blink_topic as specified in the invocation of this function in
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
              g_wifi_device_credentials.xivelyAccountId,
              g_wifi_device_credentials.xivelyDeviceId, channel );

    return xi_subscribe( context_handle, topic_name, XI_MQTT_QOS_AT_MOST_ONCE, callback,
                         user_data );
}

/**
 * @brief Handler function for event a related to changes in connection state.
 *
 * @note for more details please refer to xively.h - xi_connect function.
 *
 * @param context_handle - xively library context required for calling API functions
 * @param data - points to the structure that holds detailed information about the
 * xively libarry's connection state and settings
 * @param state - additional internal state of the library helps to determine the reason
 * of connection failure or disconnection
 */
void on_connected( xi_context_handle_t in_context_handle, void* data, xi_state_t state )
{
    xi_connection_data_t* conn_data = ( xi_connection_data_t* )data;

    switch ( conn_data->connection_state )
    {
        case XI_CONNECTION_STATE_OPEN_FAILED:
            printf( "connection to %s:%d has failed reason %d\n", conn_data->host,
                    conn_data->port, state );

            xi_connect( in_context_handle, conn_data->username, conn_data->password,
                        conn_data->connection_timeout, conn_data->keepalive_timeout,
                        conn_data->session_type, &on_connected );

            return;
        case XI_CONNECTION_STATE_OPENED:
            printf( "connected to %s:%d\n", conn_data->host, conn_data->port );

            subscribe_to_topic( in_context_handle, "blink", on_blink_topic, NULL );

            xi_sft_init( in_context_handle, g_wifi_device_credentials.xivelyAccountId,
                         g_wifi_device_credentials.xivelyDeviceId );

            /* enable the button interrupts */
            Button_IF_EnableInterrupt( SW2 | SW3 );

            break;
        case XI_CONNECTION_STATE_CLOSED:
            printf( "connection closed - reason %d!\n", state );

            /* disable button interrupts so the messages for button push/release events
             * won't be sent */
            Button_IF_DisableInterrupt( SW2 | SW3 );

            /* check if the close was due to the user's request */
            if ( XI_STATE_OK == state )
            {
                xi_events_stop();
            }
            else
            {
                WaitForWlanEvent();

                xi_connect( in_context_handle, conn_data->username, conn_data->password,
                            conn_data->connection_timeout, conn_data->keepalive_timeout,
                            conn_data->session_type, &on_connected );
            }
            return;
        default:
            printf( "invalid parameter %d\n", conn_data->connection_state );
            return;
    }
}

/**
 * @brief Function that is required by TLS library to track the current time.
 */
time_t XTIME( time_t* timer )
{
    return xi_bsp_time_getcurrenttime_seconds();
}

/**
 * @brief Function required by the TLS library.
 */
uint32_t xively_ssl_rand_generate()
{
    return xi_bsp_rng_get();
}


/******************************************************************************
          SimpleLink Network Provisioning and Connection Code
******************************************************************************/

long MainLogic()
{
    SlSecParams_t g_SecParams;
    long lRetVal          = -1;
    unsigned char pValues = 0;

    InitializeAppVariables();

    lRetVal = ConfigureSimpleLinkToDefaultState();
    if ( lRetVal < 0 )
    {
        if ( DEVICE_NOT_IN_STATION_MODE == lRetVal )
            printf( "Failed to configure the device "
                    "in its default state \n" );

        return lRetVal;
    }

    CLR_STATUS_BIT_ALL( g_ulStatus );

    /* Start simplelink */
    lRetVal = sl_Start( 0, 0, 0 );
    if ( lRetVal < 0 || ROLE_STA != lRetVal )
    {
        printf( "Failed to start the device \n" );
        return lRetVal;
    }

    memset( &g_wifi_device_credentials, 0, sizeof( g_wifi_device_credentials ) );
    parseCredentialsFromConfigFile();

    /* start ent wlan connection */
    g_SecParams.Key = ( _i8* )g_wifi_device_credentials.desiredWifiKey;
    g_SecParams.KeyLen =
        strlen( ( const char* )g_wifi_device_credentials.desiredWifiKey );
    g_SecParams.Type = g_wifi_device_credentials.desiredWifiSecurityType;

    /* 0 - Disable the server authnetication | 1 - Enable (this is the default) */
    pValues = 0;
    sl_WlanSet( SL_WLAN_CFG_GENERAL_PARAM_ID, 19, 1, &pValues );

    /* add the wlan profile */
    sl_WlanProfileAdd( ( _i8* )g_wifi_device_credentials.desiredWifiSSID,
                       strlen( g_wifi_device_credentials.desiredWifiSSID ), 0,
                       &g_SecParams, 0, 1, 0 );

    /* set the connection policy to auto */
    pValues = 0;
    sl_WlanPolicySet( SL_POLICY_CONNECTION, SL_CONNECTION_POLICY( 1, 1, 0, 0, 0 ),
                      &pValues, 1 );

    /* wait for the WiFi connection */
    WaitForWlanEvent();

    /* this is where the connection to Xively Broker can be established */
    ConnectToXively();

    /* wait for few moments */
    MAP_UtilsDelay( 80000000 );

    /* Disconnect from network */
    lRetVal = sl_WlanDisconnect();

    /* power off the network processor */
    lRetVal = sl_Stop( SL_STOP_TIMEOUT );

    return 0;
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
    int err = read_config_file( XIVELY_CFG_FILE, &config_file_context );
    if ( err )
    {
        printf( ". Reading %s config file failed. returned %d\n\r", XIVELY_CFG_FILE,
                err );
        if ( SL_FS_ERR_FILE_NOT_EXISTS == err )
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

    /* optional - parse xively broker address and port from config file */
    static char *broker_name = NULL, *broker_port_str = NULL;
    if ( 0 != parseKeyValue( config_file_context, XIVELY_HOST_KEY, 0, &broker_name ) )
    {
        broker_name = XIVELY_DEFAULT_BROKER;
    }

    if ( 0 == parseKeyValue( config_file_context, XIVELY_PORT_KEY, 0, &broker_port_str ) )
    {
        if ( 0 == ( atoi( broker_port_str ) ) )
        {
            printf( ". \"%s\" value \"%s\" is an invalid \"port\" value\n",
                    XIVELY_PORT_KEY, broker_port_str );
            err = -1;
        }
    }

    /* if everything is ok so far then do some sanity checks on the string sizes and the
     * parsed values. */
    if ( !err )
    {
        err |= validateWifiConfigurationVariables( wifi_ssid, wifi_password,
                                                   wifi_security_type );

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
        printf( "\n\rCritical configuration error(s) found when parsing configuration "
                "file\n\r" );
        printf( "Looping forever.\n\r" );
        while ( 1 )
            ;
    }

    /* assign the variables into the Application Control Block. */
    g_wifi_device_credentials.desiredWifiSecurityType =
        mapWifiSecurityTypeStringToInt( ( const char* )wifi_security_type );

    memcpy( g_wifi_device_credentials.desiredWifiSSID, wifi_ssid, strlen( wifi_ssid ) );
    memcpy( g_wifi_device_credentials.desiredWifiKey, wifi_password,
            strlen( wifi_password ) );
    memcpy( g_wifi_device_credentials.xivelyAccountId, xively_account_id,
            strlen( xively_account_id ) );
    memcpy( g_wifi_device_credentials.xivelyDeviceId, xively_device_id,
            strlen( xively_device_id ) );
    memcpy( g_wifi_device_credentials.xivelyDevicePassword, xively_device_password,
            strlen( xively_device_password ) );

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
        printf( "parseRequiredKeyValue error: out_value parameter is NULL!\n\r" );
        retVal = -1;
    }
    else if ( NULL == key )
    {
        printf( "parseRequiredKeyValue error: in_key parameter is NULL!\n\r" );
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
                printf( "Could not find an entry for \"%s\" key in configuration file\n",
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
        printf( ". \"%s\" string is too long for CC3220 wifi configuration: \"%s\"\n\r",
                WIFI_SSID_KEY, wifi_ssid );
        printf( ". Max String Length is: %d\n\r", SSID_LEN_MAX );
    }

    /* wifi password checks */
    if ( strlen( wifi_password ) > WIFI_KEY_LEN_MAX )
    {
        retVal = -1;
        printf( ". Value for \"%s\" string is too big: \"%s\"\n\r", WIFI_SSID_KEY,
                wifi_password );
        printf( ". Please reconfigure WIFI_KEY_LEN_MAX in xively_example.h to increase "
                "the buffer size\n\r" );
        printf( ". Max String Length is: %d\n\r", WIFI_KEY_LEN_MAX );
    }

    /* Security type enum checks */
    int8_t wifi_security_type_enum =
        mapWifiSecurityTypeStringToInt( ( const char* )wifi_security_type );
    if ( wifi_security_type < 0 )
    {
        retVal = -1;
        printf( ". Could not find valid \"%s\" ", WIFI_SEC_TYPE_KEY );
        printf( " in xively configuration file \"%s\"\n\r", XIVELY_CFG_FILE );
    }

    if ( 0 == strcmp( wifi_ssid, WIFI_DEFAULT_CONFIG_FIELD_VALUE ) )
    {
        retVal = -1;
        printf( ". Default WiFi SSID field entry found.\n\r" );
        printf( "   Did you forget to edit the configuration file \"%s\"?\n\r",
                XIVELY_CFG_FILE );
    }

    if ( 0 == strcmp( wifi_password, WIFI_DEFAULT_CONFIG_FIELD_VALUE ) )
    {
        retVal = -1;
        printf( ". Default WiFi Password field entry found.\n\r" );
        printf( "   Did you forget to edit the configuration file \"%s\"?\n\r",
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
        printf( ". Key \"%s\" is missing a value in the configuration file: \"%s\"\n\r",
                config_key, value );
        return -1;
    }

    if ( 0 == strcmp( value, XIVELY_DEFAULT_CONFIG_FIELD_VALUE ) )
    {
        printf( ". \"%s\" value's is still the default value.\n\r", config_key );
        printf( "   Did you forget to edit the configuration file \"%s\"?\n\r",
                XIVELY_CFG_FILE );
        return -1;
    }

    if ( strlen( value ) > XIVELY_CRED_LEN_MAX )
    {
        printf( ". \"%s\" value's string is too long for internal buffer configuration. "
                "value is: \"%s\"\n\r",
                config_key, value );
        printf( ". Max String Length is: %d\n\r", XIVELY_CRED_LEN_MAX );
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
        printf( "Now Wifi Security Type String found in configuration file!\n\r" );
        return -1;
    }

    if ( strcmp( "open", security_type ) == 0 || strcmp( "OPEN", security_type ) == 0 )
    {
        return SL_SEC_TYPE_OPEN;
    }

    if ( strcmp( "wep", security_type ) == 0 || strcmp( "WEP", security_type ) == 0 )
    {
        return SL_SEC_TYPE_WEP;
    }

    if ( strcmp( "wpa", security_type ) == 0 || strcmp( "WPA", security_type ) == 0 )
    {
        return SL_SEC_TYPE_WPA;
    }

    if ( strcmp( "wpa2", security_type ) == 0 || strcmp( "WPA2", security_type ) == 0 )
    {
        return SL_SEC_TYPE_WPA_WPA2;
    }

    if ( strcmp( "wpa_ent", security_type ) == 0 ||
         strcmp( "WPA_ENT", security_type ) == 0 )
    {
        return SL_SEC_TYPE_WPA_ENT;
    }

    /* potential future support based on CC3220 SDK:
     *
     * SL_SEC_TYPE_WPS_PBC
     * SL_SEC_TYPE_WPS_PIN
     * SL_SEC_TYPE_P2P_PBC
     * SL_SEC_TYPE_P2P_PIN_KEYPAD
     * SL_SEC_TYPE_P2P_PIN_DISPLAY
     * SL_SEC_TYPE_P2P_PIN_AUTO
     */
    printf( "Unsupported Wifi security type: \"%s\"\n\r", security_type );
    return -1;
}

/*****************************************************************************
 * These functions are from the ent_wlan example of the CC3200 SDK. They exist in this
 * project to facilitate state tracking of the WiFi connection for the device and are not
 * Xively specific.
 *
 * If you would like more information and documentation on these functions, please see the
 *source code in the directory example/ent_wlan of your CC3200 SDK installation.
 ******************************************************************************/
void SimpleLinkWlanEventHandler( SlWlanEvent_t* pWlanEvent )
{
    if ( !pWlanEvent )
    {
        return;
    }

    switch ( pWlanEvent->Event )
    {
        case SL_WLAN_CONNECT_EVENT:
        {
            SET_STATUS_BIT( g_ulStatus, STATUS_BIT_CONNECTION );

            // Copy new connection SSID and BSSID to global parameters
            memcpy( g_ucConnectionSSID,
                    pWlanEvent->EventData.STAandP2PModeWlanConnected.ssid_name,
                    pWlanEvent->EventData.STAandP2PModeWlanConnected.ssid_len );
            memcpy( g_ucConnectionBSSID,
                    pWlanEvent->EventData.STAandP2PModeWlanConnected.bssid,
                    SL_BSSID_LENGTH );

            printf( "[WLAN EVENT] STA Connected to the AP: %s , "
                    "BSSID: %x:%x:%x:%x:%x:%x\n",
                    g_ucConnectionSSID, g_ucConnectionBSSID[0], g_ucConnectionBSSID[1],
                    g_ucConnectionBSSID[2], g_ucConnectionBSSID[3],
                    g_ucConnectionBSSID[4], g_ucConnectionBSSID[5] );
        }
        break;

        case SL_WLAN_DISCONNECT_EVENT:
        {
            slWlanConnectAsyncResponse_t* pEventData = NULL;

            CLR_STATUS_BIT( g_ulStatus, STATUS_BIT_CONNECTION );
            CLR_STATUS_BIT( g_ulStatus, STATUS_BIT_IP_AQUIRED );

            pEventData = &pWlanEvent->EventData.STAandP2PModeDisconnected;

            /* If the user has initiated 'Disconnect' request,
             * 'reason_code' is SL_WLAN_DISCONNECT_USER_INITIATED_DISCONNECTION */
            if ( SL_USER_INITIATED_DISCONNECTION == pEventData->reason_code )
            {
                printf( "[WLAN EVENT]Device disconnected from the AP: %s, "
                        "BSSID: %x:%x:%x:%x:%x:%x on application's request \n",
                        g_ucConnectionSSID, g_ucConnectionBSSID[0],
                        g_ucConnectionBSSID[1], g_ucConnectionBSSID[2],
                        g_ucConnectionBSSID[3], g_ucConnectionBSSID[4],
                        g_ucConnectionBSSID[5] );
            }
            else
            {
                printf( "[WLAN ERROR]Device disconnected from the AP AP: %s,"
                        "BSSID: %x:%x:%x:%x:%x:%x on an ERROR..!! \n",
                        g_ucConnectionSSID, g_ucConnectionBSSID[0],
                        g_ucConnectionBSSID[1], g_ucConnectionBSSID[2],
                        g_ucConnectionBSSID[3], g_ucConnectionBSSID[4],
                        g_ucConnectionBSSID[5] );
            }
            memset( g_ucConnectionSSID, 0, sizeof( g_ucConnectionSSID ) );
            memset( g_ucConnectionBSSID, 0, sizeof( g_ucConnectionBSSID ) );
        }
        break;

        default:
        {
            printf( "[WLAN EVENT] Unexpected event [0x%x]\n", pWlanEvent->Event );
        }
        break;
    }
}

void SimpleLinkNetAppEventHandler( SlNetAppEvent_t* pNetAppEvent )
{
    if ( !pNetAppEvent )
    {
        return;
    }

    switch ( pNetAppEvent->Event )
    {
        case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
        {
            SlIpV4AcquiredAsync_t* pEventData = NULL;

            SET_STATUS_BIT( g_ulStatus, STATUS_BIT_IP_AQUIRED );

            /* Ip Acquired Event Data */
            pEventData = &pNetAppEvent->EventData.ipAcquiredV4;

            /* Gateway IP address */
            g_ulGatewayIP = pEventData->gateway;

            printf( "[NETAPP EVENT] IP Acquired: IP=%d.%d.%d.%d ,"
                    "Gateway=%d.%d.%d.%d\n",
                    SL_IPV4_BYTE( pNetAppEvent->EventData.ipAcquiredV4.ip, 3 ),
                    SL_IPV4_BYTE( pNetAppEvent->EventData.ipAcquiredV4.ip, 2 ),
                    SL_IPV4_BYTE( pNetAppEvent->EventData.ipAcquiredV4.ip, 1 ),
                    SL_IPV4_BYTE( pNetAppEvent->EventData.ipAcquiredV4.ip, 0 ),
                    SL_IPV4_BYTE( pNetAppEvent->EventData.ipAcquiredV4.gateway, 3 ),
                    SL_IPV4_BYTE( pNetAppEvent->EventData.ipAcquiredV4.gateway, 2 ),
                    SL_IPV4_BYTE( pNetAppEvent->EventData.ipAcquiredV4.gateway, 1 ),
                    SL_IPV4_BYTE( pNetAppEvent->EventData.ipAcquiredV4.gateway, 0 ) );
        }
        break;

        default:
        {
            printf( "[NETAPP EVENT] Unexpected event [0x%x] \n", pNetAppEvent->Event );
        }
        break;
    }
}

void SimpleLinkHttpServerCallback( SlHttpServerEvent_t* pHttpEvent,
                                   SlHttpServerResponse_t* pHttpResponse )
{
    /* Unused in this application */
}

void SimpleLinkGeneralEventHandler( SlDeviceEvent_t* pDevEvent )
{
    printf( "[GENERAL EVENT] - ID=[%d] Sender=[%d]\n\n",
            pDevEvent->EventData.deviceEvent.status,
            pDevEvent->EventData.deviceEvent.sender );
}

void SimpleLinkSockEventHandler( SlSockEvent_t* pSock )
{
    if ( !pSock )
    {
        return;
    }

    /* This application doesn't work w/ socket - Events are not expected */
    switch ( pSock->Event )
    {
        case SL_SOCKET_TX_FAILED_EVENT:
            switch ( pSock->socketAsyncEvent.SockTxFailData.status )
            {
                case SL_ECLOSE:
                    printf( "[SOCK ERROR] - close socket (%d) operation "
                            "failed to transmit all queued packets\n\n",
                            pSock->socketAsyncEvent.SockTxFailData.sd );
                    break;
                default:
                    printf( "[SOCK ERROR] - TX FAILED  :  socket %d , reason "
                            "(%d) \n\n",
                            pSock->socketAsyncEvent.SockTxFailData.sd,
                            pSock->socketAsyncEvent.SockTxFailData.status );
                    break;
            }
            break;

        default:
            printf( "[SOCK EVENT] - Unexpected Event [%x0x]\n\n", pSock->Event );
            break;
    }
}

_SlEventPropogationStatus_e
sl_Provisioning_HttpServerEventHdl( SlHttpServerEvent_t* apSlHttpServerEvent,
                                    SlHttpServerResponse_t* apSlHttpServerResponse )
{
    /* Unused in this application */
    return EVENT_PROPAGATION_CONTINUE;
}

_SlEventPropogationStatus_e
sl_Provisioning_NetAppEventHdl( SlNetAppEvent_t* apNetAppEvent )
{
    /* Unused in this application */
    return EVENT_PROPAGATION_CONTINUE;
}

_SlEventPropogationStatus_e sl_Provisioning_WlanEventHdl( SlWlanEvent_t* apEventInfo )
{
    /* Unused in this application */
    return EVENT_PROPAGATION_CONTINUE;
}

static void InitializeAppVariables()
{
    g_ulStatus    = 0;
    g_ulGatewayIP = 0;
    memset( g_ucConnectionSSID, 0, sizeof( g_ucConnectionSSID ) );
    memset( g_ucConnectionBSSID, 0, sizeof( g_ucConnectionBSSID ) );
}

static long ConfigureSimpleLinkToDefaultState()
{
    SlVersionFull ver                                  = {0};
    _WlanRxFilterOperationCommandBuff_t RxFilterIdMask = {0};

    unsigned char ucVal       = 1;
    unsigned char ucConfigOpt = 0;
    unsigned char ucConfigLen = 0;
    unsigned char ucPower     = 0;

    long lRetVal = -1;
    long lMode   = -1;

    lMode = sl_Start( 0, 0, 0 );
    ASSERT_ON_ERROR( lMode );

    /* If the device is not in station-mode, try configuring it in station-mode */
    if ( ROLE_STA != lMode )
    {
        if ( ROLE_AP == lMode )
        {
            /* If the device is in AP mode, we need to wait for this event
             * before doing anything */
            while ( !IS_IP_ACQUIRED( g_ulStatus ) )
            {
#ifndef SL_PLATFORM_MULTI_THREADED
                _SlNonOsMainLoopTask();
#endif
            }
        }

        /* Switch to STA role and restart */
        lRetVal = sl_WlanSetMode( ROLE_STA );
        ASSERT_ON_ERROR( lRetVal );

        lRetVal = sl_Stop( 0xFF );
        ASSERT_ON_ERROR( lRetVal );

        lRetVal = sl_Start( 0, 0, 0 );
        ASSERT_ON_ERROR( lRetVal );

        /* Check if the device is in station again */
        if ( ROLE_STA != lRetVal )
        {
            /* We don't want to proceed if the device is not coming up in STA-mode */
            return DEVICE_NOT_IN_STATION_MODE;
        }
    }

    /* Get the device's version-information */
    ucConfigOpt = SL_DEVICE_GENERAL_VERSION;
    ucConfigLen = sizeof( ver );
    lRetVal     = sl_DevGet( SL_DEVICE_GENERAL_CONFIGURATION, &ucConfigOpt, &ucConfigLen,
                         ( unsigned char* )( &ver ) );
    ASSERT_ON_ERROR( lRetVal );

    printf( "Host Driver Version: %s\n", SL_DRIVER_VERSION );
    printf( "Build Version %d.%d.%d.%d.31.%d.%d.%d.%d.%d.%d.%d.%d\n", ver.NwpVersion[0],
            ver.NwpVersion[1], ver.NwpVersion[2], ver.NwpVersion[3],
            ver.ChipFwAndPhyVersion.FwVersion[0], ver.ChipFwAndPhyVersion.FwVersion[1],
            ver.ChipFwAndPhyVersion.FwVersion[2], ver.ChipFwAndPhyVersion.FwVersion[3],
            ver.ChipFwAndPhyVersion.PhyVersion[0], ver.ChipFwAndPhyVersion.PhyVersion[1],
            ver.ChipFwAndPhyVersion.PhyVersion[2],
            ver.ChipFwAndPhyVersion.PhyVersion[3] );

    /* Set connection policy to Auto + SmartConfig
     *    (Device's default connection policy) */
    lRetVal = sl_WlanPolicySet( SL_POLICY_CONNECTION,
                                SL_CONNECTION_POLICY( 1, 0, 0, 0, 1 ), NULL, 0 );
    ASSERT_ON_ERROR( lRetVal );

    /* Remove all profiles */
    lRetVal = sl_WlanProfileDel( 0xFF );
    ASSERT_ON_ERROR( lRetVal );

    lRetVal = sl_WlanDisconnect();
    if ( 0 == lRetVal )
    {
        /* Wait */
        while ( IS_CONNECTED( g_ulStatus ) )
        {
#ifndef SL_PLATFORM_MULTI_THREADED
            _SlNonOsMainLoopTask();
#endif
        }
    }

    /* Enable DHCP client */
    lRetVal = sl_NetCfgSet( SL_IPV4_STA_P2P_CL_DHCP_ENABLE, 1, 1, &ucVal );
    ASSERT_ON_ERROR( lRetVal );

    /* Disable scan */
    ucConfigOpt = SL_SCAN_POLICY( 0 );
    lRetVal     = sl_WlanPolicySet( SL_POLICY_SCAN, ucConfigOpt, NULL, 0 );
    ASSERT_ON_ERROR( lRetVal );

    /* Set Tx power level for station mode
     * Number between 0-15, as dB offset from max power - 0 will set max power */
    ucPower = 0;
    lRetVal =
        sl_WlanSet( SL_WLAN_CFG_GENERAL_PARAM_ID, WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1,
                    ( unsigned char* )&ucPower );
    ASSERT_ON_ERROR( lRetVal );

    /* Set PM policy to normal */
    lRetVal = sl_WlanPolicySet( SL_POLICY_PM, SL_NORMAL_POLICY, NULL, 0 );
    ASSERT_ON_ERROR( lRetVal );

    /* Unregister mDNS services */
    lRetVal = sl_NetAppMDNSUnRegisterService( 0, 0 );
    ASSERT_ON_ERROR( lRetVal );

    /* Remove  all 64 filters (8*8) */
    memset( RxFilterIdMask.FilterIdMask, 0xFF, 8 );
    lRetVal = sl_WlanRxFilterSet( SL_REMOVE_RX_FILTER, ( _u8* )&RxFilterIdMask,
                                  sizeof( _WlanRxFilterOperationCommandBuff_t ) );
    ASSERT_ON_ERROR( lRetVal );

    lRetVal = sl_Stop( SL_STOP_TIMEOUT );
    ASSERT_ON_ERROR( lRetVal );

    InitializeAppVariables();

    return lRetVal; /* Success */
}

static void BoardInit( void )
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS


/* Set vector table base */
#if defined( ccs )
    MAP_IntVTableBaseSet( ( unsigned long )&g_pfnVectors[0] );
#endif
#if defined( ewarm )
    MAP_IntVTableBaseSet( ( unsigned long )&__vector_table );
#endif
#endif

    /* Enable Processor */
    MAP_IntMasterEnable();
    MAP_IntEnable( FAULT_SYSTICK );

    PRCMCC3200MCUInit();
}

void WaitForWlanEvent()
{
    /* Wait for WLAN Event */
    while ( ( !IS_CONNECTED( g_ulStatus ) ) || ( !IS_IP_ACQUIRED( g_ulStatus ) ) )
    {
        /* Toggle LEDs to Indicate Connection Progress */
        _SlNonOsMainLoopTask();
        MAP_UtilsDelay( 2000000 );
        GPIO_IF_LedOn( MCU_ALL_LED_IND );
        _SlNonOsMainLoopTask();
        MAP_UtilsDelay( 2000000 );
        GPIO_IF_LedOff( MCU_ALL_LED_IND );
    }
}
