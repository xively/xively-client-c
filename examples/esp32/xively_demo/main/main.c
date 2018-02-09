/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
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
#include "esp_ota_ops.h"

#include "xi_bsp_fwu_notifications_esp32.h"

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

#define XT_TASK_ESP_CORE 0 /* ESP32 core the XT task will be pinned to */
#define XT_TASK_STACK_SIZE ( 1024 * 36 )
#define APP_MAIN_LOGIC_STACK_SIZE ( 1024 * 2 )

#define XIVELY_TASK_PRIORITY 6
#define APP_TASK_PRIORITY 1

#define WIFI_CONNECTED_FLAG BIT0

/**
 * The flash filesystem partition is only required for the Xively Client's Secure
 * File Transfer logic in this demo. That implementation is in xi_bsp_io_fs_esp32.c.
 * It is initialized and made accessible from this file, in case you'd like to
 * store your own project's files in the same FS partition.
 */
#define ESP32_FS_BASE_PATH "/spiflash" /* [!] Keep in sync with xi_bsp_io_fs_esp32.c */
#define ESP32_FATFS_PARTITION "storage"
#define ESP32_FATFS_MAX_OPEN_FILES 5

static EventGroupHandle_t app_wifi_event_group;
static user_data_t user_config;

static size_t ota_download_file_size = 0;
static int ota_download_progress = 0;

static esp_err_t app_wifi_event_handler( void* ctx, system_event_t* event );
static int8_t app_init_filesystem( void );
static int8_t app_wifi_station_init( user_data_t* credentials );
static int8_t app_fetch_user_config( user_data_t* dst );
static void app_main_logic_task( void* param );

static void app_set_xi_fwu_progress_notification_callbacks( void );
static void esp32_xibsp_notify_update_started( const char* filename, size_t file_size );
static void esp32_xibsp_notify_chunk_written( size_t chunk_size, size_t offset );
static void esp32_xibsp_notify_update_applied( uint8_t app_updated );

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

    /* Initialize flash filesystem - Used by xi_bsp_io_fs_esp32.c */
    if ( 0 > app_init_filesystem() )
    {
        printf( "\n[ERROR] Initializing flash filesystem. Boot halted" );
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

    app_set_xi_fwu_progress_notification_callbacks();

    /* Configure Xively Settings */
    if ( 0 > xt_init( user_config.xi_account_id, user_config.xi_device_id,
                      user_config.xi_device_password ) )
    {
        printf( "\n[ERROR] configuring Xively Task. Boot halted" );
        while ( 1 )
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }

    /* Start Xively task */
    if ( pdPASS !=
         xTaskCreatePinnedToCore( &xt_rtos_task, "xively_task", XT_TASK_STACK_SIZE, NULL,
                                  XIVELY_TASK_PRIORITY, NULL, XT_TASK_ESP_CORE ) )
    {
        printf( "\n[ERROR] creating Xively Task" );
        while ( 1 )
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }

    /* Start a new task for the post-initialization logic. In this demo, it
    simply rints a string over and over again */
    if ( pdPASS != xTaskCreate( &app_main_logic_task, "app_main_logic",
                                APP_MAIN_LOGIC_STACK_SIZE, NULL, APP_TASK_PRIORITY,
                                NULL ) )
    {
        printf( "\n[ERROR] creating GPIO interrupt handler RTOS task" );
        printf( "\n\tInterrupts will be ignored" );
    }
}

/******************************************************************************
 *                   Filesystem Initialization and Mounting
 ******************************************************************************/
int8_t app_init_filesystem( void )
{
    /* flash_wl_handle isn't global because this demo doesn't unmount the FS anyway */
    wl_handle_t flash_wl_handle = WL_INVALID_HANDLE;
    const esp_vfs_fat_mount_config_t mount_config = {
        .max_files = ESP32_FATFS_MAX_OPEN_FILES, .format_if_mount_failed = true};

    /* This function is a convenience function provided by ESP-IDF to initialize
       the flash partition, mount it, erase it and re-mount if necessary, etc.
       You probably want a more robust implementation for a production environment */
    const esp_err_t retval = esp_vfs_fat_spiflash_mount(
        ESP32_FS_BASE_PATH, ESP32_FATFS_PARTITION, &mount_config, &flash_wl_handle );
    if ( retval != ESP_OK )
    {
        printf( "Failed to mount FATFS [0x%x]", retval );
        return -1;
    }
    return 0;
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
            /* XT will be ready for requests AFTER the first GOT_IP event */
            /* The first connection will be requested from xt_rtos_task */
            if ( xt_ready_for_requests() )
            {
                if( xt_request_machine_state( XT_REQUEST_CONNECT ) < 0 )
                {
                    xt_handle_unrecoverable_error();
                }
            }
            break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
            if ( xt_ready_for_requests() && xt_is_connected() )
            {
                if ( xt_request_machine_state( XT_REQUEST_DISCONNECT ) < 0 )
                {
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

    wifi_config_t wifi_config = {.sta = {.ssid = "", .password = ""}};

    strncpy( ( char* )wifi_config.sta.ssid, credentials->wifi_client_ssid,
             ESP_WIFI_SSID_STR_SIZE );
    strncpy( ( char* )wifi_config.sta.password, credentials->wifi_client_password,
             ESP_WIFI_PASSWORD_STR_SIZE );

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

    /* Wait for a button press for a few seconds while flashing the LED. If the button is
     * pressed, clear the contents of user_config to force the provisioning process */
    for ( int blinks_num = 20; ( ( blinks_num * 2 ) - 1 ) >= 0; blinks_num-- )
    {
        if ( -1 != io_pop_gpio_interrupt() )
        {
            printf( "\nButton pressed - Initializing provisioning process" );
            provisioning_bootmode_selected = 1;
            break;
        }
        io_led_set( blinks_num % 2 ); /* Toggle LED */
        vTaskDelay( 50 / portTICK_PERIOD_MS );
    }

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
 *                   Post-Initialization main task
 ******************************************************************************/
/* Simple Proof of Concept, simple logic. This task may be used for more resource
 * expensive operations. Remember to communicate with the Xively Client in a Task-safe
 * manner, and adjust this task's stack size (APP_MAIN_LOGIC_STACK_SIZE) to fit your needs
 */
void app_main_logic_task( void* param )
{
    unsigned int led_toggler = 0;
    while ( 1 )
    {
        printf( "\n((Factory Default Running))" );
        // printf( "\n[[New FW Running]]" );

        if ( !xt_ready_for_requests() )
        {
            /* The Xively Task was shut down for some reason. Reboot the device */
            /* This demo doesn't shut down the xively task during normal operation,
               so this serves a simple application-level watchdog */
            /* You may need to remove/rewrite it for your own logic. */
            /* You can start the xively task from here, to re-start the task instead
               of rebooting the device. Rebooting is more drastic, but it solves other
               potential malfunctions caused by whatever crashed the xively task */
            printf( "\nXively task stopped running. Rebooting" );
            esp_restart();
        }
        else if ( xt_is_connected() )
        {
            /* LED OFF by default when the device is connected, controllable over MQTT */
            if ( led_toggler == 1 )
            {
                led_toggler = 0;
                io_led_set( led_toggler );
            }
        }
        else
        {
            /* LED blinks slowly while the device is disconnected */
            ( led_toggler == 1 ) ? ( led_toggler = 0 ) : ( led_toggler = 1 );
            io_led_set( led_toggler % 2 );
        }
        vTaskDelay( 5000 / portTICK_PERIOD_MS );
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

/******************************************************************************
 *       OTA Firmware Update notifications from the Xively Client's BSP
 *
 * Note:
 * This is a custom extension of the Xively Client's BSP, used to report the
 * status of ongoing firmware downloads to the application layer.
 *
 * Relevant files:
 *    - xi_bsp_fwu_notifications_esp32.c
 *    - xi_bsp_fwu_notifications_esp32.h
 ******************************************************************************/
static void app_set_xi_fwu_progress_notification_callbacks( void )
{
    xi_bsp_fwu_notification_callbacks.update_started = esp32_xibsp_notify_update_started;
    xi_bsp_fwu_notification_callbacks.chunk_written  = esp32_xibsp_notify_chunk_written;
    xi_bsp_fwu_notification_callbacks.update_applied = esp32_xibsp_notify_update_applied;
}

void esp32_xibsp_notify_update_started( const char* filename, size_t file_size )
{
    printf( "\nDownload of file [%s] started. Size: [%d]", filename, file_size );
    ota_download_file_size = file_size;
}

void esp32_xibsp_notify_chunk_written( size_t chunk_size, size_t offset )
{
    if ( ota_download_file_size <= 0 )
    {
        return;
    }
    ota_download_progress = ( ( chunk_size + offset ) * 100 ) / ota_download_file_size;
    printf( "\n[%d%%] Firmware chunk of size [%d] written at offset [%d]",
            ota_download_progress, chunk_size, offset );

    { /* FW update light show */
        for ( int blinks_num = 2; ( ( blinks_num * 2 ) - 1 ) >= 0; blinks_num-- )
        {
            io_led_set( blinks_num % 2 );
            vTaskDelay( 50 / portTICK_PERIOD_MS );
        }
    }
}

void esp32_xibsp_notify_update_applied( uint8_t app_updated )
{
    if ( 1 == app_updated )
    {
        /* Firmware image was updated */
        printf( "SFT package download finished. FW updated; rebooting" );
        const esp_partition_t* next_partition = esp_ota_get_next_update_partition( NULL );
        esp_err_t retv                        = ESP_OK;

        retv = esp_ota_set_boot_partition( next_partition );
        if ( ESP_OK != retv )
        {
            printf( "esp_ota_set_boot_partition() failed with error %d", retv );
            return;
        }

        /* reboot the device */
        esp_restart();
    }
    /* Firmware image was not updated */
    printf( "SFT package download finished. No need to reboot" );
    return;
}
