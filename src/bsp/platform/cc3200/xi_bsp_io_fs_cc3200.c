/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_io_fs.h>
#include <string.h>
#include <stdio.h>

#include <simplelink.h>
#include <flc_api.h>

static _i32 firmware_file_handle_last_opened = 0;

static uint8_t xi_bsp_io_fs__is_firmware( const char* const resource_name )
{
    return ( 0 == strcmp( "firmware.bin", resource_name ) ) ? 1 : 0;
}

xi_state_t xi_bsp_io_fs_open( const char* const resource_name,
                              const uint32_t size,
                              const xi_fs_open_flags_t open_flags,
                              xi_fs_resource_handle_t* resource_handle_out )
{
    ( void )open_flags;

    if ( 1 == xi_bsp_io_fs__is_firmware( resource_name ) )
    {
        /* the resource is firmware, handle with FLC (flc_api.h) */

        if ( 0 != sl_extlib_FlcOpenFile( "/sys/mcuimgA.bin", size, NULL,
                                         &firmware_file_handle_last_opened,
                                         FS_MODE_OPEN_WRITE ) )
        {
            firmware_file_handle_last_opened = 0;
            *resource_handle_out             = 0;
            return XI_FS_ERROR;
        }

        *resource_handle_out = firmware_file_handle_last_opened;

        printf( "--- file_handle: %d\n", firmware_file_handle_last_opened );
        printf( "---  res_handle: %d\n", *resource_handle_out );
    }
    else
    {
        /* it's an ordinary file, handle with file io (fs.h) */
    }

    return XI_STATE_OK;
}

xi_state_t xi_bsp_io_fs_write( const xi_fs_resource_handle_t resource_handle,
                               const uint8_t* const buffer,
                               const size_t buffer_size,
                               const size_t offset,
                               size_t* const bytes_written )
{
    /* reasonably NOT supporting simultanenous firmware writes */
    if ( resource_handle == firmware_file_handle_last_opened )
    {
        /* write firmware file */
        *bytes_written = sl_extlib_FlcWriteFile( resource_handle, offset, ( _u8* )buffer,
                                                 buffer_size );
    }
    else
    {
        /* write ordinary file */
    }

    return ( buffer_size == *bytes_written ) ? XI_STATE_OK : XI_FS_ERROR;
}

xi_state_t xi_bsp_io_fs_close( const xi_fs_resource_handle_t resource_handle )
{
    if ( resource_handle == firmware_file_handle_last_opened )
    {
        firmware_file_handle_last_opened = 0;

        return ( 0 == sl_extlib_FlcCloseFile( resource_handle, NULL, NULL, 0 ) )
                   ? XI_STATE_OK
                   : XI_FS_ERROR;
    }
    else
    {
        return ( 0 == sl_FsClose( resource_handle, NULL, NULL, 0 ) ) ? XI_STATE_OK
                                                                     : XI_FS_ERROR;
    }
}

xi_state_t xi_bsp_io_fs_remove( const char* const resource_name )
{
    return ( 0 == sl_FsDel( ( const _u8* )resource_name, NULL ) ) ? XI_STATE_OK
                                                                  : XI_FS_ERROR;
}
