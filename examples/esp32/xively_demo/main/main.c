/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client codebase,
 * it is licensed under the BSD 3-Clause license.
 */
#include <string.h>
#include <errno.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event_loop.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_vfs_fat.h"

#include "xively_if.h"
#include "gpio_if.h"
#include "provisioning.h"
#include "user_data.h"

#ifndef USE_HARDCODED_CREDENTIALS
/* Default workflow uses runtime provisioning and flash storage for user data */
#define USE_HARDCODED_CREDENTIALS 0
#endif /* USE_HARDCODED_CREDENTIALS */

#if USE_HARDCODED_CREDENTIALS
#define USER_CONFIG_WIFI_SSID     "[SET YOUR WIFI NETWORK NAME HERE]"
#define USER_CONFIG_WIFI_PWD      "[SET YOUR WIFI NETWORK PASSWORD HERE]"
#define USER_CONFIG_XI_ACCOUNT_ID "[SET YOUR XIVELY ACCOUNT ID HERE]"
#define USER_CONFIG_XI_DEVICE_ID  "[SET YOUR XIVELY DEVICE  ID HERE]"
#define USER_CONFIG_XI_DEVICE_PWD "[SET YOUR XIVELY DEVICE PASSWORD HERE]"
#endif /* USE_HARDCODED_CREDENTIALS */

#define XIF_TASK_ESP_CORE    0 /* ESP32 core the XIF task will be pinned to */
#define XIF_TASK_STACK_SIZE  36 * 1024
#define GPIO_TASK_STACK_SIZE 2 * 1024
#define WIFI_CONNECTED_FLAG  BIT0

#define IO_BUTTON_WAIT_TIME_MS 500

static EventGroupHandle_t app_wifi_event_group;
static user_data_t user_config;

static esp_err_t app_wifi_event_handler( void* ctx, system_event_t* event )
{
    //printf( "\nNew WiFi event: ID [%d]", event->event_id );
    switch ( event->event_id )
    {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_CONNECTED:
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits( app_wifi_event_group, WIFI_CONNECTED_FLAG );
#if 0
            xif_request_action( XIF_REQUEST_CONTINUE );
#endif
            break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
            //printf( "\n\tHandling sudden WiFi disconnection..." );
            /* This is a workaround as ESP32 WiFi libs don't currently
               auto-reassociate. */
            /* JC TODO: something here crashes the application when the AP is turned off!! */
#if 0
            if( xif_request_action( XIF_REQUEST_PAUSE ) < 0 )
            {
                printf( "\n\tError pausing Xively Interface task" );
            }
#endif
            esp_wifi_connect();
            xEventGroupClearBits( app_wifi_event_group, WIFI_CONNECTED_FLAG );
            break;
        default:
            //printf( "\n\tWiFi event ignored at the application layer" );
            break;
    }
    return ESP_OK;
}

static int8_t app_wifi_station_init( void )
{
    printf( "\n|********************************************************|" );
    printf( "\n|               Initializing WiFi as Station             |" );
    printf( "\n|********************************************************|\n" );

#if USE_HARDCODED_CREDENTIALS
    wifi_config_t wifi_config =
    {
        .sta =
        {
            .ssid = USER_CONFIG_WIFI_SSID, .password = USER_CONFIG_WIFI_PWD
        }
    };
#else
    wifi_config_t wifi_config =
    {
        .sta =
        {
            .ssid = "", .password = ""
        }
    };

    strncpy( ( char* )wifi_config.sta.ssid, user_config.wifi_client_ssid,
             ESP_WIFI_SSID_STR_SIZE );
    strncpy( ( char* )wifi_config.sta.password, user_config.wifi_client_password,
             ESP_WIFI_PASSWORD_STR_SIZE );
#endif

    /* Initialize the TCP/IP stack, app_wifi_event_group and WiFi interface */
    app_wifi_event_group = xEventGroupCreate();
    if( NULL == app_wifi_event_group )
    {
        printf( "\nFailed to allocate FreeRTOS heap for wifi event group" );
        return -1;
    }
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init( app_wifi_event_handler, NULL ) );
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init( &wifi_init_config ) );

    ESP_ERROR_CHECK( esp_wifi_set_storage( WIFI_STORAGE_RAM ) );

    ESP_ERROR_CHECK( esp_wifi_set_mode( WIFI_MODE_STA ) );
    ESP_ERROR_CHECK( esp_wifi_set_config( ESP_IF_WIFI_STA, &wifi_config ) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    return 0;
}

int8_t app_fetch_user_config( void )
{
    int provisioning_bootmode_selected = 0;
    printf( "\n|********************************************************|" );
    printf( "\n|               Fetching User Configuration              |" );
    printf( "\n|********************************************************|" );

#if USE_HARDCODED_CREDENTIALS
    user_data_set_wifi_ssid( ( &user_config ), USER_CONFIG_WIFI_SSID );
    user_data_set_wifi_password( ( &user_config ), USER_CONFIG_WIFI_PWD );
    user_data_set_xi_account_id( ( &user_config ), USER_CONFIG_XI_ACCOUNT_ID );
    user_data_set_xi_device_id( ( &user_config ), USER_CONFIG_XI_DEVICE_ID );
    user_data_set_xi_device_password( ( &user_config ), USER_CONFIG_XI_DEVICE_PWD );
    user_data_printf( &user_config );
    return 0;
#endif

    /* Read user data from flash */
    if ( 0 > user_data_copy_from_flash( &user_config ) )
    {
        printf( "\n[ERROR] trying to copy user data from flash. Abort" );
        return -1;
    }
    printf( "\nUser data retrieved from flash:" );
    user_data_printf( &user_config );

    /* Wait for a button press for 80*50 ms while flashing the LED. If the button is
     * pressed, clear the contents of user_config to force the provisioning process */
    for ( int i = 80; i > 0; i-- )
    {
        if ( -1 != io_await_gpio_interrupt( 50 ) )
        {
            printf( "\nButton pressed - Initializing provisioning process" );
            provisioning_bootmode_selected = 1;
            break;
        }
        io_led_set( i % 2 ); /* Toggle LED */
    }
    io_led_off();

    /* If the data retrieved from NVS is missing any fields, start provisioning */
    if ( provisioning_bootmode_selected || ( 0 > user_data_is_valid( &user_config ) ) )
    {
        io_led_on();
        if ( 0 > provisioning_gather_user_data( &user_config ) )
        {
            printf( "\nDevice provisioning [ERROR]. Abort" );
            io_led_off();
            return -1;
        }
    }
    io_led_off();
    return 0;
}

void app_gpio_interrupts_handler_task( void* param )
{
    int button_input_level = -1;
    while ( 1 )
    {
        if ( -1 !=
             ( button_input_level = io_await_gpio_interrupt( IO_BUTTON_WAIT_TIME_MS ) ) )
        {
            printf( "\nButton press detected. Pin level [%d]", button_input_level );
            xif_publish_button_state( button_input_level );
        }
        taskYIELD();
    }
}

void xif_recv_mqtt_msg_callback( const xi_sub_call_params_t* const params )
{
    printf( "\nNew MQTT message received!" );
    printf( "\n\tTopic: %s", params->message.topic );
    printf( "\n\tMessage size: %d", params->message.temporary_payload_data_length );
    printf( "\n\tMessage payload: " );
    for ( size_t i = 0; i < params->message.temporary_payload_data_length; i++ )
    {
        // printf( "0x%02x ", params->message.temporary_payload_data[i] );
        printf( "%c", ( char )params->message.temporary_payload_data[i] );
    }
    if( 0 == strcmp( params->message.topic, xif_mqtt_topics.led_topic ) )
    {
        if ( 0 == params->message.temporary_payload_data_length )
            io_led_off();
        else if ( '0' == params->message.temporary_payload_data[0] )
            io_led_off();
        else
            io_led_on();
    }
}

void app_main( void )
{
    /* Initialize Non-Volatile Storage */
    if ( 0 > io_init() )
    {
        while ( 1 )
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }

    /* Initialize Non-Volatile Storage */
    if ( 0 > user_data_flash_init() )
    {
        while ( 1 )
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }

    /* Fetch user credentials - Go through Provisioning if necessary */
    if ( 0 > app_fetch_user_config() )
    {
        while ( 1 )
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }

    /* Initialize the ESP32 as a WiFi station */
    if ( 0 > app_wifi_station_init() )
    {
        while ( 1 )
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }

    /* Configure Xively interface */
    if ( 0 > xif_set_device_info( user_config.xi_account_id, user_config.xi_device_id,
                                  user_config.xi_device_password ) )
    {
        while ( 1 )
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }

    /* Start GPIO interrupt handler task */
    if ( pdPASS != xTaskCreate( &app_gpio_interrupts_handler_task, "gpio_intr_task",
                                GPIO_TASK_STACK_SIZE, NULL, 5, NULL ) )
    {
        printf( "\n[ERROR] creating GPIO interrupt handler RTOS task" );
        printf( "\n\tInterrupts will be ignored" );
    }

    /* Wait until we're connected to the WiFi network */
    xEventGroupWaitBits( app_wifi_event_group, WIFI_CONNECTED_FLAG, false, true,
            portMAX_DELAY );

    /* Start Xively task */
    if ( pdPASS != xTaskCreatePinnedToCore( &xif_rtos_task, "xif_task",
                                            XIF_TASK_STACK_SIZE, NULL, 5, NULL,
                                            XIF_TASK_ESP_CORE ) )
    {
        printf( "\n[ERROR] creating Xively Interface RTOS task" );
        while ( 1 )
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }

    /* Loop forever, read the interrupts queue and publish on button change */
    //while ( 1 )
    //{
    //    if ( xQueueReceive( io_button_queue, &io_num, portMAX_DELAY ) )
    //    {
    //        printf( "\nInterrupt at GPIO [%d] - Pin value [%d]", io_num,
    //                gpio_get_level( io_num ) );
    //    }
    //    vTaskDelay( 10 / portTICK_PERIOD_MS );
    //}
}
