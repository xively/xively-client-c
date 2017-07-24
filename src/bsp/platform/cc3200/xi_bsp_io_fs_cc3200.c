/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
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

static _i32 firmware_file_handle_last_opened = 0;

#define XI_DEBUG__FOR_FIRMWARE_UPDATE_TESTING_PURPOSES

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

#endif

xi_state_t xi_bsp_io_fs_open( const char* const resource_name,
                              const uint32_t size,
                              const xi_fs_open_flags_t open_flags,
                              xi_fs_resource_handle_t* resource_handle_out )
{
    ( void )open_flags;

#ifdef XI_DEBUG__FOR_FIRMWARE_UPDATE_TESTING_PURPOSES
    _ReadBootInfo( &sBootInfo );
#endif

    if ( 1 == xi_bsp_fwu_is_firmware( resource_name ) )
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
