/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_fwu.h>
#include <xi_bsp_time.h>
#include <xi_bsp_mem.h>
#include <xi_bsp_io_fs.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>

#ifdef __APPLE__

/* FreeBSD has getprogname() function in stdlib.h */

#else

extern char* program_invocation_short_name;

const char* getprogname()
{
    return program_invocation_short_name;
}

#endif

static char* _generate_update_filename( const char* const firmware_resource_name )
{
    static char new_filename[256] = {0};

    sprintf( new_filename, "%s_%lu", firmware_resource_name,
             xi_bsp_time_getcurrenttime_seconds() );

    return new_filename;
}

static const char* _get_firmware_test_flag_filename( const char* custom_progname )
{
    static char firmware_test_flag_filename[PATH_MAX + 20] = {0};

    firmware_test_flag_filename[0] = 0;

    strcat( firmware_test_flag_filename,
            custom_progname ? custom_progname : getprogname() );
    strcat( firmware_test_flag_filename, ".xitestfw" );

    return firmware_test_flag_filename;
}

uint8_t xi_bsp_fwu_is_this_firmware( const char* const resource_name )
{
    return ( 0 == strcmp( "firmware.bin", resource_name ) ) ? 1 : 0;
}

xi_bsp_fwu_state_t xi_bsp_fwu_on_new_firmware_ok()
{
    printf( "--- %s, no operation\n", __FUNCTION__ );

    const char* firmware_test_flag_filename = _get_firmware_test_flag_filename( NULL );

    xi_bsp_io_fs_stat_t stat = {0};

    /* check file existance which means first run of new firmware */
    xi_bsp_io_fs_state_t stat_result =
        xi_bsp_io_fs_stat( firmware_test_flag_filename, &stat );

    if ( XI_BSP_IO_FS_STATE_OK == stat_result )
    {
        /* actual firmware update happened */
        xi_bsp_io_fs_remove( firmware_test_flag_filename );

        return XI_BSP_FWU_ACTUAL_COMMIT_HAPPENED;
    }
    else
    {
        /* no update happened */
        return XI_BSP_FWU_STATE_OK;
    }
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

    if ( NULL == firmware_resource_name )
    {
        return;
    }

    /* - rename file to firmware_resource_name ## _DATETIMESTAMP */
    const char* update_fw_name = _generate_update_filename( firmware_resource_name );
    rename( firmware_resource_name, update_fw_name );

#if 1
    {
        /* - save the new firmware executable name to identify new firmware first run
         *   in function xi_bsp_fwu_on_new_firmware_ok */

        xi_bsp_io_fs_resource_handle_t resource_handle =
            XI_BSP_IO_FS_INVALID_RESOURCE_HANDLE;

        const char* firmware_test_flag_filename =
            _get_firmware_test_flag_filename( update_fw_name );

        xi_bsp_io_fs_open( firmware_test_flag_filename, 0, XI_BSP_IO_FS_OPEN_WRITE,
                           &resource_handle );

        xi_bsp_io_fs_close( resource_handle );
    }

    /* - start new firmware */
    char system_command[512] = {0};

    sprintf( system_command, "chmod a+x %s; "
                             "./%s "
                             "-a <Xively Account ID> "
                             "-u <Xively Device ID> "
                             "-P <Xively Device Password> "
                             "-p xi/blue/v1/<Xively Account ID>/d/"
                             "<Xively Device ID>/Temperature",
             update_fw_name, update_fw_name );

    printf( "[Xively C Client Firmware Update] executing command:\n%s\n",
            system_command );

    printf( "[Xively C Client Firmware Update] exiting this process\n" );

    const int system_call_result = system( system_command );

    // - exit this process
    exit( system_call_result );

#endif

    /* Control should never reach this */
}
