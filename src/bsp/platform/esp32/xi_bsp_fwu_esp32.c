/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <string.h>

#include <xi_bsp_fwu.h>
#include <xively_error.h>
#include <xi_bsp_debug.h>

#include "esp_system.h"

#include "nvs.h"
#include "nvs_flash.h"

static void _reboot_device()
{
    esp_restart();
}

uint8_t xi_bsp_fwu_is_this_firmware( const char* const resource_name )
{
    return ( 0 == strcmp( "firmware.bin", resource_name ) ) ? 1 : 0;
}

xi_bsp_fwu_state_t xi_bsp_fwu_on_new_firmware_ok()
{
#if 1 /* JC TODO: Commit the new firmware image */
    if ( 1 )
    {
        return XI_BSP_FWU_ACTUAL_COMMIT_HAPPENED;
    }

    return XI_BSP_FWU_STATE_OK;
#endif
}

void xi_bsp_fwu_on_new_firmware_failure()
{
    /* Updated image couldn't be validated / Roll back to previous image */
    xi_bsp_debug_logger( "Error validating new firmware. Rolling back to old FW" );
    /* JC TODO: Find the alternate OTA image or roll back to factory default? */
    _reboot_device();
}

void xi_bsp_fwu_on_package_download_failure()
{
    xi_bsp_debug_logger( "Firmware image download failed. OTA update aborted!" );
}

void xi_bsp_fwu_on_package_download_finished( const char* const firmware_resource_name )
{
    ( void )firmware_resource_name;

    //esp_ota_end( esp_ota_handle );

    /* reboot the device */
    _reboot_device();
}
