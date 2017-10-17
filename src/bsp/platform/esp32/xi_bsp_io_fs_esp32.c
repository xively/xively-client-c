/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_debug.h>
#include <xi_bsp_io_fs.h>
#include <xi_bsp_fwu.h>
#include <string.h>
#include <stdio.h>

#include "esp_ota_ops.h"
#include "nvs.h"
#include "nvs_flash.h"

void __attribute__( ( weak ) )
xi_esp32_ota_chunk_recv( size_t offset, size_t chunk_size );

xi_bsp_io_fs_state_t
xi_bsp_io_fs_open( const char* const resource_name,
                   const size_t size,
                   const xi_bsp_io_fs_open_flags_t open_flags,
                   xi_bsp_io_fs_resource_handle_t* resource_handle_out )
{
    ( void )open_flags;

    if ( 1 != xi_bsp_fwu_is_this_firmware( resource_name ) )
    {
        xi_bsp_debug_format( "File [%s] not supported. Use firmware.bin", resource_name );
        return XI_BSP_IO_FS_OPEN_ERROR;
    }

    const esp_partition_t* next_partition = esp_ota_get_next_update_partition( NULL );

    if ( NULL == next_partition )
    {
        xi_bsp_debug_logger( "Failed to get next OTA update partition" );
        return XI_BSP_IO_FS_OPEN_ERROR;
    }

    const esp_err_t retv =
        esp_ota_begin( next_partition, size, ( esp_ota_handle_t* )resource_handle_out );


    if ( ESP_OK != retv )
    {
        xi_bsp_debug_format( "esp_ota_begin() failed with error %d", retv );
        return XI_BSP_IO_FS_OPEN_ERROR;
    }

    return XI_BSP_IO_FS_STATE_OK;
}

xi_bsp_io_fs_state_t
xi_bsp_io_fs_write( const xi_bsp_io_fs_resource_handle_t resource_handle,
                    const uint8_t* const buffer,
                    const size_t buffer_size,
                    const size_t offset,
                    size_t* const bytes_written )
{
    ( void )offset;
    xi_bsp_debug_format( "Writing OTA file chunk.\n\tOffset: %d\n\tBuffer_size: %d",
                         offset, buffer_size );

    const esp_err_t retv = esp_ota_write( resource_handle, buffer, buffer_size );

    if ( ESP_OK != retv )
    {
        xi_bsp_debug_format( "esp_ota_write() failed with error %d", retv );
        return XI_BSP_IO_FS_WRITE_ERROR;
    }

    *bytes_written = buffer_size;
    return XI_BSP_IO_FS_STATE_OK;
}

xi_bsp_io_fs_state_t
xi_bsp_io_fs_close( const xi_bsp_io_fs_resource_handle_t resource_handle )
{
    const esp_partition_t* next_partition = esp_ota_get_next_update_partition( NULL );
    esp_err_t retv                        = ESP_OK;

    xi_bsp_debug_format( "Finished FW download to partition [%p]", next_partition );

    retv = esp_ota_end( resource_handle );
    if ( ESP_OK != retv )
    {
        xi_bsp_debug_format( "esp_ota_end() failed with error %d", retv );
        return XI_BSP_IO_FS_WRITE_ERROR;
    }

    retv = esp_ota_set_boot_partition( next_partition );
    if ( ESP_OK != retv )
    {
        xi_bsp_debug_format( "esp_ota_set_boot_partition() failed with error %d", retv );
        return XI_BSP_IO_FS_WRITE_ERROR;
    }

    return XI_BSP_IO_FS_STATE_OK;
}

xi_bsp_io_fs_state_t
xi_bsp_io_fs_read( const xi_bsp_io_fs_resource_handle_t resource_handle,
                   const size_t offset,
                   const uint8_t** buffer,
                   size_t* const buffer_size )
{
    ( void )resource_handle;
    ( void )offset;
    ( void )buffer;
    ( void )buffer_size;
    xi_bsp_debug_logger( "Error: No FS read in ESP32's OTA API" );
    return XI_BSP_IO_FS_NOT_IMPLEMENTED;
}

xi_bsp_io_fs_state_t xi_bsp_io_fs_remove( const char* const resource_name )
{
    ( void )resource_name;
    xi_bsp_debug_logger( "Error: No FS remove in ESP32's OTA API" );
    return XI_BSP_IO_FS_NOT_IMPLEMENTED;
}
