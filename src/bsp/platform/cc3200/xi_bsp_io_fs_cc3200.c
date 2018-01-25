/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_io_fs.h>
#include <xi_bsp_fwu.h>
#include <string.h>
#include <stdio.h>

#include <simplelink.h>
#include <gpio_if.h>

#define XI_DEBUG__FOR_FIRMWARE_UPDATE_TESTING_PURPOSES

static _i32 firmware_file_handle_last_opened = 0;

#ifdef XI_DEBUG__FOR_FIRMWARE_UPDATE_TESTING_PURPOSES

#include <flc_api.h>
typedef struct sBootInfo
{
    _u8 ucActiveImg;
    _u32 ulImgStatus;

} sBootInfo_t;

static sBootInfo_t sBootInfo;
#define IMG_ACT_USER1 1
#define IMG_ACT_USER2 2
#define IMG_BOOT_INFO "/sys/mcubootinfo.bin"

/* copied from TI SDK's flc.c file */
static _i32 _ReadBootInfo( sBootInfo_t* psBootInfo )
{
    _i32 lFileHandle;
    _u32 ulToken;
    _i32 status = -1;

    if ( 0 ==
         sl_FsOpen( ( _u8* )IMG_BOOT_INFO, FS_MODE_OPEN_READ, &ulToken, &lFileHandle ) )
    {
        if ( 0 < sl_FsRead( lFileHandle, 0, ( _u8* )psBootInfo, sizeof( sBootInfo_t ) ) )
        {
            status = 0;
        }
        sl_FsClose( lFileHandle, 0, 0, 0 );
    }

    return status;
}

#endif /* XI_DEBUG__FOR_FIRMWARE_UPDATE_TESTING_PURPOSES */

uint8_t xi_bsp_io_fs_is_this_cc3200_firmware_filename( const char* const filename )
{
    if ( ( 0 == strcmp( "/sys/mcuimg.bin", filename ) ) ||
         ( 0 == strcmp( "/sys/mcuimg1.bin", filename ) ) ||
         ( 0 == strcmp( "/sys/mcuimg2.bin", filename ) ) ||
         ( 0 == strcmp( "/sys/mcuimg3.bin", filename ) ) )
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
            return FS_MODE_OPEN_CREATE( size, FS_MODE_OPEN_WRITE );
        case XI_BSP_IO_FS_OPEN_APPEND:
            return FS_MODE_OPEN_WRITE;
        case XI_BSP_IO_FS_OPEN_READ:
        default:
            return FS_MODE_OPEN_READ;
    }
}

xi_bsp_io_fs_state_t xi_bsp_io_fs_open( const char* const resource_name,
                                        const size_t size,
                                        const xi_bsp_io_fs_open_flags_t open_flags,
                                        xi_bsp_io_fs_resource_handle_t* resource_handle_out )
{
    ( void )open_flags;

#ifdef XI_DEBUG__FOR_FIRMWARE_UPDATE_TESTING_PURPOSES
    _ReadBootInfo( &sBootInfo );
#endif

    if ( 1 == xi_bsp_fwu_is_this_firmware( resource_name ) )
    {
        /* the resource is firmware, handle with FLC (flc_api.h) */

        if ( 0 != sl_extlib_FlcOpenFile( "/sys/mcuimgA.bin", size, NULL,
                                         &firmware_file_handle_last_opened,
                                         FS_MODE_OPEN_WRITE ) )
        {
            firmware_file_handle_last_opened = 0;
            *resource_handle_out             = 0;
            return XI_BSP_IO_FS_OPEN_ERROR;
        }

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
            ( 1 == xi_bsp_io_fs_is_this_cc3200_firmware_filename( resource_name ) )
                ? FS_MODE_OPEN_READ
                : access_mode_desired;

        if ( 0 != sl_FsOpen( ( _u8* )resource_name, access_mode, NULL, &file_handle ) )
        {
            *resource_handle_out = 0;
            return XI_BSP_IO_FS_OPEN_ERROR;
        }

        *resource_handle_out = file_handle;

        return ( access_mode == access_mode_desired ) ? XI_BSP_IO_FS_STATE_OK
                                                      : XI_BSP_IO_FS_OPEN_READ_ONLY;
    }

    return XI_BSP_IO_FS_STATE_OK;
}

#define XI_BSP_IO_FS_READ_BUFFER_SIZE 1024

xi_bsp_io_fs_state_t xi_bsp_io_fs_read( const xi_bsp_io_fs_resource_handle_t resource_handle,
                              const size_t offset,
                              const uint8_t** buffer,
                              size_t* const buffer_size )
{
    _i32 result_file_read = 0;

    static _u8 read_buffer[ XI_BSP_IO_FS_READ_BUFFER_SIZE ];

    if ( resource_handle == firmware_file_handle_last_opened )
    {
        result_file_read = sl_extlib_FlcReadFile( resource_handle, offset, read_buffer,
                                                  XI_BSP_IO_FS_READ_BUFFER_SIZE );
    }
    else
    {
        result_file_read = sl_FsRead( resource_handle, offset, read_buffer,
                                      XI_BSP_IO_FS_READ_BUFFER_SIZE );
    }

    if ( result_file_read < 0 )
    {
        return XI_BSP_IO_FS_READ_ERROR;
    }
    else
    {
        *buffer      = read_buffer;
        *buffer_size = result_file_read;

        return XI_BSP_IO_FS_STATE_OK;
    }
}

xi_bsp_io_fs_state_t xi_bsp_io_fs_write( const xi_bsp_io_fs_resource_handle_t resource_handle,
                                         const uint8_t* const buffer,
                                         const size_t buffer_size,
                                         const size_t offset,
                                         size_t* const bytes_written )
{
    { /* led indicator of file writes */
#ifdef XI_DEBUG__FOR_FIRMWARE_UPDATE_TESTING_PURPOSES
        if ( IMG_ACT_USER1 == sBootInfo.ucActiveImg )
        {
            GPIO_IF_LedOff( MCU_ORANGE_LED_GPIO );
            GPIO_IF_LedToggle( MCU_GREEN_LED_GPIO );
        }
        else
        {
            GPIO_IF_LedOff( MCU_GREEN_LED_GPIO );
            GPIO_IF_LedToggle( MCU_ORANGE_LED_GPIO );
        }
#else
        GPIO_IF_LedToggle( MCU_GREEN_LED_GPIO );
#endif
    }

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
        *bytes_written =
            sl_FsWrite( resource_handle, offset, ( _u8* )buffer, buffer_size );
    }

    return ( buffer_size == *bytes_written ) ? XI_BSP_IO_FS_STATE_OK : XI_BSP_IO_FS_WRITE_ERROR;
}

xi_bsp_io_fs_state_t xi_bsp_io_fs_close( const xi_bsp_io_fs_resource_handle_t resource_handle )
{
    if ( resource_handle == firmware_file_handle_last_opened )
    {
        firmware_file_handle_last_opened = 0;

        return ( 0 == sl_extlib_FlcCloseFile( resource_handle, NULL, NULL, 0 ) )
                   ? XI_BSP_IO_FS_STATE_OK
                   : XI_BSP_IO_FS_CLOSE_ERROR;
    }
    else
    {
        return ( 0 == sl_FsClose( resource_handle, NULL, NULL, 0 ) ) ? XI_BSP_IO_FS_STATE_OK
                                                                     : XI_BSP_IO_FS_CLOSE_ERROR;
    }
}

xi_bsp_io_fs_state_t xi_bsp_io_fs_remove( const char* const resource_name )
{
    /* prevent possible firmware file deletion */
    if ( 1 == xi_bsp_io_fs_is_this_cc3200_firmware_filename( resource_name ) )
    {
        return XI_BSP_IO_FS_REMOVE_ERROR;
    }

    return ( 0 == sl_FsDel( ( const _u8* )resource_name, NULL ) ) ? XI_BSP_IO_FS_STATE_OK
                                                                  : XI_BSP_IO_FS_REMOVE_ERROR;
}
