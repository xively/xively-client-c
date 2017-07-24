/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_fwu.h>
#include <stdio.h>
#include <string.h>

uint8_t xi_bsp_fwu_is_firmware( const char* const resource_name )
{
    return ( 0 == strcmp( "firmware.bin", resource_name ) ) ? 1 : 0;
}

xi_state_t xi_bsp_fwu_commit()
{
    printf( "--- %s, no operation\n", __FUNCTION__ );
    return XI_STATE_OK;
}

xi_state_t xi_bsp_fwu_test()
{
    printf( "--- %s, no operation\n", __FUNCTION__ );
    return XI_STATE_OK;
}

xi_state_t xi_bsp_fwu_reboot()
{
    printf( "--- %s, no operation\n", __FUNCTION__ );
    return XI_STATE_OK;
}
