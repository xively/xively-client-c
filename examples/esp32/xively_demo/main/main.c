/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
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

#include "xively_task.h"
#include "gpio_if.h"
#include "provisioning.h"
#include "user_data.h"

/* Default workflow uses runtime provisioning and NVS storage for user data */
#define USE_HARDCODED_CREDENTIALS 0
#if USE_HARDCODED_CREDENTIALS
  #define USER_CONFIG_WIFI_SSID     "[SET YOUR WIFI NETWORK NAME HERE]"
  #define USER_CONFIG_WIFI_PWD      "[SET YOUR WIFI NETWORK PASSWORD HERE]"
  #define USER_CONFIG_XI_ACCOUNT_ID "[SET YOUR XIVELY ACCOUNT ID HERE]"
  #define USER_CONFIG_XI_DEVICE_ID  "[SET YOUR XIVELY DEVICE  ID HERE]"
  #define USER_CONFIG_XI_DEVICE_PWD "[SET YOUR XIVELY DEVICE PASSWORD HERE]"
#endif

#define XT_TASK_ESP_CORE    0 /* ESP32 core the XT task will be pinned to */
#define XT_TASK_STACK_SIZE  36 * 1024
#define GPIO_TASK_STACK_SIZE 2 * 1024

#define WIFI_CONNECTED_FLAG  BIT0

static EventGroupHandle_t app_wifi_event_group;
static user_data_t user_config;

static esp_err_t app_wifi_event_handler( void* ctx, system_event_t* event );
static int8_t app_wifi_station_init( user_data_t* credentials );
static int8_t app_fetch_user_config( user_data_t* dst );
static void app_gpio_interrupts_handler_task( void* param );

/**
 * Initialize GPIO and NVS, fetch WiFi and Xively credentials from flash or
 * through the provisioning process, start RTOS tasks for Xively, GPIO handling
 * and WiFi stack handling
 */
void app_main( void )
{
    /* Initialize GPIO and Button Interrupts */
    if ( 0 > io_init() )
    {
        printf( "\n[ERROR] Initializing GPIO interface. Boot halted" );
        while ( 1 )
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }

    /* Initialize Non-Volatile Storage */
    if ( 0 > user_data_flash_init() )
    {
        printf( "\n[ERROR] initializing user data storage interface. Boot halted" );
        while ( 1 )
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }

    /* Fetch user credentials - Go through Provisioning if necessary */
    if ( 0 > app_fetch_user_config( &user_config ) )
    {
        printf( "\n[ERROR] fetching user configuration. Boot halted" );
        while ( 1 )
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }

    /* Configure Xively Settings */
    if ( 0 > xt_init( user_config.xi_account_id, user_config.xi_device_id,
                      user_config.xi_device_password ) )
    {
        printf( "\n[ERROR] configuring Xively Task. Boot halted" );
        while ( 1 )
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }

    /* Start Xively task */
    if ( pdPASS != xTaskCreatePinnedToCore( &xt_rtos_task, "xively_task",
                                            XT_TASK_STACK_SIZE, NULL, 5, NULL,
                                            XT_TASK_ESP_CORE ) )
    {
        printf( "\n[ERROR] creating Xively Task" );
        while ( 1 )
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
    } /* Will connect from the wifi callback */

    /* Wait until the Xively Client context is created, so we can connect on STA_GOT_IP */
    while( 0 == xt_ready_for_requests() )
    {
        vTaskDelay( 100 / portTICK_PERIOD_MS );
    }

    /* Initialize the ESP32 as a WiFi station */
    if ( 0 > app_wifi_station_init( &user_config ) )
    {
        printf( "\n[ERROR] initializing WiFi station mode. Boot halted" );
        while ( 1 )
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }

    /* Wait until we're connected to the WiFi network */
    xEventGroupWaitBits( app_wifi_event_group, WIFI_CONNECTED_FLAG, false, true,
            portMAX_DELAY );

    /* Start GPIO interrupt handler task */
    if ( pdPASS != xTaskCreate( &app_gpio_interrupts_handler_task, "gpio_intr_task",
                                GPIO_TASK_STACK_SIZE, NULL, 3, NULL ) )
    {
        printf( "\n[ERROR] creating GPIO interrupt handler RTOS task" );
        printf( "\n\tInterrupts will be ignored" );
    }
}

/******************************************************************************
 *                              WiFi API & Callbacks
 ******************************************************************************/
esp_err_t app_wifi_event_handler( void* ctx, system_event_t* event )
{
    switch ( event->event_id )
    {
        case SYSTEM_EVENT_STA_START:
            ESP_ERROR_CHECK( esp_wifi_connect() );
            break;
        case SYSTEM_EVENT_STA_CONNECTED:
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits( app_wifi_event_group, WIFI_CONNECTED_FLAG );
            if( xt_request_machine_state( XT_REQUEST_CONNECT ) < 0 )
            {
                printf( "\n\tError requesting MQTT connect from xively task" );
                xt_handle_unrecoverable_error();
            }
            break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
            if ( xt_ready_for_requests() && xt_is_connected() )
            {
                if ( xt_request_machine_state( XT_REQUEST_DISCONNECT ) < 0 )
                {
                    printf( "\n\tError requesting MQTT disconnect from xively task" );
                    xt_handle_unrecoverable_error();
                }
            }
            ESP_ERROR_CHECK( esp_wifi_connect() );
            xEventGroupClearBits( app_wifi_event_group, WIFI_CONNECTED_FLAG );
            break;
        default:
            printf( "\n\tWiFi event [%d] callback ignored", event->event_id );
            break;
    }
    return ESP_OK;
}

int8_t app_wifi_station_init( user_data_t* credentials )
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
    wifi_config_t wifi_config = {.sta = {.ssid = "", .password = ""}};

    strncpy( ( char* )wifi_config.sta.ssid, credentials->wifi_client_ssid,
             ESP_WIFI_SSID_STR_SIZE );
    strncpy( ( char* )wifi_config.sta.password, credentials->wifi_client_password,
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

/******************************************************************************
 *                   Kickstart Provisioning if Necessary
 ******************************************************************************/
int8_t app_fetch_user_config( user_data_t* dst )
{
    int provisioning_bootmode_selected = 0;
    printf( "\n|********************************************************|" );
    printf( "\n|               Fetching User Configuration              |" );
    printf( "\n|********************************************************|" );

#if USE_HARDCODED_CREDENTIALS
    user_data_set_wifi_ssid( ( dst ), USER_CONFIG_WIFI_SSID );
    user_data_set_wifi_password( ( dst ), USER_CONFIG_WIFI_PWD );
    user_data_set_xi_account_id( ( dst ), USER_CONFIG_XI_ACCOUNT_ID );
    user_data_set_xi_device_id( ( dst ), USER_CONFIG_XI_DEVICE_ID );
    user_data_set_xi_device_password( ( dst ), USER_CONFIG_XI_DEVICE_PWD );
    user_data_printf( dst );
    return 0;
#endif

    /* Read user data from flash */
    if ( 0 > user_data_copy_from_flash( dst ) )
    {
        printf( "\n[ERROR] trying to copy user data from flash. Abort" );
        return -1;
    }
    printf( "\nUser data retrieved from flash:" );
    user_data_printf( dst );

    /* Wait for a button press for 80*50 ms while flashing the LED. If the button is
     * pressed, clear the contents of user_config to force the provisioning process */
    for ( int i = 40; i > 0; i-- )
    {
        if ( -1 != io_await_gpio_interrupt( 100 ) )
        {
            printf( "\nButton pressed - Initializing provisioning process" );
            provisioning_bootmode_selected = 1;
            break;
        }
        io_led_set( i % 2 ); /* Toggle LED */
    }
    io_led_off();

    /* If the data retrieved from NVS is missing any fields, start provisioning */
    if ( provisioning_bootmode_selected || ( 0 > user_data_is_valid( dst ) ) )
    {
        io_led_on();
        if ( 0 > provisioning_gather_user_data( dst ) )
        {
            printf( "\nDevice provisioning [ERROR]. Abort" );
            io_led_off();
            return -1;
        }
    }
    io_led_off();
    return 0;
}

/******************************************************************************
 *                   GPIO Interrupts Handling Task
 ******************************************************************************/
void app_gpio_interrupts_handler_task( void* param )
{
    const uint32_t IO_BUTTON_WAIT_TIME_MS = 500;
    int virtual_switch = 0; /* Switches between 1 and 0 on each button press */
    while ( 1 )
    {
        if ( -1 != io_await_gpio_interrupt( IO_BUTTON_WAIT_TIME_MS ) )
        {
            virtual_switch = !virtual_switch;
            printf( "\nButton pressed! Virtual switch [%d]", virtual_switch );
            xt_publish_button_state( virtual_switch );
        }
        taskYIELD();
    }
}

/******************************************************************************
 *                          MQTT Message Received Callback
 ******************************************************************************/
void xt_recv_mqtt_msg_callback( const xi_sub_call_params_t* const params )
{
    printf( "\nNew MQTT message received!" );
    printf( "\n\tTopic: %s", params->message.topic );
    printf( "\n\tMessage size: %d", params->message.temporary_payload_data_length );
    printf( "\n\tMessage payload: " );
    for ( size_t i = 0; i < params->message.temporary_payload_data_length; i++ )
    {
        printf( "%c", ( char )params->message.temporary_payload_data[i] );
    }
    if( 0 == strcmp( params->message.topic, xt_mqtt_topics.led_topic ) )
    {
        if ( 0 == params->message.temporary_payload_data_length )
            io_led_off();
        else if ( '0' == params->message.temporary_payload_data[0] )
            io_led_off();
        else
            io_led_on();
    }
}
