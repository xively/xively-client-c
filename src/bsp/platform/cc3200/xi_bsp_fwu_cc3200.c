/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_fwu.h>
#include <xively_error.h>

#include <xi_bsp_mem.h>
#include <xi_bsp_io_fs.h>

#include <xi_helpers.h>
#include <xi_macros.h>

#include <simplelink.h>
#include <flc_api.h>
#include "hw_types.h"
#include "rom_map.h"
#include "prcm.h"
#include "utils.h"


uint8_t xi_bsp_fwu_is_this_firmware( const char* const resource_name )
{
    return ( 0 == strcmp( "firmware.bin", resource_name ) ) ? 1 : 0;
}

xi_state_t xi_bsp_fwu_commit()
{
    if ( sl_extlib_FlcIsPendingCommit() )
    {
        sl_extlib_FlcCommit( FLC_COMMITED );
    }

    return XI_STATE_OK;
}

xi_state_t xi_bsp_fwu_test()
{
    sl_extlib_FlcTest( FLC_TEST_RESET_MCU | FLC_TEST_RESET_MCU_WITH_APP );

    return XI_STATE_OK;
}

xi_state_t xi_bsp_fwu_reboot()
{
    /* Configure hibernate RTC wakeup */
    PRCMHibernateWakeupSourceEnable( PRCM_HIB_SLOW_CLK_CTR );

    /* Delay loop */
    MAP_UtilsDelay( 8000000 );

    /* Set wake up time */
    PRCMHibernateIntervalSet( 330 );

    /* Request hibernate */
    PRCMHibernateEnter();

    /* Control should never reach here */
    return XI_INTERNAL_ERROR;
}

#define XI_BSP_FWU_GET_REVISION_RESOURCENAME( resource_name )                            \
    xi_str_cat( resource_name, ".xirev" )

xi_state_t
xi_bsp_fwu_set_revision( const char* const resource_name, const char* const revision )
{
    ( void )resource_name;
    ( void )revision;
#if 0
    char* resource_name_revision = XI_BSP_FWU_GET_REVISION_RESOURCENAME( resource_name );

    xi_fs_resource_handle_t resource_handle = XI_FS_INVALID_RESOURCE_HANDLE;

    xi_bsp_io_fs_open( resource_name_revision, 0, XI_FS_OPEN_WRITE, &resource_handle );

    size_t bytes_written = 0;

    xi_bsp_io_fs_write( resource_handle, ( const uint8_t* )revision, strlen( revision ),
                        0, &bytes_written );

    xi_bsp_io_fs_close( resource_handle );

    XI_SAFE_FREE( resource_name_revision );

#endif
    return XI_STATE_OK;
}

xi_state_t xi_bsp_fwu_get_revision( const char* const resource_name, char** revision_out )
{
    const size_t revision_len = strlen( resource_name ) + 1;

    *revision_out = xi_bsp_mem_alloc( revision_len );

    if ( NULL == *revision_out )
    {
        return XI_OUT_OF_MEMORY;
    }

    memset( *revision_out, 0, revision_len );

    strcpy( *revision_out, resource_name );

    return XI_STATE_OK;
}
