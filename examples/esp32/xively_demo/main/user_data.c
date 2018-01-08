/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client codebase,
 * it is licensed under the BSD 3-Clause license.
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#include "esp_system.h"
#include "esp_partition.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "user_data.h"
 
/* These are the keys and namespace for the ESP32's NVS key:value pairs.
 * All key names must fit in a 16character string, including '\0' */
#define UD_NVS_NAMESPACE            "storage"
#define UD_NVS_KEY_WIFI_CLIENT_SSID "wifi_cli_ssid"
#define UD_NVS_KEY_WIFI_CLIENT_PWD  "wifi_cli_pwd"
#define UD_NVS_KEY_XI_ACCOUNT_ID    "xi_account_id"
#define UD_NVS_KEY_XI_DEVICE_ID     "xi_device_id"
#define UD_NVS_KEY_XI_DEVICE_PWD    "xi_device_pwd"

static int8_t
user_data_read_esp32_nvs_str( nvs_handle nvs_hdl, char* nvs_key, char* dst );

int8_t user_data_flash_init( void )
{
    int retval = 0;

    retval = nvs_flash_init();
    if ( ESP_ERR_NVS_NO_FREE_PAGES == retval )
    {
        /* NVS partition was truncated and needs to be erased */
        const esp_partition_t* nvs_partition = esp_partition_find_first(
            ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL );

        if ( NULL == nvs_partition )
        {
            printf( "\n\t[ERROR] finding first partition. Flash init aborted" );
            return -1;
        }

        retval = esp_partition_erase_range( nvs_partition, 0, nvs_partition->size );
        if ( ESP_OK != retval )
        {
            printf( "\n\t[ERROR] erasing flash parition. Flash init aborted" );
            return -1;
        }
        /* Retry nvs_flash_init */
        retval = nvs_flash_init();
    }

    if ( ESP_OK != retval )
    {
        printf( "\n[ERROR] Initializing Non Volatile Storage: [%d]\n\t", retval );
        ESP_ERROR_CHECK( retval );
        return -1;
    }

    return 0;
}

/**
 * @brief  Read an NVS string entry from the ESP32's flash
 * @param  nvs_hdl must be set via nvs_open(...) before calling this function,
 *         and closed with nvs_close(...) after all entries have been read
 * @param  nvs_key identifies the key:value pair in NVS we want
 * @param  *dst is the pre-allocated pointer where we want to copy the string
 * @retval -2: NVS string can't fit into dst[USER_DATA_STR_SIZE]
 * @retval -1: Internal error
 *          0: String read OK
 *          1: String read OK but empty - an empty string is copied into *dst
 *          2: String had never been saved - an empty string is copied into *dst
 */
static int8_t user_data_read_esp32_nvs_str( nvs_handle nvs_hdl, char* nvs_key, char* dst )
{
    if ( ESP_ERR_NVS_INVALID_HANDLE == nvs_hdl )
        return -1;
    if ( NULL == nvs_key )
        return -1;
    if ( NULL == dst )
        return -1;

    size_t nvs_str_size = 0;
    esp_err_t retval;

    /* Read string size from Non-Volatile Storage */
    retval = nvs_get_str( nvs_hdl, nvs_key, NULL, &nvs_str_size );
    if ( ( ESP_OK != retval ) && ( ESP_ERR_NVS_NOT_FOUND != retval ) )
    {
        printf( "\n\t[ERROR] getting data size from NVS: [%d]\n\t", retval );
        ESP_ERROR_CHECK( retval );
        return -1;
    }

    if( nvs_str_size > USER_DATA_STR_SIZE )
    {
        printf( "\n\t[ERROR] The NVS string can't fit in the allocated buffer" );
        return -2;
    }
    else if ( 0 == nvs_str_size )
    {
        printf( "\n\tEntry [%s] was empty", nvs_key );
        strcpy( dst, "" );
        return 1;
    }
    else if ( ESP_ERR_NVS_NOT_FOUND == retval )
    {
        printf( "\n\tEntry [%s] has never been saved to NVS", nvs_key );
        strcpy( dst, "" );
        return 2;
    }

    /* Read string from Non-Volatile Storage */
    retval = nvs_get_str( nvs_hdl, nvs_key, dst, &nvs_str_size );
    if ( retval != ESP_OK )
    {
        printf( "\n\t[ERROR] getting data from NVS: [%d]\n\t", retval );
        ESP_ERROR_CHECK( retval );
        return -1;
    }

    return 0;
}

/**
 * @brief  Read the NVS memory into *dst so it can be manipulated
 * @param  *dst is the pre-allocated pointer where we want to copy the user data
 * @retval <0: Error, >=0: OK
 */
int8_t user_data_copy_from_flash( user_data_t* dst )
{
    if ( NULL == dst )
    {
        return -1;
    }

    nvs_handle handle;
    int retval = 0;

    /* Open NVS Interface */
    printf( "\nOpening NVS interface to read all user data" );
    retval = nvs_open( UD_NVS_NAMESPACE, NVS_READWRITE, &handle );
    if ( ESP_OK != retval )
    {
        printf( "\n\t[ERROR] Opening NVS interface: [%d]\n\t", retval );
        ESP_ERROR_CHECK( retval );
        return -1;
    }

    /* Read WiFi credentials */
    if ( 0 > user_data_read_esp32_nvs_str( handle, UD_NVS_KEY_WIFI_CLIENT_SSID,
                                           dst->wifi_client_ssid ) )
    {
        goto err_out;
    }
    if ( 0 > user_data_read_esp32_nvs_str( handle, UD_NVS_KEY_WIFI_CLIENT_PWD,
                                           dst->wifi_client_password ) )
    {
        goto err_out;
    }

    /* Read Xively credentials */
    if ( 0 > user_data_read_esp32_nvs_str( handle, UD_NVS_KEY_XI_ACCOUNT_ID,
                                           dst->xi_account_id ) )
    {
        goto err_out;
    }
    if ( 0 > user_data_read_esp32_nvs_str( handle, UD_NVS_KEY_XI_DEVICE_ID,
                                           dst->xi_device_id ) )
    {
        goto err_out;
    }
    if ( 0 > user_data_read_esp32_nvs_str( handle, UD_NVS_KEY_XI_DEVICE_PWD,
                                           dst->xi_device_password ) )
    {
        goto err_out;
    }

    /* Close NVS Interface */
    nvs_close(handle);
    return 0;

err_out:
    printf( "\n\t[ERROR] reading NVS string. User data retrieval from NVS aborted" );
    nvs_close(handle);
    return -1;
}

/**
 * @brief  Store all strings in *src into the ESP32's NVS
 * @param  *src is the datastructure we'd like to save to flash
 * @retval <0: Error, >=0: OK
 */
int8_t user_data_save_to_flash( user_data_t* src )
{
    if ( NULL == src )
    {
        return -1;
    }

    nvs_handle handle;
    int retval = 0;

    /* Open NVS Interface */
    printf( "\nOpening NVS interface to save all user data" );
    retval = nvs_open( UD_NVS_NAMESPACE, NVS_READWRITE, &handle );
    if ( ESP_OK != retval )
    {
        printf( "\n\t[ERROR] Opening NVS interface: [%d]\n\t", retval );
        ESP_ERROR_CHECK( retval );
        return -1;
    }

    /* Write WiFi credentials */
    if ( ESP_OK != ( retval = nvs_set_str( handle, UD_NVS_KEY_WIFI_CLIENT_SSID,
                                           src->wifi_client_ssid ) ) )
    {
        goto err_out;
    }
    if ( ESP_OK != ( retval = nvs_set_str( handle, UD_NVS_KEY_WIFI_CLIENT_PWD,
                                           src->wifi_client_password ) ) )
    {
        goto err_out;
    }

    /* Write Xively credentials */
    if ( ESP_OK != ( retval = nvs_set_str( handle, UD_NVS_KEY_XI_ACCOUNT_ID,
                                           src->xi_account_id ) ) )
    {
        goto err_out;
    }
    if ( ESP_OK != ( retval = nvs_set_str( handle, UD_NVS_KEY_XI_DEVICE_ID,
                     src->xi_device_id ) ) )
    {
        goto err_out;
    }
    if ( ESP_OK != ( retval = nvs_set_str( handle, UD_NVS_KEY_XI_DEVICE_PWD,
                                           src->xi_device_password ) ) )
    {
        goto err_out;
    }
    if ( ESP_OK != ( retval = nvs_commit( handle ) ) )
    {
        goto err_out;
    }

    /* Close NVS Interface */
    nvs_close( handle );
    return 0;

err_out:
    printf( "\n\t[ERROR] [0x%04x] writing NVS string. Data storage aborted", retval );
    nvs_close( handle );
    return -1;
}

/**
 * @brief  Deletes all previously stored entries from NVS
 * @retval <0: Error, >=0: OK
 */
int8_t user_data_reset_flash( void )
{
    nvs_handle handle;
    int retval = 0;

    /* Open NVS Interface */
    printf( "\nOpening NVS interface to erase all user data" );
    retval = nvs_open( UD_NVS_NAMESPACE, NVS_READWRITE, &handle );
    if ( ESP_OK != retval )
    {
        printf( "\n\t[ERROR] Opening NVS interface: [%d]\n\t", retval );
        ESP_ERROR_CHECK( retval );
        return -1;
    }

    retval = nvs_erase_all( handle );
    if ( ESP_OK != retval )
    {
        printf( "\n\t[ERROR] erasing flash data: [%d]\n\t", retval );
        ESP_ERROR_CHECK( retval );
        goto err_out;
    }

    nvs_close( handle );
    return 0;

err_out:
    nvs_close( handle );
    return -1;
}

int8_t user_data_is_valid( user_data_t* user_data )
{
    if( strlen(user_data->wifi_client_ssid) <= 0 )
    {
        return -1;
    }
    if( strlen(user_data->xi_account_id) <= 0 )
    {
        return -1;
    }
    if( strlen(user_data->xi_device_id) <= 0 )
    {
        return -1;
    }
    if( strlen(user_data->xi_device_password) <= 0 )
    {
        return -1;
    }
    return 0;
}

/******************************************************************************
*                                     Helpers                                 *
******************************************************************************/
/**
 * @brief Print a debug message displaying the contents of user_data, both in
 *        binary format and ASCII.
 *        A memory dump of the data itself is performed as to make it obvious to
 *        the developer whether there was anything stored in flash, help debug
 *        issues when modifying the user_data_t datastructure, etc.
 * @param user_data is the structure we'd like to print to UART
 */
void user_data_printf( user_data_t* user_data )
{
#if 0
    /* This debug message dumps the given *user_data memory block. Very useful
    to find possible errors when modifying the user_data_t datastructure */
    for ( uint32_t i = 0; i < sizeof( user_data_t ) / sizeof( int8_t ); i++ )
    {
        if ( 0 == i % 12 )
        {
            printf( "\r\n\t\t" );
        }
        printf( "%02x ", *( ( int8_t* )user_data + i ) );
    }
#endif

    printf( "\r\n\t* WiFi SSID: [%.64s]", user_data->wifi_client_ssid );
    //printf( "\r\n\t* WiFi Pwd: [%.64s]", user_data->wifi_client_password );
    printf( "\r\n\t* WiFi Pwd: ( REDACTED )" );
    printf( "\r\n\t* Xi Acc ID: [%.64s]", user_data->xi_account_id );
    printf( "\r\n\t* Xi Dev ID: [%.64s]", user_data->xi_device_id );
    //printf( "\r\n\t* Xi Dev Pwd: [%.64s]", user_data->xi_device_password );
    printf( "\r\n\t* Xi Dev Pwd: ( REDACTED )" );
}
