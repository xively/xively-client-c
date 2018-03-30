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
#include "stdio.h"
#include "string.h"
#include "wifi_module.h"
#include "wifi_globals.h"
#include "wifi_interface.h"

#include "user_data.h"
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
 * The software runs on STM32 and it can be used for building Wi-Fi applications using the
 * SPWF01Sx device. It is built on top of STM32Cube software technology that eases
 * portability across different STM32 microcontrollers.
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
void SystemClock_Config( void );
void UART_Msg_Gpio_Init( void );
WiFi_Status_t wifi_get_AP_settings( void );
void USART_PRINT_MSG_Configuration( UART_HandleTypeDef* UART_MsgHandle,
                                    uint32_t baud_rate );


/* Private Declarartion ------------------------------------------------------*/

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

wifi_config config;
wifi_state_t wifi_state;
UART_HandleTypeDef UART_MsgHandle;

wifi_scan net_scan[WIFI_SCAN_BUFFER_LIST];

char* ssid          = "STM";
char* seckey        = "STMdemoPWD";
WiFi_Priv_Mode mode = WPA_Personal;
char* hostname      = "192.168.0.101"; //"time-d.nist.gov";//"4.ifcfg.me"
char* gcfg_key1     = "ip_ipaddr";
char* gcfg_key2     = "nv_model";
uint8_t socket_id;
char echo[512] = "hello";
char wifi_ip_addr[20];
uint16_t len;
uint32_t baud_rate = 115200;

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

#ifdef USART_PRINT_MSG
    UART_Msg_Gpio_Init();
    USART_PRINT_MSG_Configuration( &UART_MsgHandle, 115200 );
    Set_UartMsgHandle( &UART_MsgHandle );
#endif

    config.power         = wifi_active;
    config.power_level   = high;
    config.dhcp          = on; // use DHCP IP address
    config.web_server    = WIFI_TRUE;
    config.mcu_baud_rate = baud_rate;
    wifi_state           = wifi_state_idle;

    UART_Configuration( baud_rate );

    printf( "\r\n\nInitializing the wifi module..." );

    /* Init the wi-fi module */
    if ( WiFi_MODULE_SUCCESS != wifi_init( &config ) )
    {
        printf( "Error in Config" );
        return 0;
    }
}

/**
 * @brief  Main program
 * @param  None
 * @retval None
 */

int main( void )
{
    uint8_t i           = 0;
    uint8_t socket_open = 0;
    wifi_bool SSID_found = WIFI_FALSE;
    WiFi_Status_t status = WiFi_MODULE_SUCCESS;
#if 0
    uint32_t portnumber = 32000; // 23 for 4.ifcfg.me;//37 for nist.gov
    uint16_t len;         /*Take care to change the length of the text we are sending*/
    char* protocol = "t"; // t -> tcp , s-> secure tcp
    char* data     = "Hello World!\r\n";
    len = strlen( data );
#endif

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

    while ( 1 )
    {
        switch ( wifi_state )
        {
            case wifi_state_reset:
                break;

            case wifi_state_ready:
                printf( "\r\n>> Scanning WiFi networks to find SSID [%s]",
                        user_config.wifi_client_ssid );

                wifi_bool SSID_found;
                status = wifi_network_scan( net_scan, WIFI_SCAN_BUFFER_LIST );

                if ( status == WiFi_MODULE_SUCCESS )
                {
                    for ( i = 0; i < WIFI_SCAN_BUFFER_LIST; i++ )
                    {
                        // printf( net_scan[i].ssid );
                        // printf( "\r\n" );
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
                    if ( !SSID_found )
                    {
                        printf( "\r\nGiven SSID not found!\r\n" );
                    }
                    memset( net_scan, 0x00, sizeof( net_scan ) );
                }

                wifi_state = wifi_state_idle;

                break;

            case wifi_state_connected:

                printf( "\r\n>> Connection to WiFi access point [OK]\r\n\t" );

                wifi_state = wifi_state_socket; // wifi_state_idle;

                // HAL_Delay(2000);//Let module go to sleep

                // wifi_wakeup(WIFI_TRUE);/*wakeup from sleep if module went to sleep*/

                break;

            case wifi_state_disconnected:
                wifi_state = wifi_state_reset;
                break;

            case wifi_state_socket:
#if 1
                printf( "\r\n>> Connecting to Xively's MQTT broker using credentials:" );
                printf( "\r\n\t* Account ID:  [%s]", user_config.xi_account_id );
                printf( "\r\n\t* Device ID:   [%s]", user_config.xi_device_id );
                printf( "\r\n\t* Device Pass: ( REDACTED )\n" );
                // printf( "\r\n\t* Device Pass: [%s]\n", user_config.xi_device_password );

                if ( socket_open == 0 )
                {
                    /* Read Write Socket data */
                    xi_state_t ret_state = xi_initialize( user_config.xi_account_id,
                                                          user_config.xi_device_id );
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
#else
                status = wifi_get_IP_address( ( uint8_t* )wifi_ip_addr );
                printf( "\r\n>>IP address is %s\r\n", wifi_ip_addr );

                memset( wifi_ip_addr, 0x00, 20 );

                status = GET_Configuration_Value( gcfg_key2, ( uint32_t* )wifi_ip_addr );
                printf( "\r\n>>model no is %s\r\n", wifi_ip_addr );

                printf( "\r\n >>Connecting to socket\r\n" );

                if ( socket_open == 0 )
                {
                    /* Read Write Socket data */
                    WiFi_Status_t status = WiFi_MODULE_SUCCESS;
                    status =
                        wifi_socket_client_open( ( uint8_t* )console_host, portnumber,
                                                 ( uint8_t* )protocol, &socket_id );

                    if ( status == WiFi_MODULE_SUCCESS )
                    {
                        printf( "\r\n >>Socket Open OK\r\n" );
                        printf( "\r\n >>Socket ID: %d\r\n", socket_id );
                        socket_open = 1;
                    }
                    else
                    {
                        printf( "Socket connection Error" );
                    }
                }
                else
                {
                    printf( "Socket already opened!" );
                }

                wifi_state = wifi_state_socket_write;
                break;
#endif
            case wifi_state_socket_write:
                HAL_Delay( 500 );

                printf( "\r\n >>Writing data to client\r\n" );

                memset( echo, 0x00, 512 );
                snprintf( echo, 6, "hello" );
                len = strlen( echo );
                /* Read Write Socket data */
                status = wifi_socket_client_write( socket_id, len, echo );

                if ( status == WiFi_MODULE_SUCCESS )
                {
                    printf( "\r\n >>Client Socket Write OK \r\n" );
                }
                wifi_state = wifi_state_idle;

                break;

            case wifi_state_idle:
                printf( "." );
                fflush( stdout );
                HAL_Delay( 500 );

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
    clkinitstruct.ClockType      = ( RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
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
    RCC_ClkInitStruct.ClockType      = ( RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
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
          RCC_CLOCKTYPE_PCLK2 );                           // RCC_CLOCKTYPE_SYSCLK;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI; // RCC_SYSCLKSOURCE_PLLCLK;
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
    RCC_ClkInitStruct.ClockType      = ( RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
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


#ifdef USART_PRINT_MSG
void USART_PRINT_MSG_Configuration( UART_HandleTypeDef* UART_MsgHandle,
                                    uint32_t baud_rate )
{
    UART_MsgHandle->Instance        = WIFI_UART_MSG;
    UART_MsgHandle->Init.BaudRate   = baud_rate;
    UART_MsgHandle->Init.WordLength = UART_WORDLENGTH_8B;
    UART_MsgHandle->Init.StopBits   = UART_STOPBITS_1;
    UART_MsgHandle->Init.Parity     = UART_PARITY_NONE;
    UART_MsgHandle->Init.HwFlowCtl =
        UART_HWCONTROL_NONE; // USART_HardwareFlowControl_RTS_CTS;
    UART_MsgHandle->Init.Mode = UART_MODE_TX_RX;

    if ( HAL_UART_DeInit( UART_MsgHandle ) != HAL_OK )
    {
        Error_Handler();
    }
    if ( HAL_UART_Init( UART_MsgHandle ) != HAL_OK )
    {
        Error_Handler();
    }
}

void UART_Msg_Gpio_Init()
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX/RX clock */
    USARTx_PRINT_TX_GPIO_CLK_ENABLE();
    USARTx_PRINT_RX_GPIO_CLK_ENABLE();


    /* Enable USARTx clock */
    USARTx_PRINT_CLK_ENABLE();
    __SYSCFG_CLK_ENABLE();
    /*##-2- Configure peripheral GPIO ##########################################*/
    /* UART TX GPIO pin configuration  */
    GPIO_InitStruct.Pin   = WiFi_USART_PRINT_TX_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
#if defined( USE_STM32L0XX_NUCLEO ) || defined( USE_STM32F4XX_NUCLEO ) ||                \
    defined( USE_STM32L4XX_NUCLEO )
    GPIO_InitStruct.Alternate = PRINTMSG_USARTx_TX_AF;
#endif
    HAL_GPIO_Init( WiFi_USART_PRINT_TX_GPIO_PORT, &GPIO_InitStruct );

    /* UART RX GPIO pin configuration  */
    GPIO_InitStruct.Pin  = WiFi_USART_PRINT_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
#if defined( USE_STM32L0XX_NUCLEO ) || defined( USE_STM32F4XX_NUCLEO ) ||                \
    defined( USE_STM32L4XX_NUCLEO )
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Alternate = PRINTMSG_USARTx_RX_AF;
#endif

    HAL_GPIO_Init( WiFi_USART_PRINT_RX_GPIO_PORT, &GPIO_InitStruct );

#ifdef WIFI_USE_VCOM
    /*##-3- Configure the NVIC for UART ########################################*/
    /* NVIC for USART */
    HAL_NVIC_SetPriority( USARTx_PRINT_IRQn, 0, 1 );
    HAL_NVIC_EnableIRQ( USARTx_PRINT_IRQn );
#endif
}
#endif // end of USART_PRINT_MSG


/******** Wi-Fi Indication User Callback *********/

void ind_wifi_socket_data_received( int8_t server_id,
                                    int8_t socket_id,
                                    uint8_t* data_ptr,
                                    uint32_t message_size,
                                    uint32_t chunk_size,
                                    WiFi_Socket_t socket_type )
{
#if 1
    ( void )server_id;   /* Unused */
    ( void )socket_type; /* Unused */

    /* Xively */
    xi_bsp_io_net_socket_data_received_proxy( ( uint8_t )socket_id, data_ptr,
                                              message_size, chunk_size );
#else
    printf( "\r\nData Receive Callback...\r\n" );
    memcpy( echo, data_ptr, 50 );
    printf( ( const char* )echo );
    printf( "\r\nsocket ID: %d\r\n", socket_id );
    printf( "msg size: %lu\r\n", ( unsigned long )message_size );
    printf( "chunk size: %lu\r\n", ( unsigned long )chunk_size );
    fflush( stdout );
    // wifi_state = wifi_state_socket_write;
#endif
}

void ind_wifi_socket_client_remote_server_closed( uint8_t* socket_closed_id,
                                                  WiFi_Socket_t socket_type )
{
#if 1
	( void )socket_type; /* Unused */

    /* Xively */
    xi_bsp_io_net_socket_client_remote_server_closed_proxy( socket_closed_id );
#else
    uint8_t id = *socketID;
    printf( "\r\n>>User Callback>>remote server socket closed\r\n" );
    printf( "Socket ID closed: %d",
            id ); // this will actually print the character/string, not the number
    fflush( stdout );
#endif
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

void ind_wifi_inputssi_callback( void )
{
    wifi_state = wifi_state_input_buffer;
}

void ind_wifi_error( WiFi_Status_t error_code )
{
    if ( error_code == WiFi_AT_CMD_RESP_ERROR )
    {
        wifi_state = wifi_state_idle;
        printf( "\r\n WiFi Command Failed. \r\n User should now press the RESET "
                "Button(B2). \r\n" );
    }
}
/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
