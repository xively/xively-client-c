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

// Simplelink includes
#include "simplelink.h"

// Driverlib includes
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

// Common interface includes
#include "button_if.h"
#include "common.h"
#include "gpio_if.h"
#include "i2c_if.h"
#include "pinmux.h"
#include "tmp006drv.h"

// required by sprintf
#include <stdio.h>

#ifndef NOTERM
#include "uart_if.h"
#endif

#define XIVELY_DEMO_PRINT( ... )                                                         \
    UART_PRINT( __VA_ARGS__ );                                                           \
    printf( __VA_ARGS__ );

#define APPLICATION_NAME "XIVELY_CC3200_DEMO_APP"
#define APPLICATION_VERSION "0.0.1"
#define SUCCESS 0

// Values for below macros shall be modified per the access-point's (AP) properties.
// SimpleLink device will connect to following AP when the application is executed.
#define ENT_NAME "YOUR_WIFI_SSID"
#define PASSWORD "YOUR_WIFI_PASSWORD"

// Values for below macros will be used for connecting the device to Xively's
// MQTT broker
#define XIVELY_DEVICE_ID "PASTE_YOUR_XIVELY_DEVICE_ID"
#define XIVELY_DEVICE_SECRET "PASTE_YOUR_XIVELY_DEVICE_SECRET"
#define XIVELY_ACCOUNT_ID "PASTE_YOUR_XIVELY_ACCOUNT_ID"

#define ENABLE_XIVELY_FIRMWARE_UPDATE_AND_SECURE_FILE_TRANSFER 1

// Application specific status/error codes
typedef enum {
    // Choosing -0x7D0 to avoid overlap w/ host-driver's error codes
    LAN_CONNECTION_FAILED      = -0x7D0,
    DEVICE_NOT_IN_STATION_MODE = LAN_CONNECTION_FAILED - 1,
    STATUS_CODE_MAX            = -0xBB8
} e_AppStatusCodes;

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
volatile unsigned long g_ulStatus = 0;              // SimpleLink Status
unsigned long g_ulGatewayIP       = 0;              // Network Gateway IP address
unsigned char g_ucConnectionSSID[SSID_LEN_MAX + 1]; // Connection SSID
unsigned char g_ucConnectionBSSID[BSSID_LEN_MAX];   // Connection BSSID

#if defined( ccs )
extern void ( *const g_pfnVectors[] )( void );
#endif
#if defined( ewarm )
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************

//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//****************************************************************************
long MainLogic();
static void BoardInit( void );
static void InitializeAppVariables();
void ConnectToXively();
void WaitForWlanEvent();

/******************************************************************************/
/* BEGINNING OF XIVELY CODE */
/******************************************************************************/

/* Includes used by the code below. */
#include <time.h>
#include <xi_bsp_rng.h>
#include <xi_bsp_time.h>
#include <xively.h>

/******************************************************************************/
/* BEGINNING OF XIVELY CODE GLOBAL SECTION */
/******************************************************************************/

/* Names of the MQTT topics used by this application */
const char* const gTemperatureTopicName =
    "xi/blue/v1/" XIVELY_ACCOUNT_ID "/d/" XIVELY_DEVICE_ID "/Temperature";
const char* const gGreenLedTopicName =
    "xi/blue/v1/" XIVELY_ACCOUNT_ID "/d/" XIVELY_DEVICE_ID "/Green LED";
const char* const gOrangeLedTopicName =
    "xi/blue/v1/" XIVELY_ACCOUNT_ID "/d/" XIVELY_DEVICE_ID "/Orange LED";
const char* const gRedLedTopicName =
    "xi/blue/v1/" XIVELY_ACCOUNT_ID "/d/" XIVELY_DEVICE_ID "/Red LED";
const char* const gButtonSW2TopicName =
    "xi/blue/v1/" XIVELY_ACCOUNT_ID "/d/" XIVELY_DEVICE_ID "/Button SW2";
const char* const gButtonSW3TopicName =
    "xi/blue/v1/" XIVELY_ACCOUNT_ID "/d/" XIVELY_DEVICE_ID "/Button SW3";

/* Required by push button interrupt handlers - there is no space for user argument so we
 * can't pass the xively's context handler as a function argument. */
xi_context_handle_t gXivelyContextHandle = -1;

/* Used to store the temperature timed task handles. */
xi_timed_task_handle_t gTemperatureTaskHandle = -1;

/* Used by onLed handler function to differentiate LED's */
const static ledNames gGreenLed  = MCU_GREEN_LED_GPIO;
const static ledNames gOrangeLed = MCU_ORANGE_LED_GPIO;
const static ledNames gRedLed    = MCU_RED_LED_GPIO;

/******************************************************************************/
/* END OF XIVELY CODE GLOBAL SECTION */
/******************************************************************************/

/**
* @brief Event handler for SW2 button press/release interrupts.
 *
 * The pinValue will be 1 on a button press event, 0 on button up..
 */
void pushButtonInterruptHandlerSW2()
{
    int pinValue = GPIOPinRead( GPIOA2_BASE, GPIO_PIN_6 );

    if ( pinValue == 0 )
    {
        xi_publish( gXivelyContextHandle, gButtonSW2TopicName, "0",
                    XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
    }
    else
    {
        xi_publish( gXivelyContextHandle, gButtonSW2TopicName, "1",
                    XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
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
    int pinValue = GPIOPinRead( GPIOA1_BASE, GPIO_PIN_5 );

    if ( pinValue == 0 )
    {
        xi_publish( gXivelyContextHandle, gButtonSW3TopicName, "0",
                    XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
    }
    else
    {
        xi_publish( gXivelyContextHandle, gButtonSW3TopicName, "1",
                    XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
    }

    Button_IF_EnableInterrupt( SW3 );
}

/**
 * @brief Function that handles messages sent to the device on the LEDs' topics,
 * including toggle commands and subscription changes.
 *
 * @note For more details please refer to the xively.h - xi_subscribe function.
 *
 * @param in_context_handle - xively library context required for calling API functions
 * @param call_type - describes if the handler was called carrying the subscription
 * operation result or the incoming message
 * @oaram params - structure with data
 * @param state - state of the operation
 * @oaram user_data - Pointer provided upon LedTopic subscription. For this application's
 * use, these are the LED names.
 */
void onLedTopic( xi_context_handle_t in_context_handle,
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
                XIVELY_DEMO_PRINT( "topic:%s. Subscription failed.\n",
                                   params->suback.topic );
            }
            else
            {
                XIVELY_DEMO_PRINT( "topic:%s. Subscription granted %d.\n",
                                   params->suback.topic,
                                   ( int )params->suback.suback_status );
            }
            return;
        case XI_SUB_CALL_MESSAGE:
            if ( params->message.temporary_payload_data_length == 1 )
            {
                ledNames ledName = *( ( ledNames* )user_data );

                switch ( params->message.temporary_payload_data[0] )
                {
                    case 48:
                        GPIO_IF_LedOff( ledName );
                        break;
                    case 49:
                        GPIO_IF_LedOn( ledName );
                        break;
                    default:
                        XIVELY_DEMO_PRINT( "unexpected value on topic %s \n",
                                           params->message.topic );
                        break;
                }
            }
            else
            {
                XIVELY_DEMO_PRINT( "unexpected data length on topic %s \n",
                                   params->message.topic );
            }
            return;
        default:
            return;
    }
}

/**
 * @brief Recurring time task that temporarily activates the device's temperature sensor,
 * publishes the result on the temperature topic for the device, an then re-enables LED
 * control.
 *
 * @note for more details about user time tasks please refer to the xively.h -
 * xi_schedule_timed_task function.
 *
 * @param context_handle - xively library context required for calling API functions
 * @param timed_task_handle - handle to *this* timed task, can be used to re-schedule or
 * cancel the task
 * @param user_data - data assosicated with this timed task
 */
void send_temperature( const xi_context_handle_t context_handle,
                       const xi_timed_task_handle_t timed_task_handle,
                       void* user_data )
{
    /* Prepare configuration for temperature sensor for I2C bus */
    PinTypeI2C( PIN_01, PIN_MODE_1 );
    PinTypeI2C( PIN_02, PIN_MODE_1 );

    int lRetVal        = -1;
    float fCurrentTemp = .0f;
    char msg[16]       = {'\0'};

    /* I2C Init */
    lRetVal = I2C_IF_Open( I2C_MASTER_MODE_FST );
    if ( lRetVal < 0 )
    {
        ERR_PRINT( lRetVal );
        goto end;
    }

    /* Init Temperature Sensor */
    lRetVal = TMP006DrvOpen();
    if ( lRetVal < 0 )
    {
        ERR_PRINT( lRetVal );
        goto end;
    }

    TMP006DrvGetTemp( &fCurrentTemp );

    sprintf( ( char* )&msg, "%5.02f", fCurrentTemp );
    xi_publish( context_handle, gTemperatureTopicName, msg, XI_MQTT_QOS_AT_MOST_ONCE,
                XI_MQTT_RETAIN_FALSE, NULL, NULL );

    do
    {
    } while ( I2C_IF_Close() != 0 );

end:
    /* Restore configuration for LED control */
    PinTypeGPIO( PIN_01, PIN_MODE_0, false );
    GPIODirModeSet( GPIOA1_BASE, 0x4, GPIO_DIR_MODE_OUT );

    PinTypeGPIO( PIN_02, PIN_MODE_0, false );
    GPIODirModeSet( GPIOA1_BASE, 0x8, GPIO_DIR_MODE_OUT );
}

/**
 * @brief Handler function for eventa related to changes in connection state.
 *
 * @note for more details please refer to xively.h - xi_connect function.
 *
 * @param context_handle - xively library context required for calling API functions
 * @param data - points to the structure that holds detailed information about the
 * connection state and settings
 * @param state - additional internal state of the library helps to determine the reason
 * of connection failure or disconnection
 */
void on_connected( xi_context_handle_t in_context_handle, void* data, xi_state_t state )
{
    xi_connection_data_t* conn_data = ( xi_connection_data_t* )data;

    switch ( conn_data->connection_state )
    {
        case XI_CONNECTION_STATE_OPEN_FAILED:
            XIVELY_DEMO_PRINT( "connection to %s:%d has failed reason %d\n",
                               conn_data->host, conn_data->port, state );

            xi_connect( in_context_handle, conn_data->username, conn_data->password,
                        conn_data->connection_timeout, conn_data->keepalive_timeout,
                        conn_data->session_type, &on_connected );

            return;
        case XI_CONNECTION_STATE_OPENED:
            XIVELY_DEMO_PRINT( "connected to %s:%d\n", conn_data->host, conn_data->port );

            /* register a function to publish temperature data every 5 seconds */
            gTemperatureTaskHandle =
                xi_schedule_timed_task( in_context_handle, send_temperature, 5, 1, NULL );

            if ( XI_INVALID_TIMED_TASK_HANDLE == gTemperatureTaskHandle )
            {
                XIVELY_DEMO_PRINT( "send_temperature_task couldn't be registered\n" );
            }

            /* subscribe to LED topics to listen for light toggle commands */
            xi_subscribe( in_context_handle, gGreenLedTopicName, XI_MQTT_QOS_AT_MOST_ONCE,
                          onLedTopic, ( void* )&gGreenLed );
            xi_subscribe( in_context_handle, gOrangeLedTopicName,
                          XI_MQTT_QOS_AT_MOST_ONCE, onLedTopic, ( void* )&gOrangeLed );
            xi_subscribe( in_context_handle, gRedLedTopicName, XI_MQTT_QOS_AT_MOST_ONCE,
                          onLedTopic, ( void* )&gRedLed );

            /* enable the button interrupts */
            Button_IF_EnableInterrupt( SW2 | SW3 );

            break;
        case XI_CONNECTION_STATE_CLOSED:
            XIVELY_DEMO_PRINT( "connection closed - reason %d!\n", state );

            /* cancel timed task - we don't want to send messages while the library not
             * connected */
            xi_cancel_timed_task( gTemperatureTaskHandle );
            gTemperatureTaskHandle = XI_INVALID_TIMED_TASK_HANDLE;

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
            XIVELY_DEMO_PRINT( "invalid parameter %d\n", conn_data->connection_state );
            return;
    }
}

/**
 * @brief Driver for the xively library logic
 *
 * This function initialises the library, creates a context and starts the internal event
 * dispatcher and event processing of xively library.
 *
 * This function is blocking for the sakness of this demo application however it is
 * possible to run xively's event processing with limit of iterations.
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

    xi_state_t ret_state = xi_initialize( XIVELY_ACCOUNT_ID );

    if ( XI_STATE_OK != ret_state )
    {
        XIVELY_DEMO_PRINT( "xi failed to initialise\n" );
        return;
    }

    gXivelyContextHandle = xi_create_context();

    if ( XI_INVALID_CONTEXT_HANDLE == gXivelyContextHandle )
    {
        XIVELY_DEMO_PRINT( " xi failed to create context, error: %d\n",
                           -gXivelyContextHandle );
        return;
    }

#if ENABLE_XIVELY_FIRMWARE_UPDATE_AND_SECURE_FILE_TRANSFER
    /* Pass list of files to be updated by the Xively Services. */
    const char** files_to_keep_updated =
        ( const char* [] ){"firmware.bin", "credentials.cfg"};

    xi_set_updateable_files( gXivelyContextHandle, files_to_keep_updated, 2, NULL );
#endif

    xi_state_t connect_result =
        xi_connect( gXivelyContextHandle, XIVELY_DEVICE_ID, XIVELY_DEVICE_SECRET, 10, 0,
                    XI_SESSION_CLEAN, &on_connected );

    /* start processing xively library events */
    xi_events_process_blocking();

    /* when the event dispatcher is stop destroy the context */
    xi_delete_context( gXivelyContextHandle );

    /* shutdown the xively library */
    xi_shutdown();
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

/******************************************************************************/
/* END OF XIVELY CODE */
/******************************************************************************/

//*****************************************************************************
// These functions are from the ent_wlan example of the CC3200 SDK. They exist in this
// project to facilitate state tracking of the WiFi connection for the device and are not
// Xively specific.
//
// If you would like more information and documentation on these functions, please see the
// source code in the directory example/ent_wlan of your CC3200 SDK installation.
//*****************************************************************************

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

            XIVELY_DEMO_PRINT( "[WLAN EVENT] STA Connected to the AP: %s , "
                               "BSSID: %x:%x:%x:%x:%x:%x\n",
                               g_ucConnectionSSID, g_ucConnectionBSSID[0],
                               g_ucConnectionBSSID[1], g_ucConnectionBSSID[2],
                               g_ucConnectionBSSID[3], g_ucConnectionBSSID[4],
                               g_ucConnectionBSSID[5] );
        }
        break;

        case SL_WLAN_DISCONNECT_EVENT:
        {
            slWlanConnectAsyncResponse_t* pEventData = NULL;

            CLR_STATUS_BIT( g_ulStatus, STATUS_BIT_CONNECTION );
            CLR_STATUS_BIT( g_ulStatus, STATUS_BIT_IP_AQUIRED );

            pEventData = &pWlanEvent->EventData.STAandP2PModeDisconnected;

            // If the user has initiated 'Disconnect' request,
            //'reason_code' is SL_WLAN_DISCONNECT_USER_INITIATED_DISCONNECTION
            if ( SL_USER_INITIATED_DISCONNECTION == pEventData->reason_code )
            {
                XIVELY_DEMO_PRINT( "[WLAN EVENT]Device disconnected from the AP: %s, "
                                   "BSSID: %x:%x:%x:%x:%x:%x on application's request \n",
                                   g_ucConnectionSSID, g_ucConnectionBSSID[0],
                                   g_ucConnectionBSSID[1], g_ucConnectionBSSID[2],
                                   g_ucConnectionBSSID[3], g_ucConnectionBSSID[4],
                                   g_ucConnectionBSSID[5] );
            }
            else
            {
                XIVELY_DEMO_PRINT( "[WLAN ERROR]Device disconnected from the AP AP: %s,"
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
            XIVELY_DEMO_PRINT( "[WLAN EVENT] Unexpected event [0x%x]\n",
                               pWlanEvent->Event );
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

            // Ip Acquired Event Data
            pEventData = &pNetAppEvent->EventData.ipAcquiredV4;

            // Gateway IP address
            g_ulGatewayIP = pEventData->gateway;

            XIVELY_DEMO_PRINT(
                "[NETAPP EVENT] IP Acquired: IP=%d.%d.%d.%d ,"
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
            XIVELY_DEMO_PRINT( "[NETAPP EVENT] Unexpected event [0x%x] \n",
                               pNetAppEvent->Event );
        }
        break;
    }
}

void SimpleLinkHttpServerCallback( SlHttpServerEvent_t* pHttpEvent,
                                   SlHttpServerResponse_t* pHttpResponse )
{
    // Unused in this application
}

void SimpleLinkGeneralEventHandler( SlDeviceEvent_t* pDevEvent )
{
    XIVELY_DEMO_PRINT( "[GENERAL EVENT] - ID=[%d] Sender=[%d]\n\n",
                       pDevEvent->EventData.deviceEvent.status,
                       pDevEvent->EventData.deviceEvent.sender );
}

void SimpleLinkSockEventHandler( SlSockEvent_t* pSock )
{
    if ( !pSock )
    {
        return;
    }

    //
    // This application doesn't work w/ socket - Events are not expected
    //
    switch ( pSock->Event )
    {
        case SL_SOCKET_TX_FAILED_EVENT:
            switch ( pSock->socketAsyncEvent.SockTxFailData.status )
            {
                case SL_ECLOSE:
                    XIVELY_DEMO_PRINT( "[SOCK ERROR] - close socket (%d) operation "
                                       "failed to transmit all queued packets\n\n",
                                       pSock->socketAsyncEvent.SockTxFailData.sd );
                    break;
                default:
                    XIVELY_DEMO_PRINT( "[SOCK ERROR] - TX FAILED  :  socket %d , reason "
                                       "(%d) \n\n",
                                       pSock->socketAsyncEvent.SockTxFailData.sd,
                                       pSock->socketAsyncEvent.SockTxFailData.status );
                    break;
            }
            break;

        default:
            XIVELY_DEMO_PRINT( "[SOCK EVENT] - Unexpected Event [%x0x]\n\n",
                               pSock->Event );
            break;
    }
}

_SlEventPropogationStatus_e
sl_Provisioning_HttpServerEventHdl( SlHttpServerEvent_t* apSlHttpServerEvent,
                                    SlHttpServerResponse_t* apSlHttpServerResponse )
{
    // Unused in this application
    return EVENT_PROPAGATION_CONTINUE;
}

_SlEventPropogationStatus_e
sl_Provisioning_NetAppEventHdl( SlNetAppEvent_t* apNetAppEvent )
{
    // Unused in this application
    return EVENT_PROPAGATION_CONTINUE;
}

_SlEventPropogationStatus_e sl_Provisioning_WlanEventHdl( SlWlanEvent_t* apEventInfo )
{
    // Unused in this application
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

    // If the device is not in station-mode, try configuring it in station-mode
    if ( ROLE_STA != lMode )
    {
        if ( ROLE_AP == lMode )
        {
            // If the device is in AP mode, we need to wait for this event
            // before doing anything
            while ( !IS_IP_ACQUIRED( g_ulStatus ) )
            {
#ifndef SL_PLATFORM_MULTI_THREADED
                _SlNonOsMainLoopTask();
#endif
            }
        }

        // Switch to STA role and restart
        lRetVal = sl_WlanSetMode( ROLE_STA );
        ASSERT_ON_ERROR( lRetVal );

        lRetVal = sl_Stop( 0xFF );
        ASSERT_ON_ERROR( lRetVal );

        lRetVal = sl_Start( 0, 0, 0 );
        ASSERT_ON_ERROR( lRetVal );

        // Check if the device is in station again
        if ( ROLE_STA != lRetVal )
        {
            // We don't want to proceed if the device is not coming up in STA-mode
            return DEVICE_NOT_IN_STATION_MODE;
        }
    }

    // Get the device's version-information
    ucConfigOpt = SL_DEVICE_GENERAL_VERSION;
    ucConfigLen = sizeof( ver );
    lRetVal     = sl_DevGet( SL_DEVICE_GENERAL_CONFIGURATION, &ucConfigOpt, &ucConfigLen,
                         ( unsigned char* )( &ver ) );
    ASSERT_ON_ERROR( lRetVal );

    XIVELY_DEMO_PRINT( "Host Driver Version: %s\n", SL_DRIVER_VERSION );
    XIVELY_DEMO_PRINT(
        "Build Version %d.%d.%d.%d.31.%d.%d.%d.%d.%d.%d.%d.%d\n", ver.NwpVersion[0],
        ver.NwpVersion[1], ver.NwpVersion[2], ver.NwpVersion[3],
        ver.ChipFwAndPhyVersion.FwVersion[0], ver.ChipFwAndPhyVersion.FwVersion[1],
        ver.ChipFwAndPhyVersion.FwVersion[2], ver.ChipFwAndPhyVersion.FwVersion[3],
        ver.ChipFwAndPhyVersion.PhyVersion[0], ver.ChipFwAndPhyVersion.PhyVersion[1],
        ver.ChipFwAndPhyVersion.PhyVersion[2], ver.ChipFwAndPhyVersion.PhyVersion[3] );

    // Set connection policy to Auto + SmartConfig
    //      (Device's default connection policy)
    lRetVal = sl_WlanPolicySet( SL_POLICY_CONNECTION,
                                SL_CONNECTION_POLICY( 1, 0, 0, 0, 1 ), NULL, 0 );
    ASSERT_ON_ERROR( lRetVal );

    // Remove all profiles
    lRetVal = sl_WlanProfileDel( 0xFF );
    ASSERT_ON_ERROR( lRetVal );

    lRetVal = sl_WlanDisconnect();
    if ( 0 == lRetVal )
    {
        // Wait
        while ( IS_CONNECTED( g_ulStatus ) )
        {
#ifndef SL_PLATFORM_MULTI_THREADED
            _SlNonOsMainLoopTask();
#endif
        }
    }

    // Enable DHCP client
    lRetVal = sl_NetCfgSet( SL_IPV4_STA_P2P_CL_DHCP_ENABLE, 1, 1, &ucVal );
    ASSERT_ON_ERROR( lRetVal );

    // Disable scan
    ucConfigOpt = SL_SCAN_POLICY( 0 );
    lRetVal     = sl_WlanPolicySet( SL_POLICY_SCAN, ucConfigOpt, NULL, 0 );
    ASSERT_ON_ERROR( lRetVal );

    // Set Tx power level for station mode
    // Number between 0-15, as dB offset from max power - 0 will set max power
    ucPower = 0;
    lRetVal =
        sl_WlanSet( SL_WLAN_CFG_GENERAL_PARAM_ID, WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1,
                    ( unsigned char* )&ucPower );
    ASSERT_ON_ERROR( lRetVal );

    // Set PM policy to normal
    lRetVal = sl_WlanPolicySet( SL_POLICY_PM, SL_NORMAL_POLICY, NULL, 0 );
    ASSERT_ON_ERROR( lRetVal );

    // Unregister mDNS services
    lRetVal = sl_NetAppMDNSUnRegisterService( 0, 0 );
    ASSERT_ON_ERROR( lRetVal );

    // Remove  all 64 filters (8*8)
    memset( RxFilterIdMask.FilterIdMask, 0xFF, 8 );
    lRetVal = sl_WlanRxFilterSet( SL_REMOVE_RX_FILTER, ( _u8* )&RxFilterIdMask,
                                  sizeof( _WlanRxFilterOperationCommandBuff_t ) );
    ASSERT_ON_ERROR( lRetVal );

    lRetVal = sl_Stop( SL_STOP_TIMEOUT );
    ASSERT_ON_ERROR( lRetVal );

    InitializeAppVariables();

    return lRetVal; // Success
}

static void BoardInit( void )
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
//
// Set vector table base
//
#if defined( ccs )
    MAP_IntVTableBaseSet( ( unsigned long )&g_pfnVectors[0] );
#endif
#if defined( ewarm )
    MAP_IntVTableBaseSet( ( unsigned long )&__vector_table );
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable( FAULT_SYSTICK );

    PRCMCC3200MCUInit();
}

void WaitForWlanEvent()
{
    // Wait for WLAN Event
    while ( ( !IS_CONNECTED( g_ulStatus ) ) || ( !IS_IP_ACQUIRED( g_ulStatus ) ) )
    {
        // Toggle LEDs to Indicate Connection Progress
        _SlNonOsMainLoopTask();
        MAP_UtilsDelay( 2000000 );
        GPIO_IF_LedOn( MCU_ALL_LED_IND );
        _SlNonOsMainLoopTask();
        MAP_UtilsDelay( 2000000 );
        GPIO_IF_LedOff( MCU_ALL_LED_IND );
    }
}

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
            XIVELY_DEMO_PRINT( "Failed to configure the device "
                               "in its default state \n" );

        return lRetVal;
    }

    XIVELY_DEMO_PRINT( "Device is configured in default state \n" );

    CLR_STATUS_BIT_ALL( g_ulStatus );

    // Start simplelink
    lRetVal = sl_Start( 0, 0, 0 );
    if ( lRetVal < 0 || ROLE_STA != lRetVal )
    {
        XIVELY_DEMO_PRINT( "Failed to start the device \n" );
        return lRetVal;
    }

    XIVELY_DEMO_PRINT( "Device started as STATION \n" );

    // start ent wlan connection
    g_SecParams.Key    = PASSWORD;
    g_SecParams.KeyLen = strlen( ( const char* )g_SecParams.Key );
    g_SecParams.Type   = SL_SEC_TYPE_WPA_WPA2;

    // 0 - Disable the server authnetication | 1 - Enable (this is the deafult)
    pValues = 0;
    sl_WlanSet( SL_WLAN_CFG_GENERAL_PARAM_ID, 19, 1, &pValues );

    // add the wlan profile
    sl_WlanProfileAdd( ENT_NAME, strlen( ENT_NAME ), 0, &g_SecParams, 0, 1, 0 );

    // set the connection policy to auto
    pValues = 0;
    sl_WlanPolicySet( SL_POLICY_CONNECTION, SL_CONNECTION_POLICY( 1, 1, 0, 0, 0 ),
                      &pValues, 1 );

    // wait for the WiFi connection
    WaitForWlanEvent();

    // this is where the connection to Xively Broker can be established
    ConnectToXively();

    // wait for few moments
    MAP_UtilsDelay( 80000000 );

    // Disconnect from network
    lRetVal = sl_WlanDisconnect();

    //
    // power off the network processor
    //
    lRetVal = sl_Stop( SL_STOP_TIMEOUT );

    return SUCCESS;
}

int main()
{
    long lRetVal = -1;

    //
    // Initialize Board configurations
    //
    BoardInit();

    PinMuxConfig();

    // configure RED and Green LED
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
