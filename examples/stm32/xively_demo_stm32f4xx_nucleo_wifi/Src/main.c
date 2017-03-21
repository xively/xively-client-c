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

/* Private function prototypes -----------------------------------------------*/
WiFi_Status_t wifi_get_AP_settings( void );

wifi_state_t wifi_state;
wifi_config config;
wifi_scan net_scan[WIFI_SCAN_BUFFER_LIST];
xi_state_t xi_connection_state = XI_SOCKET_NO_ACTIVE_CONNECTION_ERROR;
uint8_t is_burst_mode          = 0;

char console_ssid[40];
char console_psk[20];

char* ssid          = "WiFi Network Name";
char* seckey        = "WiFi Network Password";
WiFi_Priv_Mode mode = WPA_Personal;

#define XI_ACCOUNT_ID "Xively Account ID"
#define XI_DEVICE_ID "Xively Device ID"
#define XI_DEVICE_PASS "Xively Device Password"

/* the interval for the time function */
#define XI_PUBLISH_INTERVAL_SEC 5

/* combines name with the account and device id */
#define XI_TOPIC_NAME_MANGLE( name )                                                     \
    "xi/blue/v1/" XI_ACCOUNT_ID "/d/" XI_DEVICE_ID "/" name

/* declaration of topic handlers for PUB'ed topics */
static xi_state_t pub_accelerometer( const char* topic_name );
static xi_state_t pub_magnetometer( const char* topic_name );
static xi_state_t pub_barometer( const char* topic_name );
static xi_state_t pub_button( const char* topic_name );
static xi_state_t pub_gyroscope( const char* topic_name );
static xi_state_t pub_humidity( const char* topic_name );
static xi_state_t pub_temperature( const char* topic_name );

/* topic's initialisation function */
static xi_state_t init_xively_topics( xi_context_handle_t in_context_handle );

xi_context_handle_t gXivelyContextHandle      = XI_INVALID_CONTEXT_HANDLE;
xi_timed_task_handle_t gXivelyTimedTaskHandle = XI_INVALID_TIMED_TASK_HANDLE;

xi_state_t pub_accelerometer( const char* topic_name )
{
    char out_msg[IO_AXES_JSON_BUFFER_MAX_SIZE] = "";
    SensorAxes_t sensor_input;

    if ( io_read_accelero( &sensor_input ) < 0 )
    {
        printf( "\r\n\t[ERROR] trying to read accelerometer input" );
        return -1;
    }
    if ( io_axes_to_json( sensor_input, out_msg, IO_AXES_JSON_BUFFER_MAX_SIZE ) < 0 )
    {
        printf( "\r\n\t[ERROR] trying to create JSON string from sensor input" );
        return -1;
    }

    return xi_publish( gXivelyContextHandle, topic_name, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_magnetometer( const char* topic_name )
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
    return xi_publish( gXivelyContextHandle, topic_name, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_gyroscope( const char* topic_name )
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
    return xi_publish( gXivelyContextHandle, topic_name, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_barometer( const char* topic_name )
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
    return xi_publish( gXivelyContextHandle, topic_name, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_humidity( const char* topic_name )
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
    return xi_publish( gXivelyContextHandle, topic_name, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_temperature( const char* topic_name )
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
    printf( "temp topic name: \"%s\"\n\r", topic_name );
    return xi_publish( gXivelyContextHandle, topic_name, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_button( const char* topic_name )
{
    char* payload;
    ( io_read_button() ) ? ( payload = "1" ) : ( payload = "0" );
    return xi_publish( gXivelyContextHandle, topic_name, payload,
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
    xi_connection_state             = conn_data->connection_state;

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
                printf( "Subscription failed for LED topic\r\n" );
            }
            else
            {
                printf( "Subscription successful for LED topic\r\n",
                        params->suback.topic );
            }
            return;
        case XI_SUB_CALL_MESSAGE:
            printf( "received message on LED topic\r\n" );
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

void publish_mqtt_topics()
{
    if ( !is_burst_mode )
    {
        pub_gyroscope( XI_TOPIC_NAME_MANGLE( "Gyroscope" ) );
        pub_accelerometer( XI_TOPIC_NAME_MANGLE( "Accelerometer" ) );
    }

    pub_button( XI_TOPIC_NAME_MANGLE( "Button" ) );
    pub_magnetometer( XI_TOPIC_NAME_MANGLE( "Magnetometer" ) );
    pub_barometer( XI_TOPIC_NAME_MANGLE( "Barometer" ) );
    pub_humidity( XI_TOPIC_NAME_MANGLE( "Humidity" ) );
    pub_temperature( XI_TOPIC_NAME_MANGLE( "Temperature" ) );
    printf( "\n\r" );
}

/*
 * Initializes Xively's demo application topics
 */
xi_state_t init_xively_topics( xi_context_handle_t in_context_handle )
{
    xi_state_t ret = XI_STATE_OK;

    ret = xi_subscribe( in_context_handle, XI_TOPIC_NAME_MANGLE( "LED" ),
                        XI_MQTT_QOS_AT_MOST_ONCE, &on_led_msg, NULL );

    if ( XI_STATE_OK != ret )
    {
        return ret;
    }

    /* registration of the publish function */
    gXivelyTimedTaskHandle = xi_schedule_timed_task(
        in_context_handle, &publish_mqtt_topics, XI_PUBLISH_INTERVAL_SEC, 1, NULL );

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
                printf( "\r\n >>Connecting to Xively\r\n" );

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
                HAL_Delay( 100 );

                if ( is_burst_mode && XI_CONNECTION_STATE_OPENED == xi_connection_state )
                {
                    pub_gyroscope( XI_TOPIC_NAME_MANGLE( "Gyroscope" ) );
                    pub_accelerometer( XI_TOPIC_NAME_MANGLE( "Accelerometer" ) );
                    pub_button( XI_TOPIC_NAME_MANGLE( "Button" ) );
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
            printf( "Entering Burst Mode!\n\r" );
        }
        else
        {
            printf( "Entering Standard Mode!\n\r" );
        }

        if ( io_button_exti_debouncer( GPIO_Pin ) )
        {
            printf( "\r\n>> Nucleo board button [PRESSED]" );
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
    WiFi_Status_t status = WiFi_MODULE_SUCCESS;
    printf( "\r\n\n/********************************************************\n" );
    printf( "\r *\n" );
    printf( "\r * Xively Client Example version 1.0\n" );
    printf( "\r * for STM32F401RE Nucleo\n" );
    printf( "\r * X-CUBE-WIFI1 Expansion Software v2.1.1\n" );
    printf( "\r * X-NUCLEO-IDW01M1 Wi-Fi Configuration.\n" );
    printf( "\r *\n" );
    printf( "\r *******************************************************/\n" );
    printf( "\r\nDo you want to setup SSID?(y/n):" );
    fflush( stdout );
    scanf( "%s", console_input );
    printf( "\r\n" );

    if ( console_input[0] == 'y' )
    {
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

        printf( "Enter the password:" );
        fflush( stdout );
        console_count = 0;

        console_count = scanf( "%s", console_psk );
        printf( "\r\n" );
        if ( console_count == 19 )
        {
            printf( "Exceeded number of psk characters permitted" );
            return WiFi_NOT_SUPPORTED;
        }
        printf( "Enter the encryption mode(0:Open, 1:WEP, 2:WPA2/WPA2-Personal):" );
        fflush( stdout );
        scanf( "%s", console_input );
        printf( "\r\n" );
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
    printf( "\r * Configuration Complete\n" );
    printf( "\r * Port Number:32000, Protocol: TCP/IP\n" );
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
