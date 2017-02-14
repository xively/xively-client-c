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

/* Xively's headers */
#include "xively.h"
#include "xi_bsp_io_net_socket_proxy.h"
#include "xi_bsp_time.h"
#include "demo_io.h"

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
/* Private macro -------------------------------------------------------------*/
#define WIFI_SCAN_BUFFER_LIST 25

/* Private variables ---------------------------------------------------------*/
volatile uint8_t button_pressed_interrupt_flag = 0;

/* Private function prototypes -----------------------------------------------*/
WiFi_Status_t wifi_get_AP_settings( void );

wifi_state_t wifi_state;
wifi_config config;
wifi_scan net_scan[WIFI_SCAN_BUFFER_LIST];

char console_ssid[40];
char console_psk[20];

char* ssid          = "SSID";
char* seckey        = "PASSWORD";
WiFi_Priv_Mode mode = WPA_Personal;

#define XI_ACCOUNT_ID "XIVELY ACCOUNT ID"
#define XI_DEVICE_ID "XIVELY DEVICE ID"
#define XI_DEVICE_PASS "XIVELY DEVICE PASSWORD"

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
#define XI_TOPIC_NAME_MANGLE( name ) \
    "xi/blue/v1/" XI_ACCOUNT_ID "/d/" XI_DEVICE_ID "/" name

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
    {{XI_TOPIC_NAME_MANGLE( "Accelerometer" )}, NULL, pub_accelerometer},
    {{XI_TOPIC_NAME_MANGLE( "Magnetometer" )}, NULL, pub_magnetometer},
    {{XI_TOPIC_NAME_MANGLE( "Barometer" )}, NULL, pub_barometer},
    {{XI_TOPIC_NAME_MANGLE( "Gyroscope" )}, NULL, pub_gyroscope},
    {{XI_TOPIC_NAME_MANGLE( "Humidity" )}, NULL, pub_humidity},
    {{XI_TOPIC_NAME_MANGLE( "Button" )}, NULL, pub_button},
    {{XI_TOPIC_NAME_MANGLE( "LED" )}, &on_led_msg, NULL},
    {{XI_TOPIC_NAME_MANGLE( "Temperature" )}, NULL, pub_temperature},
};

/* store the length of the array */
static const size_t topics_array_length =
    sizeof( topics_array ) / sizeof( topics_array[0] );

xi_context_handle_t gXivelyContextHandle      = XI_INVALID_CONTEXT_HANDLE;
xi_timed_task_handle_t gXivelyTimedTaskHandle = XI_INVALID_TIMED_TASK_HANDLE;

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
    xi_publish( gXivelyContextHandle, XI_TOPIC_NAME_MANGLE( "Button" ), payload,
                XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
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
    int i = 0;
    for ( ; i < topics_array_length; ++i )
    {
        const mqtt_topic_configuration_t* const topic_config = &topics_array[i];

        if ( NULL != topic_config->on_msg_push )
        {
            topic_config->on_msg_push( &topic_config->topic_descr );
        }
    }
}

/*
 * Initializes Xively's demo application topics
 */
xi_state_t init_xively_topics( xi_context_handle_t in_context_handle )
{
    xi_state_t ret = XI_STATE_OK;

    int i = 0;
    for ( ; i < topics_array_length; ++i )
    {
        const mqtt_topic_configuration_t* const topic_config = &topics_array[i];

        if ( NULL != topic_config->on_msg_pull )
        {
            ret = xi_subscribe( in_context_handle, topic_config->topic_descr.name,
                                XI_MQTT_QOS_AT_MOST_ONCE, &on_mqtt_topic,
                                ( void* )topic_config );

            if ( XI_STATE_OK != ret )
            {
                return ret;
            }
        }
    }

    /* registration of the publish function */
    gXivelyTimedTaskHandle = xi_schedule_timed_task( in_context_handle, &push_mqtt_topics,
                                                     XI_PUBLISH_INTERVAL_SEC, 1, NULL );

    if ( XI_INVALID_TIMED_TASK_HANDLE == gXivelyTimedTaskHandle )
    {
        printf( "timed task couldn't been registered\r\n" );
        return XI_FAILED_INITIALIZATION;
    }

    return ret;
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
    __GPIOA_CLK_ENABLE();
    HAL_Init();

    /* Configure the system clock to 64 MHz */
    SystemClock_Config();
    SystemCoreClockUpdate();

    /* configure the timers  */
    Timer_Config();

    UART_Configuration( 115200 );
#ifdef USART_PRINT_MSG
    UART_Msg_Gpio_Init();
    USART_PRINT_MSG_Configuration( 115200 );
#endif

    printf( "\r\n>> Initializing the sensor extension board" );
    io_nucleoboard_init();
    io_sensorboard_init();
    io_sensorboard_enable();

    config.power       = wifi_sleep;
    config.power_level = high;
    config.dhcp        = on; // use DHCP IP address
    config.web_server  = WIFI_TRUE;

    wifi_state = wifi_state_idle;

    status = wifi_get_AP_settings();
    if ( status != WiFi_MODULE_SUCCESS )
    {
        printf( "\r\nError in AP Settings" );
        return 0;
    }

    printf( "\r\n>> Initializing the WiFi extension board" );

    /* Init the wi-fi module */
    status = wifi_init( &config );
    if ( status != WiFi_MODULE_SUCCESS )
    {
        printf( "Error in Config" );
        return 0;
    }

    while ( 1 )
    {
        xi_events_process_tick();
        switch ( wifi_state )
        {
            case wifi_state_reset:
                break;

            case wifi_state_ready:

                printf( "\r\n >>running WiFi Scan...\r\n" );

                status = wifi_network_scan( net_scan, WIFI_SCAN_BUFFER_LIST );

                if ( status == WiFi_MODULE_SUCCESS )
                {
                    for ( i = 0; i < WIFI_SCAN_BUFFER_LIST; i++ )
                    {
                        // printf(net_scan[i].ssid);
                        // printf("\r\n");
                        if ( ( ( char* )strstr( ( const char* )net_scan[i].ssid,
                                                ( const char* )console_ssid ) ) != NULL )
                        {
                            printf( "\r\n >>network present...connecting to AP...\r\n" );
                            SSID_found = WIFI_TRUE;
                            wifi_connect( console_ssid, console_psk, mode );
                            break;
                        }
                    }
                    if ( !SSID_found )
                    {
                        printf( "\r\nGiven SSID not found!\r\n" );
                    }
                    memset( net_scan, 0x00, sizeof( net_scan ) );
                }
                wifi_state = wifi_state_idle;

                break;

            case wifi_state_connected:

                printf( "\r\n >>connected...\r\n" );

                wifi_state = wifi_state_idle;

                HAL_Delay( 2000 ); // Let module go to sleep

                wifi_wakeup( WIFI_TRUE ); /*wakeup from sleep if module went to sleep*/

                break;

            case wifi_state_disconnected:
                wifi_state = wifi_state_reset;
                break;

            case wifi_state_socket:
                printf( "\r\n >>Connecting to Xively socket\r\n" );

                if ( socket_open == 0 )
                {
                    /* Read Write Socket data */
                    xi_state_t ret_state =
                        xi_initialize( XI_ACCOUNT_ID, XI_DEVICE_ID, 0 );

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

                    xi_connect( gXivelyContextHandle, XI_DEVICE_ID, XI_DEVICE_PASS, 10, 0,
                                XI_SESSION_CLEAN, &on_connected );
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

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (HSI)
 *            SYSCLK(Hz)                     = 64000000
 *            HCLK(Hz)                       = 64000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 2
 *            APB2 Prescaler                 = 1
 *            PLLMUL                         = 16
 *            Flash Latency(WS)              = 2
 * @param  None
 * @retval None
 */

#ifdef USE_STM32F1xx_NUCLEO

void SystemClock_Config( void )
{
    RCC_ClkInitTypeDef clkinitstruct = {0};
    RCC_OscInitTypeDef oscinitstruct = {0};

    /* Configure PLL ------------------------------------------------------*/
    /* PLL configuration: PLLCLK = (HSI / 2) * PLLMUL = (8 / 2) * 16 = 64 MHz */
    /* PREDIV1 configuration: PREDIV1CLK = PLLCLK / HSEPredivValue = 64 / 1 = 64 MHz */
    /* Enable HSI and activate PLL with HSi_DIV2 as source */
    oscinitstruct.OscillatorType      = RCC_OSCILLATORTYPE_HSE;
    oscinitstruct.HSEState            = RCC_HSE_ON;
    oscinitstruct.LSEState            = RCC_LSE_OFF;
    oscinitstruct.HSIState            = RCC_HSI_OFF;
    oscinitstruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    oscinitstruct.HSEPredivValue      = RCC_HSE_PREDIV_DIV1;
    oscinitstruct.PLL.PLLState        = RCC_PLL_ON;
    oscinitstruct.PLL.PLLSource       = RCC_PLLSOURCE_HSE;
    oscinitstruct.PLL.PLLMUL          = RCC_PLL_MUL9;
    if ( HAL_RCC_OscConfig( &oscinitstruct ) != HAL_OK )
    {
        /* Initialization Error */
        while ( 1 )
            ;
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
       clocks dividers */
    clkinitstruct.ClockType = ( RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 );
    clkinitstruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    clkinitstruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV1;
    clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV2;
    if ( HAL_RCC_ClockConfig( &clkinitstruct, FLASH_LATENCY_2 ) != HAL_OK )
    {
        /* Initialization Error */
        while ( 1 )
            ;
    }
}
#endif

#ifdef USE_STM32F4XX_NUCLEO

void SystemClock_Config( void )
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;

    /* Enable Power Control clock */
    __PWR_CLK_ENABLE();

    /* The voltage scaling allows optimizing the power consumption when the device is
       clocked below the maximum system frequency, to update the voltage scaling value
       regarding system frequency refer to product datasheet.  */
    __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE2 );

    /* Enable HSI Oscillator and activate PLL with HSI as source */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = 0x10;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM            = 16;
    RCC_OscInitStruct.PLL.PLLN            = 336;
    RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ            = 7;
    HAL_RCC_OscConfig( &RCC_OscInitStruct );

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
       clocks dividers */
    RCC_ClkInitStruct.ClockType = ( RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                    RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 );
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_2 );
}
#endif

#ifdef USE_STM32L0XX_NUCLEO

/**
 * @brief  System Clock Configuration
 * @param  None
 * @retval None
 */
void SystemClock_Config( void )
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;

    __PWR_CLK_ENABLE();

    __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );

    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = 0x10;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLMUL          = RCC_PLLMUL_4;
    RCC_OscInitStruct.PLL.PLLDIV          = RCC_PLLDIV_2;
    HAL_RCC_OscConfig( &RCC_OscInitStruct );

    RCC_ClkInitStruct.ClockType =
        ( RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 |
          RCC_CLOCKTYPE_PCLK2 );                             // RCC_CLOCKTYPE_SYSCLK;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_HSI; // RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_1 );

    __SYSCFG_CLK_ENABLE();
}
#endif

#ifdef USE_STM32L4XX_NUCLEO
/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (MSI)
 *            SYSCLK(Hz)                     = 80000000
 *            HCLK(Hz)                       = 80000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 1
 *            APB2 Prescaler                 = 1
 *            MSI Frequency(Hz)              = 4000000
 *            PLL_M                          = 1
 *            PLL_N                          = 40
 *            PLL_R                          = 2
 *            PLL_P                          = 7
 *            PLL_Q                          = 4
 *            Flash Latency(WS)              = 4
 * @param  None
 * @retval None
 */
void SystemClock_Config( void )
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

    /* MSI is enabled after System reset, activate PLL with MSI as source */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState            = RCC_MSI_ON;
    RCC_OscInitStruct.MSIClockRange       = RCC_MSIRANGE_6;
    RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_MSI;
    RCC_OscInitStruct.PLL.PLLM            = 1;
    RCC_OscInitStruct.PLL.PLLN            = 40;
    RCC_OscInitStruct.PLL.PLLR            = 2;
    RCC_OscInitStruct.PLL.PLLP            = 7;
    RCC_OscInitStruct.PLL.PLLQ            = 4;
    HAL_RCC_OscConfig( &RCC_OscInitStruct );

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
       clocks dividers */
    RCC_ClkInitStruct.ClockType = ( RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                    RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 );
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_4 );
}
#endif

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

    /* Infinite loop */
    while ( 1 )
    {
    }
}
#endif

/**
 * @brief  Query the User for SSID, password, encryption mode and hostname
 * @param  None
 * @retval WiFi_Status_t
 */
WiFi_Status_t wifi_get_AP_settings( void )
{
    uint8_t console_input[1], console_count = 0;
    //  wifi_bool set_AP_config = WIFI_FALSE;
    WiFi_Status_t status = WiFi_MODULE_SUCCESS;
    printf( "\r\n\n/********************************************************\n" );
    printf( "\r *                                                      *\n" );
    printf( "\r * X-CUBE-WIFI1 Expansion Software v2.1.1               *\n" );
    printf( "\r * X-NUCLEO-IDW01M1 Wi-Fi Configuration.                *\n" );
    printf( "\r * Client-Socket Example                                *\n" );
    printf( "\r *                                                      *\n" );
    printf( "\r *******************************************************/\n" );
    printf( "\r\nDo you want to setup SSID?(y/n):" );
    fflush( stdout );
    scanf( "%s", console_input );
    printf( "\r\n" );

    // HAL_UART_Receive(&UartMsgHandle, (uint8_t *)console_input, 1, 100000);
    if ( console_input[0] == 'y' )
    {
        //              set_AP_config = WIFI_TRUE;
        printf( "Enter the SSID:" );
        fflush( stdout );

        console_count = 0;
        console_count = scanf( "%s", console_ssid );
        printf( "\r\n" );

        if ( console_count == 39 )
        {
            printf( "Exceeded number of ssid characters permitted" );
            return WiFi_NOT_SUPPORTED;
        }

        // printf("entered =%s\r\n",console_ssid);
        printf( "Enter the password:" );
        fflush( stdout );
        console_count = 0;

        console_count = scanf( "%s", console_psk );
        printf( "\r\n" );
        // printf("entered =%s\r\n",console_psk);
        if ( console_count == 19 )
        {
            printf( "Exceeded number of psk characters permitted" );
            return WiFi_NOT_SUPPORTED;
        }
        printf( "Enter the encryption mode(0:Open, 1:WEP, 2:WPA2/WPA2-Personal):" );
        fflush( stdout );
        scanf( "%s", console_input );
        printf( "\r\n" );
        // printf("entered =%s\r\n",console_input);
        switch ( console_input[0] )
        {
            case '0':
                mode = None;
                break;
            case '1':
                mode = WEP;
                break;
            case '2':
                mode = WPA_Personal;
                break;
            default:
                printf( "\r\nWrong Entry. Priv Mode is not compatible\n" );
                return WiFi_NOT_SUPPORTED;
        }
    }
    else
    {
        printf( "\r\n\n Module will connect with default settings." );
        memcpy( console_ssid, ( const char* )ssid, strlen( ( char* )ssid ) );
        memcpy( console_psk, ( const char* )seckey, strlen( ( char* )seckey ) );
    }

    printf( "\r\n\n/**************************************************************\n" );
    printf( "\r * Configuration Complete                                     *\n" );
    printf( "\r * Port Number:32000, Protocol: TCP/IP                        *\n" );
    printf( "\r *************************************************************/\n" );

    return status;
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
    printf( "\r\nWiFi Initialised and Ready..\r\n" );
    wifi_state = wifi_state_ready;
}

void ind_wifi_connected()
{
    wifi_state = wifi_state_connected;
}

void ind_wifi_resuming()
{
    printf( "\r\nwifi resuming from sleep user callback... \r\n" );
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
