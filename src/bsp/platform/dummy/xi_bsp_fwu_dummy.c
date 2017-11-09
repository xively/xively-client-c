/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_fwu.h>
#include <xively_error.h>

uint8_t xi_bsp_fwu_is_this_firmware( const char* const resource_name )
{
    ( void )resource_name;
    return 0;
}

xi_bsp_fwu_state_t xi_bsp_fwu_on_new_firmware_ok()
{
    return XI_BSP_FWU_STATE_OK;
}

void xi_bsp_fwu_on_new_firmware_failure()
{
    return;
}

void xi_bsp_fwu_on_package_download_failure()
{
    return;
}

void xi_bsp_fwu_order_resource_downloads( const char* const* resource_names,
                                          uint16_t list_len,
                                          int32_t* download_order )
{
    ( void )( resource_names );
    ( void )( list_len );
    ( void )( download_order );
}


void xi_bsp_fwu_on_package_download_finished( const char* const firmware_resource_name )
{
    ( void )firmware_resource_name;

    return;
}
