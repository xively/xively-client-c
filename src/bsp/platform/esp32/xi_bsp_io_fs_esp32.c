/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <sys/stat.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <xi_bsp_debug.h>
#include <xi_bsp_io_fs.h>
#include <xi_bsp_fwu.h>
#include <xi_bsp_mem.h>
#include "xi_bsp_fwu_notifications_esp32.h"

#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_ota_ops.h"

#define XI_ESP32_MAX_FILENAME_LEN 128
#define ESP32_FS_BASE_PATH "/spiflash"

#define XI_BSP_IO_FS_READ_BUFFER_SIZE 1024

#define XI_BSP_IO_FS_CHECK_CND( cnd, e, s )                                              \
    if ( ( cnd ) )                                                                       \
    {                                                                                    \
        ( s ) = ( e );                                                                   \
        perror( "Xi FS BSP errno" );                                                     \
        goto err_handling;                                                               \
    }

/* ESP32 Over The Air FW updates API handle */
static esp_ota_handle_t open_firmware_bin_handle = 0;

/* translates bsp errno errors to the xi_bsp_io_fs_state_t values */
xi_bsp_io_fs_state_t xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( int errno_value )
{
    xi_bsp_io_fs_state_t ret = XI_BSP_IO_FS_STATE_OK;

    switch ( errno_value )
    {
        case EBADF:
        case EACCES:
        case EFAULT:
        case ENOENT:
        case ENOTDIR:
            ret = XI_BSP_IO_FS_RESOURCE_NOT_AVAILABLE;
            break;
        case ELOOP:
        case EOVERFLOW:
            ret = XI_BSP_IO_FS_ERROR;
            break;
        case EINVAL:
            ret = XI_BSP_IO_FS_INVALID_PARAMETER;
            break;
        case ENAMETOOLONG:
            ret = XI_BSP_IO_FS_INVALID_PARAMETER;
            break;
        case ENOMEM:
            ret = XI_BSP_IO_FS_OUT_OF_MEMORY;
            break;
        default:
            ret = XI_BSP_IO_FS_ERROR;
    }

    return ret;
}

xi_bsp_io_fs_state_t
xi_bsp_io_fs_open( const char* const resource_name,
                   const size_t size,
                   const xi_bsp_io_fs_open_flags_t open_flags,
                   xi_bsp_io_fs_resource_handle_t* resource_handle_out )
{
    char filepath[XI_ESP32_MAX_FILENAME_LEN] = {0};
    xi_bsp_io_fs_state_t ret                 = XI_BSP_IO_FS_STATE_OK;
    esp_err_t esp_retv                       = ESP_OK;
    FILE* fp                                 = NULL;

    if ( NULL == resource_name || NULL == resource_handle_out )
    {
        return XI_BSP_IO_FS_INVALID_PARAMETER;
    }

    if ( xi_bsp_fwu_is_this_firmware( resource_name ) )
    {
        const esp_partition_t* next_partition = esp_ota_get_next_update_partition( NULL );

        if ( NULL == next_partition )
        {
            xi_bsp_debug_logger( "Failed to get next OTA update partition" );
            return XI_BSP_IO_FS_OPEN_ERROR;
        }

        esp_retv = esp_ota_begin( next_partition, size,
                                  ( esp_ota_handle_t* )resource_handle_out );

        if ( ESP_OK != esp_retv )
        {
            xi_bsp_debug_format( "esp_ota_begin() failed with error %d", esp_retv );
            return XI_BSP_IO_FS_OPEN_ERROR;
        }

        if ( NULL != xi_bsp_fwu_notification_callbacks.update_started )
        {
            ( xi_bsp_fwu_notification_callbacks.update_started )( resource_name, size );
        }
        open_firmware_bin_handle = *resource_handle_out;
    }
    else
    {
        snprintf( filepath, XI_ESP32_MAX_FILENAME_LEN, "%s/%s", ESP32_FS_BASE_PATH,
                  resource_name );
        fp = fopen( ( const char* )filepath,
                    ( open_flags & XI_BSP_IO_FS_OPEN_READ ) ? "rb" : "wb" );

        /* if error on fopen check the errno value */
        XI_BSP_IO_FS_CHECK_CND(
            NULL == fp, xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( errno ), ret );

        *resource_handle_out = ( xi_bsp_io_fs_resource_handle_t )fp;

        return XI_BSP_IO_FS_STATE_OK;
    }

    return XI_BSP_IO_FS_STATE_OK;

err_handling:
    return ret;
}

xi_bsp_io_fs_state_t
xi_bsp_io_fs_write( const xi_bsp_io_fs_resource_handle_t resource_handle,
                    const uint8_t* const buffer,
                    const size_t buffer_size,
                    const size_t offset,
                    size_t* const bytes_written )
{
    xi_bsp_io_fs_state_t ret = XI_BSP_IO_FS_STATE_OK;
    esp_err_t retv           = ESP_OK;

    if ( ( esp_ota_handle_t )resource_handle == open_firmware_bin_handle )
    {
        retv = esp_ota_write( resource_handle, buffer, buffer_size );
        if ( ESP_OK != retv )
        {
            xi_bsp_debug_format( "esp_ota_write() failed with error %d", retv );
            return XI_BSP_IO_FS_WRITE_ERROR;
        }

        *bytes_written = buffer_size;
        if ( NULL != xi_bsp_fwu_notification_callbacks.chunk_written )
        {
            ( xi_bsp_fwu_notification_callbacks.chunk_written )( buffer_size, offset );
        }
    }
    else
    {
        if ( NULL == buffer || 0 == buffer_size ||
             XI_BSP_IO_FS_INVALID_RESOURCE_HANDLE == resource_handle )
        {
            return XI_BSP_IO_FS_INVALID_PARAMETER;
        }

        FILE* fp = ( FILE* )resource_handle;

        /* let's set the offset */
        const int fop_ret = fseek( fp, offset, SEEK_SET );

        /* if error on fseek check errno */
        XI_BSP_IO_FS_CHECK_CND(
            fop_ret != 0, xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( errno ), ret );

        *bytes_written = fwrite( buffer, ( size_t )1, buffer_size, fp );

        /* if error on fwrite check errno */
        XI_BSP_IO_FS_CHECK_CND(
            buffer_size != *bytes_written,
            xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( ferror( fp ) ), ret );
    }
    return XI_BSP_IO_FS_STATE_OK;

err_handling:
    return ret;
}

xi_bsp_io_fs_state_t
xi_bsp_io_fs_read( const xi_bsp_io_fs_resource_handle_t resource_handle,
                   const size_t offset,
                   const uint8_t** buffer,
                   size_t* const buffer_size )
{
    xi_bsp_io_fs_state_t ret = XI_BSP_IO_FS_STATE_OK;
    FILE* fp                 = ( FILE* )resource_handle;
    int fop_ret              = 0;

    if ( XI_BSP_IO_FS_INVALID_RESOURCE_HANDLE == resource_handle )
    {
        xi_bsp_debug_logger( "\nInvalid resource handle - Filesystem read failed" );
        return XI_BSP_IO_FS_INVALID_PARAMETER;
    }

    if ( ( esp_ota_handle_t )resource_handle == open_firmware_bin_handle )
    {
        xi_bsp_debug_logger( "Error: No FS read of FW files in ESP32's OTA API" );
        return XI_BSP_IO_FS_NOT_IMPLEMENTED;
    }
    else
    {
        static uint8_t read_buffer[XI_BSP_IO_FS_READ_BUFFER_SIZE] = {0};

        /* let's set an offset */
        fop_ret = fseek( fp, offset, SEEK_SET );

        /* if error on fseek check errno */
        XI_BSP_IO_FS_CHECK_CND(
            fop_ret != 0, xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( errno ), ret );

        /* use the fread to read the file chunk */
        fop_ret = fread( read_buffer, ( size_t )1, XI_BSP_IO_FS_READ_BUFFER_SIZE, fp );

        /* if error on fread check errno */
        XI_BSP_IO_FS_CHECK_CND(
            fop_ret == 0, xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( errno ), ret );

        *buffer      = read_buffer;
        *buffer_size = fop_ret;
    }
    return XI_BSP_IO_FS_STATE_OK;

err_handling:
    *buffer      = NULL;
    *buffer_size = 0;

    return ret;
}

xi_bsp_io_fs_state_t
xi_bsp_io_fs_close( const xi_bsp_io_fs_resource_handle_t resource_handle )
{
    xi_bsp_io_fs_state_t ret = XI_BSP_IO_FS_STATE_OK;
    FILE* fp                 = ( FILE* )resource_handle;
    int fop_ret              = 0;

    if ( ( esp_ota_handle_t )resource_handle == open_firmware_bin_handle )
    {
        esp_err_t retv = ESP_OK;

        retv = esp_ota_end( resource_handle );
        if ( ESP_OK != retv )
        {
            xi_bsp_debug_format( "esp_ota_end() failed with error %d", retv );
            return XI_BSP_IO_FS_WRITE_ERROR;
        }
    }
    else
    {
        if ( XI_BSP_IO_FS_INVALID_RESOURCE_HANDLE == resource_handle )
        {
            return XI_BSP_IO_FS_INVALID_PARAMETER;
        }

        fop_ret = fclose( fp );

        /* if error on fclose check errno */
        XI_BSP_IO_FS_CHECK_CND(
            0 != fop_ret, xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( errno ), ret );
    }
    return XI_BSP_IO_FS_STATE_OK;

err_handling:
    return ret;
}

xi_bsp_io_fs_state_t xi_bsp_io_fs_remove( const char* const resource_name )
{
    char filepath[XI_ESP32_MAX_FILENAME_LEN] = {0};
    int ret                                  = -1;

    if ( xi_bsp_fwu_is_this_firmware( resource_name ) )
    {
        xi_bsp_debug_logger( "FS Remove not implemented for FW images" );
        return XI_BSP_IO_FS_NOT_IMPLEMENTED;
    }
    else
    {
        snprintf( filepath, XI_ESP32_MAX_FILENAME_LEN, "%s/%s", ESP32_FS_BASE_PATH,
                  resource_name );

        ret = remove( filepath );
        XI_BSP_IO_FS_CHECK_CND(
            0 != ret, xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( errno ), ret );
    }
    return XI_BSP_IO_FS_STATE_OK;

err_handling:
    return ret;
}
