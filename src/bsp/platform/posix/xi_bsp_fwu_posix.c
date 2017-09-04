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

xi_state_t xi_bsp_fwu_on_new_firmware_ok()
{
    printf( "--- %s, no operation\n", __FUNCTION__ );
    return XI_NOT_IMPLEMENTED;
}

xi_state_t xi_bsp_fwu_on_new_firmware_failure()
{
    printf( "--- %s, \n", __FUNCTION__ );
    return XI_NOT_IMPLEMENTED;
}

xi_state_t xi_bsp_fwu_on_firmware_package_download_finished(
    const char* const firmware_resource_name )
{
    printf( "--- %s, firmware name: %s\n", __FUNCTION__, firmware_resource_name );

    // - rename file to firmware_resource_name ## _DATETIMESTAMP
    const char* update_fw_name = _generate_update_filename( firmware_resource_name );
    rename( firmware_resource_name, update_fw_name );

#if 1
    // - start new firmware
    char system_command[512] = {0};

    sprintf( system_command, "chmod a+x %s; "
                             "./%s "
                             "-a <add your Xively Account ID here> "
                             "-u <add your Xively Device ID here> "
                             "-P <add your Xively password here> "
                             "-p <add target topic here>",
             update_fw_name, update_fw_name );

    printf( "[Xively C Client Firmware Update] executing command:\n%s\n",
            system_command );

    printf( "[Xively C Client Firmware Update] exiting this process\n" );

    const int system_call_result = system( system_command );

    // - exit this process
    exit( system_call_result );
#endif

    /* Control should never reach this */
    return XI_STATE_OK;
}
