/**
 ******************************************************************************
 * @file    main.c
 * @author  Central LAB
 * @version V2.1.0
 * @date    17-May-2016
 * @brief   Main program body
 ******************************************************************************
 * @attention
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
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
volatile uint8_t button_pressed_interrupt_flag = 0;
static user_data_t user_config;

wifi_state_t wifi_state;
wifi_config wifi_module_config;
wifi_scan net_scan[WIFI_SCAN_BUFFER_LIST];

/* Private function prototypes -----------------------------------------------*/
static inline void print_user_config_debug_banner( void );
static int8_t user_config_init( void );
static int8_t get_ap_credentials( user_data_t* udata );
static int8_t get_xively_credentials( user_data_t* udata );

/* the interval for the time function */
#define XI_PUBLISH_INTERVAL_SEC 2

typedef struct topic_descr_s
{
    const char* const name;
} mqtt_topic_descr_t;

/* declaration of fn pointer for the msg handler */
typedef void ( *on_msg_handler_fn_t )( const uint8_t* const msg, size_t msg_size );

/* declaration of fn pointer for the polling push mechanism */
typedef xi_state_t ( *on_msg_push_fn_t )(
    const mqtt_topic_descr_t* const mqtt_topic_descr );

/* each topic will have it's descriptor and the handler in case of the message */
typedef struct mqtt_topic_configuration_s
{
    mqtt_topic_descr_t topic_descr;
    on_msg_handler_fn_t on_msg_pull;
    on_msg_push_fn_t on_msg_push;
} mqtt_topic_configuration_t;

/* combines name with the account and device id */
#define XI_TOPIC_MAX_LEN 256

/* declaration of topic handlers for SUB'ed topics */
static void on_led_msg( const uint8_t* const msg, size_t msg_size );

/* declaration of topic handlers for PUB'ed topics */
static xi_state_t pub_accelerometer( const mqtt_topic_descr_t* const mqtt_topic_descr );
static xi_state_t pub_magnetometer( const mqtt_topic_descr_t* const mqtt_topic_descr );
static xi_state_t pub_barometer( const mqtt_topic_descr_t* const mqtt_topic_descr );
static xi_state_t pub_button( const mqtt_topic_descr_t* const mqtt_topic_descr );
static xi_state_t pub_gyroscope( const mqtt_topic_descr_t* const mqtt_topic_descr );
static xi_state_t pub_humidity( const mqtt_topic_descr_t* const mqtt_topic_descr );
static xi_state_t pub_temperature( const mqtt_topic_descr_t* const mqtt_topic_descr );

/* topic's initialisation function */
static xi_state_t init_xively_topics( xi_context_handle_t in_context_handle );

/* table of topics used for this demo */
static const mqtt_topic_configuration_t topics_array[] = {
    {{"Accelerometer"}, NULL, pub_accelerometer},
    {{"Magnetometer"}, NULL, pub_magnetometer},
    {{"Barometer"}, NULL, pub_barometer},
    {{"Gyroscope"}, NULL, pub_gyroscope},
    {{"Humidity"}, NULL, pub_humidity},
    {{"Button"}, NULL, pub_button},
    {{"LED"}, &on_led_msg, NULL},
    {{"Temperature"}, NULL, pub_temperature},
};

/* store the length of the array */
static const size_t topics_array_length =
    sizeof( topics_array ) / sizeof( topics_array[0] );

xi_context_handle_t gXivelyContextHandle      = XI_INVALID_CONTEXT_HANDLE;
xi_timed_task_handle_t gXivelyTimedTaskHandle = XI_INVALID_TIMED_TASK_HANDLE;

/* The pointer returned by this function must be free()d by the caller */
static char* build_xively_topic( char* topic_name, char* account_id, char* device_id )
{
    char* dst  = NULL;
    int retval = 0;

    if ( ( NULL == topic_name ) || ( NULL == account_id ) || ( NULL == device_id ) )
    {
        return NULL;
    }

    dst = malloc( XI_TOPIC_MAX_LEN );
    if ( dst == NULL )
    {
        return NULL;
    }

    retval = snprintf( dst, XI_TOPIC_MAX_LEN, "xi/blue/v1/%.64s/d/%.64s/%.64s",
                       account_id, device_id, topic_name );
    if ( ( retval >= XI_TOPIC_MAX_LEN ) || ( retval < 0 ) )
    {
        return NULL;
    }
    return dst;
}

/* handler for led msg */
void on_led_msg( const uint8_t* const msg, size_t msg_size )
{
    printf( "\r\n>> Got a new MQTT message in the LED topic" );
    printf( "\r\n\t Message size: %d", msg_size );
    printf( "\r\n\t Message payload: " );
    for ( size_t i = 0; i < msg_size; i++ )
    {
        printf( "0x%02x ", msg[i] );
    }
    if ( msg_size != 1 )
    {
        printf( "\r\n\tUnrecognized LED message [IGNORED] Expected 1 or 0" );
        return;
    }
    ( msg[0] == '0' ) ? io_led_off() : io_led_on();
}

xi_state_t pub_accelerometer( const mqtt_topic_descr_t* const mqtt_topic_descr )
{
    char out_msg[IO_AXES_JSON_BUFFER_MAX_SIZE] = "";
    SensorAxes_t sensor_input;

    if ( io_read_accelero( &sensor_input ) < 0 )
    {
        printf( "\r\n\t[ERROR] trying to read accelerometer input" );
        return -1;
    } if ( io_axes_to_json( sensor_input, out_msg, IO_AXES_JSON_BUFFER_MAX_SIZE ) < 0 )
    {
        printf( "\r\n\t[ERROR] trying to create JSON string from sensor input" );
        return -1;
    }
    return xi_publish( gXivelyContextHandle, mqtt_topic_descr->name, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_magnetometer( const mqtt_topic_descr_t* const mqtt_topic_descr )
{
    char out_msg[IO_AXES_JSON_BUFFER_MAX_SIZE] = "";
    SensorAxes_t sensor_input;

    if ( io_read_magneto( &sensor_input ) < 0 )
    {
        printf( "\r\n\t[ERROR] trying to read magnetometer input" );
        return -1;
    }
    if ( io_axes_to_json( sensor_input, out_msg, IO_AXES_JSON_BUFFER_MAX_SIZE ) < 0 )
    {
        printf( "\r\n\t[ERROR] trying to create JSON string from sensor input" );
        return -1;
    }
    return xi_publish( gXivelyContextHandle, mqtt_topic_descr->name, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_gyroscope( const mqtt_topic_descr_t* const mqtt_topic_descr )
{
    char out_msg[IO_AXES_JSON_BUFFER_MAX_SIZE] = "";
    SensorAxes_t sensor_input;

    if ( io_read_gyro( &sensor_input ) < 0 )
    {
        printf( "\r\n\t[ERROR] trying to read gyroscope input" );
        return -1;
    }
    if ( io_axes_to_json( sensor_input, out_msg, IO_AXES_JSON_BUFFER_MAX_SIZE ) < 0 )
    {
        printf( "\r\n\t[ERROR] trying to create JSON string from sensor input" );
        return -1;
    }
    return xi_publish( gXivelyContextHandle, mqtt_topic_descr->name, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_barometer( const mqtt_topic_descr_t* const mqtt_topic_descr )
{
    char out_msg[IO_FLOAT_BUFFER_MAX_SIZE] = "";
    float sensor_input                     = 0;
    if ( io_read_pressure( &sensor_input ) < 0 )
    {
        printf( "\r\n\t[ERROR] trying to read barometer input" );
        return -1;
    }
    if ( io_float_to_string( sensor_input, out_msg, IO_FLOAT_BUFFER_MAX_SIZE ) )
    {
        printf( "\r\n\t[ERROR] trying to create string from sensor input" );
        return -1;
    }
    return xi_publish( gXivelyContextHandle, mqtt_topic_descr->name, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_humidity( const mqtt_topic_descr_t* const mqtt_topic_descr )
{
    char out_msg[IO_FLOAT_BUFFER_MAX_SIZE] = "";
    float sensor_input                     = 0;
    if ( io_read_humidity( &sensor_input ) < 0 )
    {
        printf( "\r\n\t[ERROR] trying to read hygrometer input" );
        return -1;
    }
    if ( io_float_to_string( sensor_input, out_msg, IO_FLOAT_BUFFER_MAX_SIZE ) )
    {
        printf( "\r\n\t[ERROR] trying to create string from sensor input" );
        return -1;
    }
    return xi_publish( gXivelyContextHandle, mqtt_topic_descr->name, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_temperature( const mqtt_topic_descr_t* const mqtt_topic_descr )
{
    char out_msg[IO_FLOAT_BUFFER_MAX_SIZE] = "";
    float sensor_input                     = 0;
    if ( io_read_temperature( &sensor_input ) < 0 )
    {
        printf( "\r\n\t[ERROR] trying to read thermometer input" );
        return -1;
    }
    if ( io_float_to_string( sensor_input, out_msg, IO_FLOAT_BUFFER_MAX_SIZE ) )
    {
        printf( "\r\n\t[ERROR] trying to create string from sensor input" );
        return -1;
    }
    return xi_publish( gXivelyContextHandle, mqtt_topic_descr->name, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_button( const mqtt_topic_descr_t* const mqtt_topic_descr )
{
    char* payload;
    ( io_read_button() ) ? ( payload = "1" ) : ( payload = "0" );
    return xi_publish( gXivelyContextHandle, mqtt_topic_descr->name, payload,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

void pub_button_interrupt( void )
{
    /* TODO: Verify we are connected to Xively before calling xi_publish */
    const char payload[] = "1";
    char* button_topic   = build_xively_topic( "Button", user_config.xi_account_id,
                                             user_config.xi_device_id );

    if ( NULL != button_topic )
    {
        xi_publish( gXivelyContextHandle, button_topic, payload, XI_MQTT_QOS_AT_MOST_ONCE,
                    XI_MQTT_RETAIN_FALSE, NULL, NULL );
    }
    else
    {
        printf( "\r\n>> Button MQTT topic construction [ERROR]" );
    }
    free( button_topic );
}

void on_connected( xi_context_handle_t in_context_handle, void* data, xi_state_t state )
{
    /* sanity check */
    assert( NULL != data );

    xi_connection_data_t* conn_data = ( xi_connection_data_t* )data;

    printf( "\r\nconnected, state : %i", conn_data->connection_state );
    switch ( conn_data->connection_state )
    {
        /* connection attempt failed */
        case XI_CONNECTION_STATE_OPEN_FAILED:
            printf(
                "[%d] Xively: Connection failed %s:%d, error %d, reconnecting....\r\n",
                ( int )xi_bsp_time_getcurrenttime_seconds(), conn_data->host,
                conn_data->port, state );

            xi_connect( in_context_handle, conn_data->username, conn_data->password,
                        conn_data->connection_timeout, conn_data->keepalive_timeout,
                        conn_data->session_type, &on_connected );
            break;
        /* connection has been closed */
        case XI_CONNECTION_STATE_CLOSED:
            /* unregister task handle */
            xi_cancel_timed_task( gXivelyTimedTaskHandle );
            gXivelyTimedTaskHandle = XI_INVALID_TIMED_TASK_HANDLE;

            printf(
                "[%d] Xively: Connection closed %s:%d reason %d, reconnecting....\r\n",
                ( int )xi_bsp_time_getcurrenttime_seconds(), conn_data->host,
                conn_data->port, state );

            xi_connect( in_context_handle, conn_data->username, conn_data->password,
                        conn_data->connection_timeout, conn_data->keepalive_timeout,
                        conn_data->session_type, &on_connected );
            break;
        /* connection has been established */
        case XI_CONNECTION_STATE_OPENED:
            printf( "[%d] Xively: Connected to %s:%d\r\n",
                    ( int )xi_bsp_time_getcurrenttime_seconds(), conn_data->host,
                    conn_data->port );

            {
                /* here put the activation code */
                init_xively_topics( in_context_handle );
            }

            break;
        /* something really bad happened */
        default:
            printf( "[%d] Xively: Invalid connection status %s:%d\r\n",
                    ( int )xi_bsp_time_getcurrenttime_seconds(), conn_data->host,
                    conn_data->port );
    };
}

/* handler for xively mqtt topic subscription */
void on_mqtt_topic( xi_context_handle_t in_context_handle,
                    xi_sub_call_type_t call_type,
                    const xi_sub_call_params_t* const params,
                    xi_state_t state,
                    void* user_data )
{
    /* sanity checks */
    assert( NULL != user_data );

    /* user data is the mqtt_topic_configuration_t */
    const mqtt_topic_configuration_t* topic_conf =
        ( const mqtt_topic_configuration_t* )user_data;

    switch ( call_type )
    {
        case XI_SUB_CALL_SUBACK:
            if ( XI_MQTT_SUBACK_FAILED == params->suback.suback_status )
            {
                printf( "Subscription failed for %s topic\r\n", params->suback.topic );
            }
            else
            {
                printf( "Subscription successful for %s topic\r\n",
                        params->suback.topic );
            }
            return;
        case XI_SUB_CALL_MESSAGE:
            printf( "received message on %s topic\r\n", params->suback.topic );
            /* if there is a handler call it with the message payload */
            if ( NULL != topic_conf->on_msg_pull )
            {
                topic_conf->on_msg_pull( params->message.temporary_payload_data,
                                         params->message.temporary_payload_data_length );
            }
            break;
        default:
            return;
    }
}

void push_mqtt_topics()
{
    char* xi_topic = NULL;
    int i = 0;
    for ( ; i < topics_array_length; ++i )
    {
        const mqtt_topic_configuration_t* const topic_config = &topics_array[i];
        xi_topic = build_xively_topic( ( char* ) topic_config->topic_descr.name,
                                       user_config.xi_account_id,
                                       user_config.xi_device_id );

        if ( NULL == xi_topic )
        {
            continue;
        }

        if ( NULL != topic_config->on_msg_push )
        {
            topic_config->on_msg_push( xi_topic );
        }
        free( xi_topic );
    }
}

/*
 * Initializes Xively's demo application topics
 */
xi_state_t init_xively_topics( xi_context_handle_t in_context_handle )
{
    xi_state_t ret = XI_STATE_OK;
    char* xi_topic = NULL;

    int i = 0;
    for ( ; i < topics_array_length; ++i )
    {
        const mqtt_topic_configuration_t* const topic_config = &topics_array[i];
        xi_topic = build_xively_topic( ( char* ) topic_config->topic_descr.name,
                                       user_config.xi_account_id,
                                       user_config.xi_device_id );

        if ( xi_topic == NULL )
        {
            ret = XI_FAILED_INITIALIZATION;
            goto error_out;
        }

        if ( NULL != topic_config->on_msg_pull )
        {
            printf( "\r\n>> Subscribing to MQTT topic [%s]", xi_topic );
            ret = xi_subscribe( in_context_handle, xi_topic,
                                XI_MQTT_QOS_AT_MOST_ONCE, &on_mqtt_topic,
                                ( void* )topic_config );

            if ( XI_STATE_OK != ret )
            {
                goto error_out;
            }
        }

        free( xi_topic );
    }

    /* registration of the publish function */
    gXivelyTimedTaskHandle = xi_schedule_timed_task( in_context_handle, &push_mqtt_topics,
                                                     XI_PUBLISH_INTERVAL_SEC, 1, NULL );

    if ( XI_INVALID_TIMED_TASK_HANDLE == gXivelyTimedTaskHandle )
    {
        printf( "timed task couldn't been registered\r\n" );
        ret = XI_FAILED_INITIALIZATION;
        goto error_out;
    }

    return ret;

error_out:
    if ( NULL != xi_topic )
    {
        free( xi_topic );
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

    if ( user_config_init() < 0 )
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
                    printf( "Socket not opened!" );
                }

                wifi_state = wifi_state_idle;
                break;

            case wifi_state_idle:
                printf( "." );
                fflush( stdout );
                HAL_Delay( 500 );
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

static int8_t user_config_init( void )
{
#if USE_FLASH_STORAGE
    if ( user_data_copy_from_flash( &user_config ) < 0 )
    {
        printf( "\r\n>> [ERROR] trying to copy user data from flash" );
        return -1;
    }
    printf( "\r\n>> User data retrieved from flash:" );
    user_data_printf( &user_config );
    fflush( stdout );

    if ( io_read_button() || ( user_data_validate_checksum( &user_config ) < 0 ) )
    {
        printf( "\r\n>> Starting forced user configuration mode" );
        print_user_config_debug_banner();

        /* Get WiFi AP Credentials from the user */
        if ( get_ap_credentials( &user_config ) < 0 )
        {
            printf( "\r\n>> [ERROR] Getting AP credentials from user" );
            return -1;
        }

        printf( "\r\n>> Saving user data to flash" );
        user_data_save_to_flash( &user_config );

        /* Get Xively Credentials from the user */
        if ( get_xively_credentials( &user_config ) < 0 )
        {
            printf( "\r\n>> [ERROR] Getting Xively credentials from user" );
            return -1;
        }

        printf( "\r\n>> Saving user data to flash" );
        user_data_save_to_flash( &user_config );
    }
#else
    print_user_config_debug_banner();

    /* Get Xively Credentials from the user */
    if ( get_ap_credentials( &user_config ) < 0 )
    {
        printf( "\r\n>> [ERROR] Getting AP credentials from user" );
        return -1;
    }

    /* Get WiFi AP Credentials from the user */
    if ( get_xively_credentials( &user_config ) < 0 )
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
