/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
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

#include <gpio_if.h>

static void _reboot_device()
{
    /* Configure hibernate RTC wakeup */
    PRCMHibernateWakeupSourceEnable( PRCM_HIB_SLOW_CLK_CTR );

    /* Delay loop */
    MAP_UtilsDelay( 8000000 );

    /* Set wake up time */
    PRCMHibernateIntervalSet( 330 );

    /* Request hibernate */
    PRCMHibernateEnter();
}

uint8_t xi_bsp_fwu_is_this_firmware( const char* const resource_name )
{
    return ( 0 == strcmp( "firmware.bin", resource_name ) ) ? 1 : 0;
}

xi_bsp_fwu_state_t xi_bsp_fwu_on_new_firmware_ok()
{
    if ( sl_extlib_FlcIsPendingCommit() )
    {
        sl_extlib_FlcCommit( FLC_COMMITED );

        return XI_BSP_FWU_ACTUAL_COMMIT_HAPPENED;
    }

    return XI_BSP_FWU_STATE_OK;
}

void xi_bsp_fwu_on_new_firmware_failure()
{
    if ( sl_extlib_FlcIsPendingCommit() )
    {
        sl_extlib_FlcCommit( FLC_NOT_COMMITED );
    }

    _reboot_device();
}

void xi_bsp_fwu_on_package_download_failure()
{
    GPIO_IF_LedOn( MCU_RED_LED_GPIO );
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
    if ( NULL != firmware_resource_name )
    {
        /* Firmware image was updated */
        sl_extlib_FlcTest( FLC_TEST_RESET_MCU | FLC_TEST_RESET_MCU_WITH_APP );

        /* reboot the device */
        _reboot_device();
    }
    /* Firmware image was not updated */
    return;
}
