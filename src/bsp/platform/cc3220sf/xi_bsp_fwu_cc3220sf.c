/* Copyright (c) 2003-2018, Xively All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_fwu.h>
#include <xi_bsp_debug.h>
#include <xively_error.h>

#include "simplelink.h"
#include "hw_types.h"
#include "prcm.h"

static void _reboot_device()
{
    xi_bsp_debug_logger( " Rebooting the CC3220!" );

    const _i16 status = sl_Stop( 200 );

    if ( 0 > status )
    {
        xi_bsp_debug_format( "SimpleLink processor failed to stop with error: [ %i ]",
                             status );

        while ( 1 )
            ;
    }

    /* Reset the MCU in order to test the bundle */
    PRCMHibernateCycleTrigger();
}

static _i16 _ota_get_pending_commit()
{
    SlFsControlGetStorageInfoResponse_t fs_control_storage_response;

    /* read bundle state and check if is "PENDING COMMIT" */
    const _i16 status =
        ( _i16 )sl_FsCtl( ( SlFsCtl_e )SL_FS_CTL_GET_STORAGE_INFO, 0, NULL, NULL, 0,
                          ( uint8_t* )&fs_control_storage_response,
                          sizeof( SlFsControlGetStorageInfoResponse_t ), NULL );
    if ( 0 > status )
    {
        xi_bsp_debug_format( "Error checking pending commit, status: [ %i ]", status );
        return status;
    }

    return ( SL_FS_BUNDLE_STATE_PENDING_COMMIT ==
             ( SlFsBundleState_e )fs_control_storage_response.FilesUsage.Bundlestate );
}

static _i16 _ota_commit()
{
    const SlFsControl_t fs_control = {.IncludeFilters = 0};

    const int16_t status =
        ( _i16 )sl_FsCtl( SL_FS_CTL_BUNDLE_COMMIT, 0, NULL, ( uint8_t* )&fs_control,
                          sizeof( SlFsControl_t ), NULL, 0, NULL );

    if ( 0 > status )
    {
        xi_bsp_debug_format( "Error attempting a commit, status: [ %i ]", status );
    }
    return status;
}

static _i16 _ota_rollback()
{
    const SlFsControl_t fs_control = {.IncludeFilters = 0};

    const _i16 status =
        ( _i16 )sl_FsCtl( SL_FS_CTL_BUNDLE_ROLLBACK, 0, NULL, ( uint8_t* )&fs_control,
                          sizeof( SlFsControl_t ), NULL, 0, NULL );

    if ( 0 > status )
    {
        xi_bsp_debug_format( "Error attempting a rollback, status: [ %i ]", status );
    }
    return status;
}


uint8_t xi_bsp_fwu_is_this_firmware( const char* const resource_name )
{
    return ( 0 == strcmp( "firmware.tar", resource_name ) ) ? 1 : 0;
}

xi_bsp_fwu_state_t xi_bsp_fwu_on_new_firmware_ok()
{
    if ( _ota_get_pending_commit() )
    {
        xi_bsp_debug_logger( " Pending commit, firmware OK." );

        /* What should we do if we get a failure to commit? Reboot? */
        _ota_commit();

        /* reset the watchdog timer so we don't falsly revert. */
        PRCMPeripheralReset( 0x0000000B );

        return XI_BSP_FWU_ACTUAL_COMMIT_HAPPENED;
    }

    return XI_BSP_FWU_STATE_OK;
}

void xi_bsp_fwu_on_new_firmware_failure()
{
    if ( _ota_get_pending_commit() )
    {
        xi_bsp_debug_logger( " Pending commit, firmware BAD." );

        _ota_rollback();
    }

    _reboot_device();
}

void xi_bsp_fwu_on_package_download_failure()
{
    /* how  do you toggle LED without a reference on CC3220? */
}

void xi_bsp_fwu_order_resource_downloads( const char* const* resource_names,
                                          uint16_t list_len,
                                          int32_t* download_order )
{
    ( void )( resource_names );
    ( void )( list_len );
    ( void )( download_order );

    /* just go in default order. */
}

void xi_bsp_fwu_on_package_download_finished( const char* const firmware_resource_name )
{
    ( void )firmware_resource_name;
    xi_bsp_debug_logger( "Download finished, attemting a reboot." );
    /* reboot the device */
    _reboot_device();
}
