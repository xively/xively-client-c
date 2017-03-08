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
 * Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
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
#include "main.h"
#include "wifi_interface.h"
#include "stdio.h"
#include "string.h"
#include "user_data.h"
#include "demo_bsp.h"
#include "demo_io.h"

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
#define USE_FLASH_STORAGE 1
#define WIFI_SCAN_BUFFER_LIST 25

#define USE_HARDCODED_CREDENTIALS 0 /* Overrules USE_FLASH_STORAGE */
#if USE_HARDCODED_CREDENTIALS
#define USER_CONFIG_WIFI_SSID "User's WiFi Network Name"
#define USER_CONFIG_WIFI_PWD "User's WiFi Network Password"
#define USER_CONFIG_WIFI_ENCR WPA_Personal /* [ WPA_Personal | WEP | None ] */
#define USER_CONFIG_XI_ACCOUNT_ID "Xively Account ID"
#define USER_CONFIG_XI_DEVICE_ID "Xively Device ID"
#define USER_CONFIG_XI_DEVICE_PWD "Xively Device Password"
#endif
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
volatile uint8_t button_pressed_interrupt_flag = 0;
static user_data_t user_config;

wifi_state_t wifi_state;
wifi_config wifi_module_config;
wifi_scan net_scan[WIFI_SCAN_BUFFER_LIST];
xi_state_t xi_connection_state = XI_SOCKET_NO_ACTIVE_CONNECTION_ERROR;
uint8_t is_burst_mode          = 0;

xi_context_handle_t gXivelyContextHandle      = XI_INVALID_CONTEXT_HANDLE;
xi_timed_task_handle_t gXivelyTimedTaskHandle = XI_INVALID_TIMED_TASK_HANDLE;

/* Private function prototypes -----------------------------------------------*/
static inline void print_user_config_debug_banner( void );
static int8_t user_config_init( user_data_t* dst );
static int8_t get_ap_credentials( user_data_t* udata );
static int8_t get_xively_credentials( user_data_t* udata );

/* the interval for the time function */
#define XI_PUBLISH_INTERVAL_SEC 5

/* combines name with the account and device id */
#define XI_TOPIC_MAX_LEN 256

/* Xively channel names - These weill be used to build the MQTT topic addresses */
#define ACCELEROMETER_CHANNEL_NAME "Accelerometer"
#define GYROSCOPE_CHANNEL_NAME     "Gyroscope"
#define MAGNETOMETER_CHANNEL_NAME  "Magnetometer"
#define BAROMETER_CHANNEL_NAME     "Barometer"
#define HUMIDITY_CHANNEL_NAME      "Humidity"
#define TEMPERATURE_CHANNEL_NAME   "Temperature"
#define BUTTON_CHANNEL_NAME        "Button"
#define LED_CHANNEL_NAME           "LED"

/* declaration of topic handlers for PUB'ed topics */
static xi_state_t pub_accelerometer( void );
static xi_state_t pub_magnetometer( void );
static xi_state_t pub_barometer( void );
static xi_state_t pub_button( void );
static xi_state_t pub_gyroscope( void );
static xi_state_t pub_humidity( void );
static xi_state_t pub_temperature( void );

/* topic's initialisation function */
static xi_state_t init_xively_topics( xi_context_handle_t in_context_handle );

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

    retval = snprintf( dst, dst_len, "xi/blue/v1/%.64s/d/%.64s/%.64s",
                       account_id, device_id, topic_name );

    if ( ( retval >= dst_len ) || ( retval < 0 ) )
    {
        return -1;
    }
    return 0;
}

xi_state_t pub_accelerometer( void )
{
    char xi_topic[XI_TOPIC_MAX_LEN];
    char out_msg[IO_AXES_JSON_BUFFER_MAX_SIZE];
    SensorAxes_t sensor_input;

    if ( 0 > build_xively_topic( ACCELEROMETER_CHANNEL_NAME, user_config.xi_account_id,
                                 user_config.xi_device_id, xi_topic, XI_TOPIC_MAX_LEN ) )
    {
        return -1;
    }

    if ( 0 > io_read_accelero( &sensor_input ) )
    {
        return -1;
    }
    if ( 0 > io_axes_to_json( sensor_input, out_msg, IO_AXES_JSON_BUFFER_MAX_SIZE ) )
    {
        return -1;
    }

    return xi_publish( gXivelyContextHandle, xi_topic, out_msg, XI_MQTT_QOS_AT_MOST_ONCE,
                       XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_magnetometer( void )
{
    char xi_topic[XI_TOPIC_MAX_LEN];
    char out_msg[IO_AXES_JSON_BUFFER_MAX_SIZE];
    SensorAxes_t sensor_input;

    if ( 0 > build_xively_topic( MAGNETOMETER_CHANNEL_NAME, user_config.xi_account_id,
                                 user_config.xi_device_id, xi_topic, XI_TOPIC_MAX_LEN ) )
    {
        return -1;
    }

    if ( 0 > io_read_magneto( &sensor_input ) )
    {
        return -1;
    }
    if ( 0 > io_axes_to_json( sensor_input, out_msg, IO_AXES_JSON_BUFFER_MAX_SIZE ) )
    {
        return -1;
    }
    return xi_publish( gXivelyContextHandle, xi_topic, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_gyroscope( void )
{
    char xi_topic[XI_TOPIC_MAX_LEN];
    char out_msg[IO_AXES_JSON_BUFFER_MAX_SIZE];
    SensorAxes_t sensor_input;

    if ( 0 > build_xively_topic( GYROSCOPE_CHANNEL_NAME, user_config.xi_account_id,
                                 user_config.xi_device_id, xi_topic, XI_TOPIC_MAX_LEN ) )
    {
        return -1;
    }

    if ( 0 > io_read_gyro( &sensor_input ) )
    {
        return -1;
    }
    if ( 0 > io_axes_to_json( sensor_input, out_msg, IO_AXES_JSON_BUFFER_MAX_SIZE ) )
    {
        return -1;
    }
    return xi_publish( gXivelyContextHandle, xi_topic, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_barometer( void )
{
    char xi_topic[XI_TOPIC_MAX_LEN];
    char out_msg[IO_FLOAT_BUFFER_MAX_SIZE];
    float sensor_input = 0;

    if ( 0 > build_xively_topic( BAROMETER_CHANNEL_NAME, user_config.xi_account_id,
                                 user_config.xi_device_id, xi_topic, XI_TOPIC_MAX_LEN ) )
    {
        return -1;
    }

    if ( 0 > io_read_pressure( &sensor_input ) )
    {
        return -1;
    }
    if ( 0 > io_float_to_string( sensor_input, out_msg, IO_FLOAT_BUFFER_MAX_SIZE ) )
    {
        return -1;
    }
    return xi_publish( gXivelyContextHandle, xi_topic, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_humidity( void )
{
    char xi_topic[XI_TOPIC_MAX_LEN];
    char out_msg[IO_FLOAT_BUFFER_MAX_SIZE];
    float sensor_input = 0;

    if ( 0 > build_xively_topic( HUMIDITY_CHANNEL_NAME, user_config.xi_account_id,
                                 user_config.xi_device_id, xi_topic, XI_TOPIC_MAX_LEN ) )
    {
        return -1;
    }

    if ( 0 > io_read_humidity( &sensor_input ) )
    {
        return -1;
    }
    if ( 0 > io_float_to_string( sensor_input, out_msg, IO_FLOAT_BUFFER_MAX_SIZE ) )
    {
        return -1;
    }
    return xi_publish( gXivelyContextHandle, xi_topic, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_temperature( void )
{
    char xi_topic[XI_TOPIC_MAX_LEN];
    char out_msg[IO_FLOAT_BUFFER_MAX_SIZE];
    float sensor_input = 0;

    if ( 0 > build_xively_topic( TEMPERATURE_CHANNEL_NAME, user_config.xi_account_id,
                                 user_config.xi_device_id, xi_topic, XI_TOPIC_MAX_LEN ) )
    {
        return -1;
    }

    if ( 0 > io_read_temperature( &sensor_input ) )
    {
        return -1;
    }
    if ( 0 > io_float_to_string( sensor_input, out_msg, IO_FLOAT_BUFFER_MAX_SIZE ) )
    {
        return -1;
    }
    return xi_publish( gXivelyContextHandle, xi_topic, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_button( void )
{
    char xi_topic[XI_TOPIC_MAX_LEN];
    char* payload;

    if ( 0 > build_xively_topic( BUTTON_CHANNEL_NAME, user_config.xi_account_id,
                                 user_config.xi_device_id, xi_topic, XI_TOPIC_MAX_LEN ) )
    {
        return -1;
    }

    ( io_read_button() ) ? ( payload = "1" ) : ( payload = "0" );
    return xi_publish( gXivelyContextHandle, xi_topic, payload,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

void pub_button_interrupt( void )
{
    /* TODO: Verify we are connected to Xively before calling xi_publish */
    char xi_topic[XI_TOPIC_MAX_LEN];
    const char payload[] = "1";


    if ( 0 > build_xively_topic( BUTTON_CHANNEL_NAME, user_config.xi_account_id,
                                 user_config.xi_device_id, xi_topic, XI_TOPIC_MAX_LEN ) )
    {
        return;
    }


    xi_publish( gXivelyContextHandle, xi_topic, payload, XI_MQTT_QOS_AT_MOST_ONCE,
                XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

void publish_mqtt_topics()
{
    if ( !is_burst_mode )
    {
        pub_gyroscope();
        pub_accelerometer();
    }

    pub_button();
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
xi_state_t init_xively_topics( xi_context_handle_t in_context_handle )
{
    char xi_topic[XI_TOPIC_MAX_LEN];
    xi_state_t ret = XI_STATE_OK;

    if ( 0 > build_xively_topic( LED_CHANNEL_NAME, user_config.xi_account_id,
                                 user_config.xi_device_id, xi_topic, XI_TOPIC_MAX_LEN ) )
    {
        return XI_FAILED_INITIALIZATION;
    }

    printf( "\r\n>> Subscribing to MQTT topic [%s]", xi_topic );
    ret = xi_subscribe( in_context_handle, xi_topic, XI_MQTT_QOS_AT_MOST_ONCE,
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

    UART_Configuration( WIFI_BOARD_UART_BAUDRATE );
#ifdef USART_PRINT_MSG
    UART_Msg_Gpio_Init();
    USART_PRINT_MSG_Configuration( DEBUG_UART_BAUDRATE );
#endif

#ifdef USE_FLASH_STORAGE
    if ( user_data_flash_init() < 0 )
    {
        printf( "\r\n>> User data flash initialization [ERROR]" );
        return -1;
    }
#endif

    /* Init the sensor board */
    printf( "\r\n>> Initializing the sensor extension board" );
    io_nucleoboard_init();
    io_sensorboard_init();
    io_sensorboard_enable();

    /* Init the wi-fi module */
    printf( "\r\n>> Initializing the WiFi extension board" );
    fflush( stdout );
    wifi_module_config.power       = wifi_sleep;
    wifi_module_config.power_level = high;
    wifi_module_config.dhcp        = on; /* use DHCP IP address */
    //wifi_module_config.web_server  = WIFI_TRUE;

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
    uint8_t i                  = 0;
    uint8_t socket_open        = 0;
    wifi_bool SSID_found       = WIFI_FALSE;
    WiFi_Status_t status       = WiFi_MODULE_SUCCESS;

    if ( system_init() < 0 )
    {
        printf( "\r\n>> System initialization [ERROR] - System boot aborted" );
        while ( 1 )
            ;
    }

    if ( user_config_init( &user_config ) < 0 )
    {
        printf( "\r\n>> User data initialization [ERROR] - System boot aborted" );
        while ( 1 )
            ;
    }

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
                printf( "\r\n\t* Device Pass: [%s]\n", user_config.xi_device_password );

                if ( socket_open == 0 )
                {
                    /* Read Write Socket data */
                    xi_state_t ret_state = xi_initialize( user_config.xi_account_id,
                                                          user_config.xi_device_id, 0 );
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
                    pub_button();
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
        if ( !io_button_exti_debouncer( GPIO_Pin ) )
        {
            return;
        }

        is_burst_mode = !( is_burst_mode );
        if ( is_burst_mode )
        {
            printf( "\r\n>> Entering Burst Mode!" );
        }
        else
        {
            printf( "\r\n>> Entering Standard Mode!" );
        }

        if ( io_button_exti_debouncer( GPIO_Pin ) )
        {
            printf( "\r\n>> Nucleo board button [PRESSED]" );
            button_pressed_interrupt_flag = 1;
            pub_button_interrupt();
        }
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

static inline void print_user_config_debug_banner( void )
{
    printf( "\r\n|********************************************************|" );
    printf( "\r\n|               User Configuration Routine               |" );
    printf( "\r\n|               --------------------------               |" );
    printf( "\r\n| WARNING: Do NOT copy-paste long strings to your serial |" );
    printf( "\r\n| -------  terminal. Only strings under ~6 characters    |" );
    printf( "\r\n|          can be copied with medium/high reliability    |" );
    printf( "\r\n|                                                        |" );
    printf( "\r\n| NOTE: You will only need to do this once.              |" );
    printf( "\r\n|                                                        |" );
    printf( "\r\n| NOTE: Next time you want to update these, keep the     |" );
    printf( "\r\n| nucleo board's User button pressed during boot         |" );
    printf( "\r\n|********************************************************|" );
    fflush( stdout );
}

static int8_t user_config_init( user_data_t* dst )
{
#if USE_HARDCODED_CREDENTIALS
    user_data_set_wifi_ssid( dst, USER_CONFIG_WIFI_SSID );
    user_data_set_wifi_psk( dst, USER_CONFIG_WIFI_PWD );
    user_data_set_wifi_encryption( dst, USER_CONFIG_WIFI_ENCR );
    user_data_set_xi_account_id( dst, USER_CONFIG_XI_ACCOUNT_ID );
    user_data_set_xi_device_id( dst, USER_CONFIG_XI_DEVICE_ID );
    user_data_set_xi_device_password( dst, USER_CONFIG_XI_DEVICE_PWD );
#elif USE_FLASH_STORAGE
    if ( user_data_copy_from_flash( dst ) < 0 )
    {
        printf( "\r\n>> [ERROR] trying to copy user data from flash" );
        return -1;
    }
    printf( "\r\n>> User data retrieved from flash:" );
    user_data_printf( dst );
    fflush( stdout );

    if ( io_read_button() || ( user_data_validate_checksum( dst ) < 0 ) )
    {
        printf( "\r\n>> Starting forced user configuration mode" );
        print_user_config_debug_banner();

        /* Get WiFi AP Credentials from the user */
        if ( get_ap_credentials( dst ) < 0 )
        {
            printf( "\r\n>> [ERROR] Getting AP credentials from user" );
            return -1;
        }

        printf( "\r\n>> Saving user data to flash" );
        user_data_save_to_flash( dst );

        /* Get Xively Credentials from the user */
        if ( get_xively_credentials( dst ) < 0 )
        {
            printf( "\r\n>> [ERROR] Getting Xively credentials from user" );
            return -1;
        }

        printf( "\r\n>> Saving user data to flash" );
        user_data_save_to_flash( dst );
    }
#else
    print_user_config_debug_banner();

    /* Get Xively Credentials from the user */
    if ( get_ap_credentials( dst ) < 0 )
    {
        printf( "\r\n>> [ERROR] Getting AP credentials from user" );
        return -1;
    }

    /* Get WiFi AP Credentials from the user */
    if ( get_xively_credentials( dst ) < 0 )
    {
        printf( "\r\n>> [ERROR] Getting Xively credentials from user" );
        return -1;
    }
#endif
    return 0;
}

/* TODO: This insecure implementation is temporary until we've got HTTP config mode */
static int8_t get_ap_credentials( user_data_t* udata )
{
    char single_char_input[2] = "0";

    printf( "\r\n|********************************************************|" );
    printf( "\r\n|              Gathering WiFi AP Credentials             |" );
    printf( "\r\n|********************************************************|" );
    fflush( stdout );

    printf( "\r\n>> Would you like to update your WiFi credentials? [y/N]: " );
    fflush( stdout );
    scanf( "%2s", single_char_input );
    switch ( single_char_input[0] )
    {
        case 'y':
        case 'Y':
            break;
        default:
            return 0;
    }

    printf( "\r\n>> Enter WiFi SSID: " );
    fflush( stdout );
    scanf( "%s", udata->wifi_client_ssid );

    printf( "\r\n>> Enter WiFi Password: " );
    fflush( stdout );
    scanf( "%s", udata->wifi_client_password );

    printf( "\r\n>> Enter WiFi encryption mode [0:Open, 1:WEP, 2:WPA2/WPA2-Personal]: " );
    fflush( stdout );
    scanf( "%2s", single_char_input );
    switch ( single_char_input[0] )
    {
        case '0':
            user_data_set_wifi_encryption( udata, None );
            break;
        case '1':
            user_data_set_wifi_encryption( udata, WEP );
            break;
        case '2':
            user_data_set_wifi_encryption( udata, WPA_Personal );
            break;
        default:
            printf( "\r\n>> Wrong Entry. Mode [%s] is not an option",
                    single_char_input );
            return -2;
    }
    return 0;
}

/* TODO: This insecure implementation is temporary until we've got HTTP config mode */
static int8_t get_xively_credentials( user_data_t* udata )
{
    char single_char_input[2] = "0";

    printf( "\r\n|********************************************************|" );
    printf( "\r\n|              Gathering Xively Credentials              |" );
    printf( "\r\n|********************************************************|" );
    fflush( stdout );

    printf( "\r\n>> Would you like to update your WiFi credentials? [y/N]: " );
    fflush( stdout );
    scanf( "%2s", single_char_input );
    switch ( single_char_input[0] )
    {
        case 'y':
        case 'Y':
            break;
        default:
            return 0;
    }

    printf( "\r\n>> Enter the xively Account ID: " );
    fflush( stdout );
    scanf( "%s", udata->xi_account_id );

    printf( "\r\n>> Enter the xively Device ID: " );
    fflush( stdout );
    scanf( "%s", udata->xi_device_id );

    printf( "\r\n>> Enter the xively Device Password: " );
    fflush( stdout );
    scanf( "%s", udata->xi_device_password );

    return 0;
}

/******** Wi-Fi Indication User Callback *********/

void ind_wifi_socket_data_received( uint8_t socket_id,
                                    uint8_t* data_ptr,
                                    uint32_t message_size,
                                    uint32_t chunk_size )
{
    /* Xively */
    xi_bsp_io_net_socket_data_received_proxy( socket_id, data_ptr, message_size,
                                              chunk_size );
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
