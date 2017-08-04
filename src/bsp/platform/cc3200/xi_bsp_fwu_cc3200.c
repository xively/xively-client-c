/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_fwu.h>
#include <xively_error.h>

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

xi_state_t xi_bsp_fwu_on_firmware_package_download_finished()
{
    sl_extlib_FlcTest( FLC_TEST_RESET_MCU | FLC_TEST_RESET_MCU_WITH_APP );

    /* reboot the device */

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
