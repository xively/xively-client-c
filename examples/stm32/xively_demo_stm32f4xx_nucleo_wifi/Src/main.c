/**
 ******************************************************************************
 * @file    main.c
 * @author  Xively
 * @version V1.0.0
 * @date    03-March-2017
 * @brief   Main program body of Xively Client Example for STM32F4 Nucleo Wifi
 ******************************************************************************
 */

/**
 * @attention
 *
 * Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 *
 * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include <assert.h>
#include "stdio.h"
#include "string.h"

#include "main.h"
#include "wifi_interface.h"
#include "user_data.h"
#include "demo_bsp.h"
#include "demo_io.h"
#include "provisioning.h"

/* Xively's headers */
#include "xively.h"
#include "xi_bsp_io_net_socket_proxy.h"
#include "xi_bsp_time.h"

/**
 * @mainpage Documentation for X-CUBE-WIFI Software for STM32, Expansion for STM32Cube
 * <b>Introduction</b> <br>
 * X-CUBE-WIFI1 is an expansion software package for STM32Cube.
 * The software runs on STM32 and it can be used for building Wi-Fi applications using
 * the SPWF01Sx device.
 * It is built on top of STM32Cube software technology that eases portability across
 * different STM32 microcontrollers.
 *
 *
 * \htmlinclude extra.html
 */

/** @defgroup WIFI_Examples
 * @{
 */

/** @defgroup WIFI_Example_Client_Socket
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define DEBUG_UART_BAUDRATE 115200
#define WIFI_BOARD_UART_BAUDRATE 115200
#define WIFI_SCAN_BUFFER_LIST 25

#ifndef USE_HARDCODED_CREDENTIALS
/* Default workflow uses runtime provisioning and flash storage for user data */
#define USE_HARDCODED_CREDENTIALS 0
#endif /* USE_HARDCODED_CREDENTIALS */

#if USE_HARDCODED_CREDENTIALS
#define USER_CONFIG_WIFI_SSID "User's WiFi Network Name"
#define USER_CONFIG_WIFI_PWD "User's WiFi Network Password"
#define USER_CONFIG_WIFI_ENCR WPA_Personal /* [ WPA_Personal | WEP | None ] */
#define USER_CONFIG_XI_ACCOUNT_ID "Xively Account ID"
#define USER_CONFIG_XI_DEVICE_ID "Xively Device ID"
#define USER_CONFIG_XI_DEVICE_PWD "Xively Device Password"
#endif /* USE_HARDCODED_CREDENTIALS */

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

static user_data_t user_config;

wifi_state_t wifi_state;
wifi_config wifi_module_config;
wifi_scan net_scan[WIFI_SCAN_BUFFER_LIST];
xi_state_t xi_connection_state = XI_SOCKET_NO_ACTIVE_CONNECTION_ERROR;
uint8_t virtual_switch_state   = 0; /* Pressing the user button flips this value */
#define is_burst_mode virtual_switch_state

xi_context_handle_t gXivelyContextHandle      = XI_INVALID_CONTEXT_HANDLE;
xi_timed_task_handle_t gXivelyTimedTaskHandle = XI_INVALID_TIMED_TASK_HANDLE;

/* Private function prototypes -----------------------------------------------*/

/* the interval for the time function */
#define XI_PUBLISH_INTERVAL_SEC 5

/* combines name with the account and device id */
#define XI_TOPIC_MAX_LEN 256

/* Xively channel names - These weill be used to build the MQTT topic addresses */
#define ACCELEROMETER_CHANNEL_NAME "Accelerometer"
#define GYROSCOPE_CHANNEL_NAME "Gyroscope"
#define MAGNETOMETER_CHANNEL_NAME "Magnetometer"
#define BAROMETER_CHANNEL_NAME "Barometer"
#define HUMIDITY_CHANNEL_NAME "Humidity"
#define TEMPERATURE_CHANNEL_NAME "Temperature"
#define BUTTON_CHANNEL_NAME "Button"
#define LED_CHANNEL_NAME "LED"

/* RAM can be saved by building each of these when needed and free()ing them after use */
typedef struct mqtt_topics_s
{
    char accelerometer[XI_TOPIC_MAX_LEN];
    char gyroscope[XI_TOPIC_MAX_LEN];
    char magnetometer[XI_TOPIC_MAX_LEN];
    char barometer[XI_TOPIC_MAX_LEN];
    char hygrometer[XI_TOPIC_MAX_LEN];
    char temperature[XI_TOPIC_MAX_LEN];
    char button[XI_TOPIC_MAX_LEN];
    char led[XI_TOPIC_MAX_LEN];
} mqtt_topics_t;

static mqtt_topics_t mqtt_topics;

/* declaration of topic handlers for PUB'ed topics */
static xi_state_t pub_accelerometer( void );
static xi_state_t pub_magnetometer( void );
static xi_state_t pub_barometer( void );
static xi_state_t pub_virtual_switch( void );
static xi_state_t pub_gyroscope( void );
static xi_state_t pub_humidity( void );
static xi_state_t pub_temperature( void );
static void pub_button_interrupt( void );
static void on_led_msg( xi_context_handle_t in_context_handle,
                        xi_sub_call_type_t call_type,
                        const xi_sub_call_params_t* const params,
                        xi_state_t state,
                        void* user_data );

/* topic's initialisation function */
static xi_state_t init_xively_topics( xi_context_handle_t in_context_handle );
static int8_t update_all_mqtt_topics( char* xi_account_id, char* xi_device_id );
static int8_t build_xively_topic(
    char* topic_name, char* account_id, char* device_id, char* dst, uint32_t dst_len );

/**
 * @brief This function receives a pointer to a pre-allocated buffer to be filled in.
 *        We recommend that string is allocated XI_TOPIC_MAX_LEN bytes
 */
static int8_t build_xively_topic(
    char* topic_name, char* account_id, char* device_id, char* dst, uint32_t dst_len )
{
    int retval = 0;

    assert( NULL != topic_name );
    assert( NULL != account_id );
    assert( NULL != device_id );
    assert( NULL != dst );

    retval = snprintf( dst, dst_len, "xi/blue/v1/%.64s/d/%.64s/%.64s", account_id,
                       device_id, topic_name );

    if ( ( retval >= dst_len ) || ( retval < 0 ) )
    {
        return -1;
    }
    return 0;
}

/**
 * @brief  Build the MQTT topic addresses with the given account_id and device_id
 * @param
 * @retval -1: Error
 * @retval 0: OK
 */
static int8_t update_all_mqtt_topics( char* xi_account_id, char* xi_device_id )
{
    assert( strlen( xi_account_id ) > 0 );
    assert( strlen( xi_device_id ) > 0 );

    if ( 0 > build_xively_topic( ACCELEROMETER_CHANNEL_NAME, xi_account_id, xi_device_id,
                                 mqtt_topics.accelerometer, XI_TOPIC_MAX_LEN ) )
        return -1;

    if ( 0 > build_xively_topic( GYROSCOPE_CHANNEL_NAME, xi_account_id, xi_device_id,
                                 mqtt_topics.gyroscope, XI_TOPIC_MAX_LEN ) )
        return -1;

    if ( 0 > build_xively_topic( MAGNETOMETER_CHANNEL_NAME, xi_account_id, xi_device_id,
                                 mqtt_topics.magnetometer, XI_TOPIC_MAX_LEN ) )
        return -1;

    if ( 0 > build_xively_topic( BAROMETER_CHANNEL_NAME, xi_account_id, xi_device_id,
                                 mqtt_topics.barometer, XI_TOPIC_MAX_LEN ) )
        return -1;

    if ( 0 > build_xively_topic( HUMIDITY_CHANNEL_NAME, xi_account_id, xi_device_id,
                                 mqtt_topics.hygrometer, XI_TOPIC_MAX_LEN ) )
        return -1;

    if ( 0 > build_xively_topic( TEMPERATURE_CHANNEL_NAME, xi_account_id, xi_device_id,
                                 mqtt_topics.temperature, XI_TOPIC_MAX_LEN ) )
        return -1;

    if ( 0 > build_xively_topic( BUTTON_CHANNEL_NAME, xi_account_id, xi_device_id,
                                 mqtt_topics.button, XI_TOPIC_MAX_LEN ) )
        return -1;

    if ( 0 > build_xively_topic( LED_CHANNEL_NAME, xi_account_id, xi_device_id,
                                 mqtt_topics.led, XI_TOPIC_MAX_LEN ) )
        return -1;

    return 0;
}

xi_state_t pub_accelerometer( void )
{
    char out_msg[IO_AXES_JSON_BUFFER_MAX_SIZE];
    SensorAxes_t sensor_input;

    if ( 0 > io_read_accelero( &sensor_input ) )
    {
        return -1;
    }
    if ( 0 > io_axes_to_json( sensor_input, out_msg, IO_AXES_JSON_BUFFER_MAX_SIZE ) )
    {
        return -1;
    }

    return xi_publish( gXivelyContextHandle, mqtt_topics.accelerometer, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_magnetometer( void )
{
    char out_msg[IO_AXES_JSON_BUFFER_MAX_SIZE];
    SensorAxes_t sensor_input;

    if ( 0 > io_read_magneto( &sensor_input ) )
    {
        return -1;
    }
    if ( 0 > io_axes_to_json( sensor_input, out_msg, IO_AXES_JSON_BUFFER_MAX_SIZE ) )
    {
        return -1;
    }
    return xi_publish( gXivelyContextHandle, mqtt_topics.magnetometer, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_gyroscope( void )
{
    char out_msg[IO_AXES_JSON_BUFFER_MAX_SIZE];
    SensorAxes_t sensor_input;

    if ( 0 > io_read_gyro( &sensor_input ) )
    {
        return -1;
    }
    if ( 0 > io_axes_to_json( sensor_input, out_msg, IO_AXES_JSON_BUFFER_MAX_SIZE ) )
    {
        return -1;
    }
    return xi_publish( gXivelyContextHandle, mqtt_topics.gyroscope, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_barometer( void )
{
    char out_msg[IO_FLOAT_BUFFER_MAX_SIZE];
    float sensor_input = 0;

    if ( 0 > io_read_pressure( &sensor_input ) )
    {
        return -1;
    }
    if ( 0 > io_float_to_string( sensor_input, out_msg, IO_FLOAT_BUFFER_MAX_SIZE ) )
    {
        return -1;
    }
    return xi_publish( gXivelyContextHandle, mqtt_topics.barometer, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_humidity( void )
{
    char out_msg[IO_FLOAT_BUFFER_MAX_SIZE];
    float sensor_input = 0;

    if ( 0 > io_read_humidity( &sensor_input ) )
    {
        return -1;
    }
    if ( 0 > io_float_to_string( sensor_input, out_msg, IO_FLOAT_BUFFER_MAX_SIZE ) )
    {
        return -1;
    }
    return xi_publish( gXivelyContextHandle, mqtt_topics.hygrometer, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_temperature( void )
{
    char out_msg[IO_FLOAT_BUFFER_MAX_SIZE];
    float sensor_input = 0;

    if ( 0 > io_read_temperature( &sensor_input ) )
    {
        return -1;
    }
    if ( 0 > io_float_to_string( sensor_input, out_msg, IO_FLOAT_BUFFER_MAX_SIZE ) )
    {
        return -1;
    }
    return xi_publish( gXivelyContextHandle, mqtt_topics.temperature, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_virtual_switch( void )
{
    const char* payload = ( virtual_switch_state == 1 ) ? "1" : "0";

    return xi_publish( gXivelyContextHandle, mqtt_topics.button, payload,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

void pub_button_interrupt( void )
{
    const char* payload = ( virtual_switch_state == 1 ) ? "1" : "0";

    if ( ( int )XI_CONNECTION_STATE_OPENED != ( int )xi_connection_state )
    {
        printf( "\r\n\tInterrupt publish [ABORT] Device not connected to the broker" );
        return;
    }

    xi_publish( gXivelyContextHandle, mqtt_topics.button, payload,
                XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

void publish_mqtt_topics()
{
    if ( !is_burst_mode )
    {
        pub_gyroscope();
        pub_accelerometer();
    }

    pub_virtual_switch();
    pub_magnetometer();
    pub_barometer();
    pub_humidity();
    pub_temperature();
}

void on_connected( xi_context_handle_t in_context_handle, void* data, xi_state_t state )
{
    /* sanity check */
    assert( NULL != data );

    xi_connection_data_t* conn_data = ( xi_connection_data_t* )data;
    xi_connection_state             = conn_data->connection_state;

    printf( "\r\n>> MQTT connection state: %i", conn_data->connection_state );
    switch ( conn_data->connection_state )
    {
        /* connection attempt failed */
        case XI_CONNECTION_STATE_OPEN_FAILED:
            printf( "\r\n>> [%d] Xively Connection [ERROR] Broker: %s:%d, Error: %d",
                    ( int )xi_bsp_time_getcurrenttime_seconds(), conn_data->host,
                    conn_data->port, state );
            printf( "\r\n\tReconnecting to the MQTT broker" );

            xi_connect( in_context_handle, conn_data->username, conn_data->password,
                        conn_data->connection_timeout, conn_data->keepalive_timeout,
                        conn_data->session_type, &on_connected );
            break;
        /* connection has been closed */
        case XI_CONNECTION_STATE_CLOSED:
            /* unregister task handle */
            xi_cancel_timed_task( gXivelyTimedTaskHandle );
            gXivelyTimedTaskHandle = XI_INVALID_TIMED_TASK_HANDLE;

            printf( "\r\n>> [%d] Xively Connection Closed. Broker: %s:%d, Reason: %d",
                    ( int )xi_bsp_time_getcurrenttime_seconds(), conn_data->host,
                    conn_data->port, state );
            printf( "\r\n\tReconnecting to the MQTT broker" );

            xi_connect( in_context_handle, conn_data->username, conn_data->password,
                        conn_data->connection_timeout, conn_data->keepalive_timeout,
                        conn_data->session_type, &on_connected );
            break;
        /* connection has been established */
        case XI_CONNECTION_STATE_OPENED:
            printf( "\r\n>> [%d] Xively Connection [ESTABLISHED] Broker: %s:%d",
                    ( int )xi_bsp_time_getcurrenttime_seconds(), conn_data->host,
                    conn_data->port );

            {
                /* here put the activation code */
                init_xively_topics( in_context_handle );
            }

            break;
        /* something really bad happened */
        default:
            printf( "\r\n>> [%d] Xively Internal [ERROR] Invalid connection status: %d",
                    ( int )xi_bsp_time_getcurrenttime_seconds(),
                    ( int )conn_data->connection_state );
    };
}

/* handler for xively mqtt topic subscription */
void on_led_msg( xi_context_handle_t in_context_handle,
                 xi_sub_call_type_t call_type,
                 const xi_sub_call_params_t* const params,
                 xi_state_t state,
                 void* user_data )
{
    switch ( call_type )
    {
        case XI_SUB_CALL_SUBACK:
            if ( XI_MQTT_SUBACK_FAILED == params->suback.suback_status )
            {
                printf( "\r\n>> Subscription to LED topic [FAIL]" );
            }
            else
            {
                printf( "\r\n>> Subscription to LED topic [OK]" );
            }
            return;
        case XI_SUB_CALL_MESSAGE:
            printf( "\r\n>> received message on LED topic" );
            printf( "\r\n\t Message size: %d",
                    params->message.temporary_payload_data_length );
            printf( "\r\n\t Message payload: " );
            for ( size_t i = 0; i < params->message.temporary_payload_data_length; i++ )
            {
                printf( "0x%02x ", params->message.temporary_payload_data[i] );
            }

            if ( params->message.temporary_payload_data_length != 1 )
            {
                printf( "\r\n\tUnrecognized LED message [IGNORED] Expected 1 or 0" );
                return;
            }

            if ( params->message.temporary_payload_data[0] == '0' )
            {
                io_led_off();
            }
            else
            {
                io_led_on();
            }
            break;
        default:
            return;
    }
}

/*
 * Initializes Xively's demo application topics
 */
static xi_state_t init_xively_topics( xi_context_handle_t in_context_handle )
{
    xi_state_t ret = XI_STATE_OK;

    if ( 0 >
         update_all_mqtt_topics( user_config.xi_account_id, user_config.xi_device_id ) )
    {
        printf( "\r\n>> Topic composition [ERROR]" );
        return XI_FAILED_INITIALIZATION;
    }

    printf( "\r\n>> Subscribing to MQTT topic [%s]", mqtt_topics.led );
    ret = xi_subscribe( in_context_handle, mqtt_topics.led, XI_MQTT_QOS_AT_MOST_ONCE,
                        &on_led_msg, NULL );

    if ( XI_STATE_OK != ret )
    {
        printf( "\r\n>> Xively subscription [ERROR]" );
        return XI_MQTT_SUBSCRIPTION_FAILED;
    }

    /* registration of the publish function */
    gXivelyTimedTaskHandle = xi_schedule_timed_task(
        in_context_handle, &publish_mqtt_topics, XI_PUBLISH_INTERVAL_SEC, 1, NULL );

    if ( XI_INVALID_TIMED_TASK_HANDLE == gXivelyTimedTaskHandle )
    {
        printf( "\r\n>> Libxively timed task registration [ERROR]" );
        return XI_FAILED_INITIALIZATION;
    }

    return ret;
}

static inline int8_t system_init( void )
{
    __GPIOA_CLK_ENABLE();
    HAL_Init();

    /* Configure the system clock to 64 MHz */
    SystemClock_Config();
    SystemCoreClockUpdate();

    /* configure the timers  */
    Timer_Config();

    UART_Configuration( WIFI_BOARD_UART_BAUDRATE ); /* From stm32_spwf_wifi.c */
#ifdef USART_PRINT_MSG
    UART_Msg_Gpio_Init();
    if ( 0 > USART_PRINT_MSG_Configuration( DEBUG_UART_BAUDRATE ) )
    {
        return -1;
    }
#endif /* USART_PRINT_MSG */

#if !USE_HARDCODED_CREDENTIALS
    if ( user_data_flash_init() < 0 )
    {
        printf( "\r\n>> User data flash initialization [ERROR]" );
        return -1;
    }
#endif /* !USE_HARDCODED_CREDENTIALS */

    /* Init the sensor board */
    printf( "\r\n>> Initializing the sensor extension board" );
    if ( 0 > io_nucleoboard_init() )
    {
        printf( "\r\n>> Nucleo Board peripheral initialization  [ERROR]" );
        return -1;
    }
    if ( 0 > io_sensorboard_init() )
    {
        printf( "\r\n>> Sensor Board initializtion [ERROR]" );
        return -1;
    }
    io_sensorboard_enable();

    /* Init the wi-fi module */
    printf( "\r\n>> Initializing the WiFi extension board" );
    fflush( stdout );
    wifi_module_config.power       = wifi_sleep;
    wifi_module_config.power_level = high;
    wifi_module_config.dhcp        = on; /* use DHCP IP address */
    // wifi_module_config.web_server  = WIFI_TRUE;

    wifi_state = wifi_state_idle;

    if ( WiFi_MODULE_SUCCESS != wifi_init( &wifi_module_config ) )
    {
        printf( "\r\n>> WiFi board initialization [ERROR]" );
        return -1;
    }
    return 0;
}

/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
int main( void )
{
    uint8_t i            = 0;
    uint8_t socket_open  = 0;
    wifi_bool SSID_found = WIFI_FALSE;
    WiFi_Status_t status = WiFi_MODULE_SUCCESS;

    /* Initialize uC and peripherals */
    if ( 0 > system_init() )
    {
        printf( "\r\n>> System initialization [ERROR] - System boot aborted" );
        while ( 1 )
            ;
    }

#if USE_HARDCODED_CREDENTIALS
    user_data_set_wifi_ssid( (&user_config), USER_CONFIG_WIFI_SSID );
    user_data_set_wifi_password( (&user_config), USER_CONFIG_WIFI_PWD );
    user_data_set_wifi_encryption( (&user_config), USER_CONFIG_WIFI_ENCR );
    user_data_set_xi_account_id( (&user_config), USER_CONFIG_XI_ACCOUNT_ID );
    user_data_set_xi_device_id( (&user_config), USER_CONFIG_XI_DEVICE_ID );
    user_data_set_xi_device_password( (&user_config), USER_CONFIG_XI_DEVICE_PWD );
#else
    /* Read user data from flash */
    if ( user_data_copy_from_flash( &user_config ) < 0 )
    {
        printf( "\r\n>> [ERROR] trying to copy user data from flash. Abort" );
        while ( 1 )
            ;
    }
    printf( "\r\n>> User data retrieved from flash:" );
    user_data_printf( &user_config );
    fflush( stdout );

    /* Provision the device if requested or if flash data is missing||corrupt */
    const int8_t provisioning_bootmode = io_read_button();
    if ( ( 1 == provisioning_bootmode ) ||
         ( 0 > user_data_validate_checksum( &user_config ) ) )
    {
        ( 1 == provisioning_bootmode )
            ? printf( "\r\n>> User requested device reprovisioning" )
            : printf( "\r\n>> [ERROR] Invalid credentials recovered from flash" );
        if ( 0 > provisioning_gather_user_data( &user_config ) )
        {
            printf( "\r\n>> Device provisioning [ERROR]. Abort" );
            while ( 1 )
                ;
        }
    }
#endif /* USE_HARDCODED_CREDENTIALS */

    /* Application state machine */
    while ( 1 )
    {
        xi_events_process_tick();
        switch ( wifi_state )
        {
            case wifi_state_reset:
                break;

            case wifi_state_ready:
                printf( "\r\n>> Scanning WiFi networks to find SSID [%s]",
                        user_config.wifi_client_ssid );
                status = wifi_network_scan( net_scan, WIFI_SCAN_BUFFER_LIST );
                if ( status == WiFi_MODULE_SUCCESS )
                {
                    for ( i = 0; i < WIFI_SCAN_BUFFER_LIST; i++ )
                    {
                        // printf(net_scan[i].ssid);
                        // printf("\r\n");
                        if ( NULL != ( char* )strstr(
                                         ( const char* )net_scan[i].ssid,
                                         ( const char* )user_config.wifi_client_ssid ) )
                        {
                            printf( "\r\n>> Network found. Connecting to AP\r\n\t" );
                            SSID_found = WIFI_TRUE;
                            wifi_connect( user_config.wifi_client_ssid,
                                          user_config.wifi_client_password,
                                          ( WiFi_Priv_Mode )
                                              user_config.wifi_client_encryption_mode );
                            break;
                        }
                    }
                    memset( net_scan, 0x00, sizeof( net_scan ) );
                }
                if ( SSID_found )
                {
                    wifi_state = wifi_state_idle;
                }
                else
                {
                    printf( "\r\n>> WiFi scan [ERROR] Network not found!" );
                }
                break;

            case wifi_state_connected:
                printf( "\r\n>> Connection to WiFi access point [OK]\r\n\t" );
                wifi_state = wifi_state_idle;
                HAL_Delay( 2000 ); /* Let module go to sleep - IMPORTANT - TODO: Why? */
                wifi_wakeup( WIFI_TRUE ); /*wakeup from sleep if module went to sleep*/
                break;

            case wifi_state_disconnected:
                wifi_state = wifi_state_reset;
                break;

            case wifi_state_socket:
                printf( "\r\n>> Connecting to Xively's MQTT broker using credentials:" );
                printf( "\r\n\t* Account ID:  [%s]", user_config.xi_account_id );
                printf( "\r\n\t* Device ID:   [%s]", user_config.xi_device_id );
                printf( "\r\n\t* Device Pass: ( REDACTED )\n" );
                // printf( "\r\n\t* Device Pass: [%s]\n", user_config.xi_device_password );

                if ( socket_open == 0 )
                {
                    /* Read Write Socket data */
                    xi_state_t ret_state = xi_initialize( user_config.xi_account_id );
                    if ( XI_STATE_OK != ret_state )
                    {
                        printf( "\r\n xi failed to initialise\n" );
                        return -1;
                    }

                    gXivelyContextHandle = xi_create_context();
                    if ( XI_INVALID_CONTEXT_HANDLE == gXivelyContextHandle )
                    {
                        printf( "\r\n xi failed to create context, error: %li\n",
                                -gXivelyContextHandle );
                        return -1;
                    }

                    xi_connect( gXivelyContextHandle, user_config.xi_device_id,
                                user_config.xi_device_password, 10, 0, XI_SESSION_CLEAN,
                                &on_connected );
                }
                else
                {
                    printf( "\r\n>> [ERROR] Xively socket is closed!" );
                }

                wifi_state = wifi_state_idle;
                break;

            case wifi_state_idle:
                printf( "." );
                fflush( stdout );
                HAL_Delay( 100 );

                if ( is_burst_mode &&
                     ( ( int )XI_CONNECTION_STATE_OPENED == ( int )xi_connection_state ) )
                {
                    pub_gyroscope();
                    pub_accelerometer();
                    pub_virtual_switch();
                }
                break;

            default:
                break;
        }
    }
}

/**
 * @brief  This function Is called when there's an EXTernal Interrupt caused
 *         by a GPIO pin. It must be kickstarted by each interrupt from the
 *         pin-specific handlers at stm32_xx_it.c
 * @param  Pin number of the GPIO generating the EXTI IRQ
 * @retval None
 */
void HAL_GPIO_EXTI_Callback( uint16_t GPIO_Pin )
{
    if ( GPIO_Pin == IO_NUCLEO_BUTTON_PIN )
    {
        if ( io_button_exti_debouncer( GPIO_Pin ) <= 0 )
        {
            return;
        }

        virtual_switch_state = !( virtual_switch_state );
        if ( virtual_switch_state )
        {
            printf( "\r\n>> Entering Burst Mode! Motion data publish rate increased" );
        }
        else
        {
            printf( "\r\n>> Entering Standard Mode!" );
        }

        printf( "\r\n>> Nucleo board button [PRESSED]" );
        pub_button_interrupt();
    }
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed( uint8_t* file, uint32_t line )
{
    /* User can add his own implementation to report the file name and line number,
ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    printf( "\r\n>> Fatal system [ERROR] - Assert() failed, execution halted" );

    /* Infinite loop */
    while ( 1 )
    {
    }
}
#endif

/******** Wi-Fi Indication User Callback *********/

void ind_wifi_socket_data_received( int8_t server_id,
                                    int8_t socket_id,
                                    uint8_t* data_ptr,
                                    uint32_t message_size,
                                    uint32_t chunk_size )
{
    ( void )server_id; /* Unused */
    /* Xively */
    xi_bsp_io_net_socket_data_received_proxy( ( uint8_t )socket_id, data_ptr,
                                              message_size, chunk_size );
}

void ind_wifi_socket_client_remote_server_closed( uint8_t* socket_closed_id )
{
    /* Xively */
    xi_bsp_io_net_socket_client_remote_server_closed_proxy( socket_closed_id );
}

void ind_wifi_on()
{
    printf( "\r\n>> WiFi initialization [OK]" );
    wifi_state = wifi_state_ready;
}

void ind_wifi_connected()
{
    wifi_state = wifi_state_connected;
}

void ind_wifi_resuming()
{
    printf( "\r\n>> WiFi module woke up from sleep" );
    // Change the state to connect to socket if not connected
    wifi_state = wifi_state_socket;
}

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
