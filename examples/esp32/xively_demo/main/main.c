/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client codebase,
 * it is licensed under the BSD 3-Clause license.
 */

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

#define APP_XI_ACCOUNT_ID "[SET YOUR XIVELY ACCOUNT ID HERE]"
#define APP_XI_DEVICE_ID  "[SET YOUR XIVELY DEVICE  ID HERE]"
#define APP_XI_DEVICE_PWD "[SET YOUR XIVELY DEVICE PASSWORD HERE]"
#define APP_WIFI_SSID "[SET YOUR WIFI NETWORK NAME HERE]"
#define APP_WIFI_PASS "[SET YOUR WIFI NETWORK PASSWORD HERE]"

//#define XIF_STACK_SIZE 36*1024
#define XIF_STACK_SIZE 48*1024
#define WIFI_CONNECTED_FLAG BIT0

static EventGroupHandle_t app_wifi_event_group;

static esp_err_t app_wifi_event_handler( void* ctx, system_event_t* event )
{
    printf( "\nNew WiFi event: ID [%d]", event->event_id );
    switch ( event->event_id )
    {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits( app_wifi_event_group, WIFI_CONNECTED_FLAG );
#if 1
            xif_events_continue();
#endif
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            printf( "\n\tHandling sudden WiFi disconnection..." );
            /* This is a workaround as ESP32 WiFi libs don't currently
               auto-reassociate. */
            /* JC TODO: This crashes the application when the AP is turned off!! */
            esp_wifi_connect();
            xEventGroupClearBits( app_wifi_event_group, WIFI_CONNECTED_FLAG );
#if 1
            if( xif_events_pause() < 0 )
            {
                printf( "\n\tError pausing Xively Interface task" );
            }
#endif
            break;
        default:
            printf( "\n\tWiFi event ignored at the application layer" );
            break;
    }
    return ESP_OK;
}

static void app_wifi_init( void )
{
    tcpip_adapter_init();
    app_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init( app_wifi_event_handler, NULL ) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init( &cfg ) );
    ESP_ERROR_CHECK( esp_wifi_set_storage( WIFI_STORAGE_RAM ) );
    wifi_config_t wifi_config = {
        .sta =
            {
                .ssid = APP_WIFI_SSID, .password = APP_WIFI_PASS,
            },
    };
    printf( "\nSetting WiFi configuration SSID %s...", wifi_config.sta.ssid );
    ESP_ERROR_CHECK( esp_wifi_set_mode( WIFI_MODE_STA ) );
    ESP_ERROR_CHECK( esp_wifi_set_config( WIFI_IF_STA, &wifi_config ) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

void TEST_task( void* param )
{
    unsigned int i = 0;
    while ( 1 )
    {
        printf( "\nTest task loopin' [%d]", ++i );
        vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }
}

void app_main( void )
{
    app_wifi_init();

    /* Wait for WiFI to show as connected */
    xEventGroupWaitBits( app_wifi_event_group, WIFI_CONNECTED_FLAG, false, true,
                         portMAX_DELAY );

    if ( xif_set_device_info( APP_XI_ACCOUNT_ID, APP_XI_DEVICE_ID, APP_XI_DEVICE_PWD )
         < 0 )
    {
        while ( 1 )
            ;
    }

    xTaskCreatePinnedToCore( &xif_rtos_task, "xif_task", XIF_STACK_SIZE, NULL, 5,
                             NULL, 1 );
    //xTaskCreatePinnedToCore( &xif_rtos_task, "xif_task", XIF_STACK_SIZE, NULL, 5,
    //                         NULL, 1 );

    unsigned int i = 0;
    while ( 1 )
    {
        printf( "\napp_main loopin' [%d]", ++i );
        vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }
}
