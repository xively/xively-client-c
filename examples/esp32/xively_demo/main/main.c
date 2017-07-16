/* tcp_perf Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


/*
tcp_perf example

Using this example to test tcp throughput performance.
esp<->esp or esp<->ap

step1:
    init wifi as AP/STA using config SSID/PASSWORD.

step2:
    create a tcp server/client socket using config PORT/(IP).
    if server: wating for connect.
    if client connect to server.
step3:
    send/receive data to/from each other.
    if the tcp connect established. esp will send or receive data.
    you can see the info in serial output.
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

#define APP_XI_ACCOUNT_ID "22ebd0b6-cefe-44d3-93b5-49c14f8a2c40"
#define APP_XI_DEVICE_ID  "f710088e-ceaa-453b-8b64-1cdced09ce67"
#define APP_XI_DEVICE_PWD "thHlyjCa2f3vff8TQBq4QUKxxgE8xJJslAcxSbM5BPM="
#define APP_WIFI_SSID "palantir"
#define APP_WIFI_PASS "palantir555"

#define XIF_STACK_SIZE 36*1024
#define CONNECTED_BIT BIT0

static EventGroupHandle_t app_wifi_event_group;

static esp_err_t app_wifi_event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(app_wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
        esp_wifi_connect();
        xEventGroupClearBits(app_wifi_event_group, CONNECTED_BIT);
        break;
    default:
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

void app_main( void )
{
    app_wifi_init();

    /* Wait for WiFI to show as connected */
    xEventGroupWaitBits( app_wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY );

    xif_set_device_info( APP_XI_ACCOUNT_ID, APP_XI_DEVICE_ID, APP_XI_DEVICE_PWD );

    xTaskCreatePinnedToCore( &xif_rtos_task, "xif_task", XIF_STACK_SIZE, NULL, 5, NULL, 1 );
}
