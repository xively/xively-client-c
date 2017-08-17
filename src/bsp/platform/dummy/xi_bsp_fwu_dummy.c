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

xi_state_t xi_bsp_fwu_commit()
{
    return XI_NOT_IMPLEMENTED;
}

xi_state_t xi_bsp_fwu_test()
{
    return XI_NOT_IMPLEMENTED;
}

xi_state_t xi_bsp_fwu_reboot()
{
    return XI_NOT_IMPLEMENTED;
}
