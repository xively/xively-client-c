/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_fwu.h>
#include <xi_bsp_time.h>
#include <xi_bsp_mem.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static char* _generate_update_filename( const char* const firmware_resource_name )
{
    static char new_filename[256] = {0};

    sprintf( new_filename, "%s_%lu", firmware_resource_name,
             xi_bsp_time_getcurrenttime_seconds() );

    return new_filename;
}

uint8_t xi_bsp_fwu_is_this_firmware( const char* const resource_name )
{
    return ( 0 == strcmp( "firmware.bin", resource_name ) ) ? 1 : 0;
}

void xi_bsp_fwu_on_new_firmware_ok()
{
    printf( "--- %s, no operation\n", __FUNCTION__ );
}

void xi_bsp_fwu_on_new_firmware_failure()
{
    printf( "--- %s, \n", __FUNCTION__ );
}

void xi_bsp_fwu_on_package_download_failure()
{
    printf( "--- %s, \n", __FUNCTION__ );
}

void xi_bsp_fwu_on_package_download_finished( const char* const firmware_resource_name )
{
    printf( "--- %s, firmware name: %s\n", __FUNCTION__, firmware_resource_name );

    // - rename file to firmware_resource_name ## _DATETIMESTAMP
    const char* update_fw_name = _generate_update_filename( firmware_resource_name );
    rename( firmware_resource_name, update_fw_name );

#if 0
    // - start new firmware
    char system_command[512] = {0};

    sprintf( system_command, "chmod a+x %s; "
                             "./%s "
                             "-a 4d3c7986-8d53-4cf8-903e-7fd30ff63be1 "
                             "-u fc4ba77e-0ba0-4ef2-bb46-d30bc59546ab "
                             "-P JMnzPt3zDH2ZY2Q/ZQRgzublMGF6VfAFv7vFxFf5tLY= "
                             "-p xi/blue/v1/4d3c7986-8d53-4cf8-903e-7fd30ff63be1/d/"
                             "b03e863d-2478-48cf-976e-fa2d5ddf4ec8/Temperature",
             update_fw_name, update_fw_name );

    printf( "[Xively C Client Firmware Update] executing command:\n%s\n",
            system_command );

    printf( "[Xively C Client Firmware Update] exiting this process\n" );

    system( system_command );

    // - exit this process
    exit( 0 );
#endif

    /* Control should never reach this */
}
