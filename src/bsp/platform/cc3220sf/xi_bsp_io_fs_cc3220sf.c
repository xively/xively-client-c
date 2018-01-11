/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_io_fs.h>
#include <xi_bsp_fwu.h>
#include <xi_bsp_debug.h>
#include <string.h>
#include <stdio.h>

#include "simplelink.h"
#include "OtaArchive.h"

static _i32 firmware_file_handle_last_opened = 0;
static OtaArchive_t firmware_archive;

typedef struct sBootInfo
{
    uint8_t ucActiveImg;
    uint32_t ulImgStatus;
    uint32_t ulStartWdtKey;
    uint32_t ulStartWdtTime;
} sBootInfo_t;


static xi_bsp_io_fs_state_t _start_ota_watchdog_timer()
{
    _i32 file_handle = 0;
    _u32 token       = 0;
    sBootInfo_t boot_info;
    _i32 return_val         = 0;
    _u32 timeout_in_seconds = 50;

    file_handle = sl_FsOpen( ( unsigned char* )"/sys/mcubootinfo.bin",
                             SL_FS_CREATE | SL_FS_OVERWRITE | SL_FS_CREATE_SECURE |
                                 SL_FS_CREATE_MAX_SIZE( sizeof( sBootInfo_t ) ) |
                                 SL_FS_CREATE_PUBLIC_WRITE | SL_FS_CREATE_NOSIGNATURE,
                             ( _u32* )&token );

    if ( 0 > file_handle )
    {
        return XI_BSP_IO_FS_OPEN_ERROR;
    }

    memset( &boot_info, 0, sizeof( sBootInfo_t ) );
    boot_info.ulStartWdtTime =
        40000000 * timeout_in_seconds; // Max time in seconds (104s)
    boot_info.ulStartWdtKey =
        0xAE42DB15; // This is the start key provided from the TI OTA example.

    return_val =
        sl_FsWrite( file_handle, 0, ( uint8_t* )&boot_info, sizeof( sBootInfo_t ) );
    return_val = sl_FsClose( file_handle, NULL, NULL, 0 );

    if ( 0 != return_val )
    {
        return XI_BSP_IO_FS_CLOSE_ERROR;
    }

    return XI_BSP_IO_FS_STATE_OK;
}


uint8_t xi_bsp_io_fs_is_this_cc3220sf_firmware_filename( const char* const filename )
{
    if ( ( 0 == strcmp( "/sys/mcuimg.bin", filename ) ) ||
         ( 0 == strcmp( "/codecert", filename ) ) ||
         ( 0 == strcmp( "/sys/mcubootinfo.bin", filename ) ) )
    {
        return 1;
    }

    return 0;
}

_u32 xi_bsp_io_fs_open_flags_to_sl_flags( const uint32_t size,
                                          const xi_bsp_io_fs_open_flags_t open_flags )
{
    switch ( open_flags )
    {
        case XI_BSP_IO_FS_OPEN_WRITE:
            return SL_FS_CREATE | SL_FS_OVERWRITE | SL_FS_CREATE_MAX_SIZE( size );
        case XI_BSP_IO_FS_OPEN_APPEND:
            return SL_FS_WRITE;
        case XI_BSP_IO_FS_OPEN_READ:
        default:
            return SL_FS_READ;
    }
}

xi_bsp_io_fs_state_t
xi_bsp_io_fs_open( const char* const resource_name,
                   const size_t size,
                   const xi_bsp_io_fs_open_flags_t open_flags,
                   xi_bsp_io_fs_resource_handle_t* resource_handle_out )
{
    xi_bsp_io_fs_state_t bsp_state = XI_BSP_IO_FS_STATE_OK;

    xi_bsp_debug_format( " Opening file: [ %s ] with flags: [ %i ]", resource_name,
                         open_flags );

    if ( 1 == xi_bsp_fwu_is_this_firmware( resource_name ) )
    {
        _i16 status;

        status = _start_ota_watchdog_timer();

        if ( XI_BSP_IO_FS_STATE_OK != status )
        {
            firmware_file_handle_last_opened = 0;
            *resource_handle_out             = 0;
            return XI_BSP_IO_FS_OPEN_ERROR;
        }

        status = OtaArchive_init( &firmware_archive );

        if ( 0 > status )
        {
            xi_bsp_debug_format( "Ota Archive initialization failed, status: %d",
                                 status );

            firmware_file_handle_last_opened = 0;
            *resource_handle_out             = 0;

            return XI_BSP_IO_FS_OPEN_ERROR;
        }

        xi_bsp_debug_logger( "Initialized the Ota Archive." );

        /* Specify a non-zero value for processing the archive. */
        firmware_file_handle_last_opened = -1;

        *resource_handle_out = firmware_file_handle_last_opened;
    }
    else
    {
        /* it's an ordinary file, handle with file io (fs.h) */

        _i32 file_handle = 0;

        const _u32 access_mode_desired =
            xi_bsp_io_fs_open_flags_to_sl_flags( size, open_flags );

        /* prevent accidental write of CC3200 firmware files by limiting access rights to
         * read only */
        const _u32 access_mode =
            ( 1 == xi_bsp_io_fs_is_this_cc3220sf_firmware_filename( resource_name ) )
                ? SL_FS_READ
                : access_mode_desired;

        file_handle = sl_FsOpen( ( _u8* )resource_name, access_mode, NULL );

        if ( 0 > file_handle )
        {
            xi_bsp_debug_format( "Failed to open with error: [ %i ]", file_handle );

            *resource_handle_out = 0;

            return XI_BSP_IO_FS_OPEN_ERROR;
        }

        *resource_handle_out = file_handle;

        bsp_state = ( access_mode == access_mode_desired ) ? XI_BSP_IO_FS_STATE_OK
                                                           : XI_BSP_IO_FS_OPEN_READ_ONLY;
    }

    xi_bsp_debug_format( " Opened resource: [ %s ] with handle: [ %i ]", resource_name,
                         resource_handle_out );

    return bsp_state;
}

#define XI_BSP_IO_FS_READ_BUFFER_SIZE 1024

xi_bsp_io_fs_state_t
xi_bsp_io_fs_read( const xi_bsp_io_fs_resource_handle_t resource_handle,
                   const size_t offset,
                   const uint8_t** buffer,
                   size_t* const buffer_size )
{
    xi_bsp_debug_format( " Reading from filehandle: [ %i ]", resource_handle );

    _i32 result_file_read = 0;

    static _u8 read_buffer[XI_BSP_IO_FS_READ_BUFFER_SIZE];

    if ( resource_handle == firmware_file_handle_last_opened )
    {
        /* We process the firmware archive and store the internal files,
 * the archive itself is not stored thus cannot be read. */
        xi_bsp_debug_logger( " Can't read from the archive file." );
        return XI_BSP_IO_FS_READ_ERROR;
    }

    result_file_read =
        sl_FsRead( resource_handle, offset, read_buffer, XI_BSP_IO_FS_READ_BUFFER_SIZE );

    if ( result_file_read < 0 )
    {
        return XI_BSP_IO_FS_READ_ERROR;
    }
    else
    {
        xi_bsp_debug_format( "Reading... [ %s ]", read_buffer );

        *buffer      = read_buffer;
        *buffer_size = result_file_read;

        return XI_BSP_IO_FS_STATE_OK;
    }
}

xi_bsp_io_fs_state_t
xi_bsp_io_fs_write( const xi_bsp_io_fs_resource_handle_t resource_handle,
                    const uint8_t* const buffer,
                    const size_t buffer_size,
                    const size_t offset,
                    size_t* const bytes_written )
{
    xi_bsp_debug_format( " Writing chunk to filehandle: [ %i ]", resource_handle );
    _i32 status;
    static _u32 archive_done = 0;

    if ( resource_handle == firmware_file_handle_last_opened )
    {
        status                     = 0;
        _i16 archive_bytes_written = 0;
        _i16 bytes_to_write        = buffer_size;
        *bytes_written             = 0;

        while ( ( 0 == status ) && ( *bytes_written < buffer_size ) &&
                ( 0 == archive_done ) )
        {
            xi_bsp_debug_format( " [ %i ] bytes left to write!", bytes_to_write );

            // Archive process only processes chunks up to 512 bytes.
            if ( bytes_to_write >= 512 )
            {
                status = OtaArchive_process( &firmware_archive,
                                             ( _u8* )( buffer + *bytes_written ), 512,
                                             &archive_bytes_written );
            }
            else
            {
                status = OtaArchive_process( &firmware_archive,
                                             ( _u8* )( buffer + *bytes_written ),
                                             bytes_to_write, &archive_bytes_written );
            }

            if ( status == 1 )
            {
                archive_done = 1;
            }

            xi_bsp_debug_format( " Wrote %i bytes to the file!", archive_bytes_written );

            bytes_to_write -= archive_bytes_written;
            *bytes_written += archive_bytes_written;
        }

        if ( 0 > status )
        {
            xi_bsp_debug_format( " Error writing to file with status code: [ %i ]",
                                 status );
            return XI_BSP_IO_FS_WRITE_ERROR;
        }

        return XI_BSP_IO_FS_STATE_OK;
    }
    else
    {
        xi_bsp_debug_format( " Writing... [ %s ]", buffer );
        *bytes_written =
            sl_FsWrite( resource_handle, offset, ( _u8* )buffer, buffer_size );
    }

    return ( buffer_size == *bytes_written ) ? XI_BSP_IO_FS_STATE_OK
                                             : XI_BSP_IO_FS_WRITE_ERROR;
}

xi_bsp_io_fs_state_t
xi_bsp_io_fs_close( const xi_bsp_io_fs_resource_handle_t resource_handle )
{
    xi_bsp_debug_format( " Closing filehandle: [ %i ]", resource_handle );

    if ( resource_handle == firmware_file_handle_last_opened )
    {
        firmware_file_handle_last_opened = 0;

        return XI_BSP_IO_FS_STATE_OK;
    }
    else
    {
        return ( 0 == sl_FsClose( resource_handle, NULL, NULL, 0 ) )
                   ? XI_BSP_IO_FS_STATE_OK
                   : XI_BSP_IO_FS_CLOSE_ERROR;
    }
}

xi_bsp_io_fs_state_t xi_bsp_io_fs_remove( const char* const resource_name )
{
    xi_bsp_debug_format( " Deleting file: [ %s ]", resource_name );

    /* prevent possible firmware file deletion */
    if ( 1 == xi_bsp_io_fs_is_this_cc3220sf_firmware_filename( resource_name ) )
    {
        return XI_BSP_IO_FS_REMOVE_ERROR;
    }

    return ( 0 == sl_FsDel( ( const _u8* )resource_name, NULL ) )
               ? XI_BSP_IO_FS_STATE_OK
               : XI_BSP_IO_FS_REMOVE_ERROR;
}
