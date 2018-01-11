/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <string.h>

#include <xi_bsp_fwu.h>
#include <xively_error.h>
#include <xi_bsp_debug.h>
#include "xi_bsp_fwu_notifications_esp32.h"

#include "esp_system.h"
#include "esp_ota_ops.h"

uint8_t xi_bsp_fwu_is_this_firmware( const char* const resource_name )
{
    return ( 0 == strcmp( "firmware.bin", resource_name ) ) ? 1 : 0;
}

xi_bsp_fwu_state_t xi_bsp_fwu_on_new_firmware_ok()
{
    /* ESP32 doesn't have a commit/rollback mechanism in place for freshly
       installed images yet */
    xi_bsp_debug_logger( "New firmware image OK. No commit action implemented" );
    return XI_BSP_FWU_ACTUAL_COMMIT_HAPPENED;
}

void xi_bsp_fwu_on_new_firmware_failure()
{
    /* Updated image couldn't be validated / Roll back to previous image */
    /* ESP32 doesn't have a commit/rollback mechanism in place for freshly
       installed images yet */
    xi_bsp_debug_logger( "Error validating new image. No rollback action implemented" );
}

void xi_bsp_fwu_on_package_download_failure()
{
    xi_bsp_debug_logger( "Firmware image download failed. OTA update aborted!" );
}

void xi_bsp_fwu_on_package_download_finished( const char* const firmware_resource_name )
{
    if ( NULL != firmware_resource_name )
    {
        /* Firmware image was updated */
        xi_bsp_debug_logger( "SFT package download finished. FW updated; rebooting" );
        const esp_partition_t* next_partition = esp_ota_get_next_update_partition( NULL );
        esp_err_t retv                        = ESP_OK;

        retv = esp_ota_set_boot_partition( next_partition );
        if ( ESP_OK != retv )
        {
            xi_bsp_debug_format( "esp_ota_set_boot_partition() failed with error %d",
                                 retv );
            return;
        }

        if ( NULL != xi_bsp_fwu_notification_callbacks.update_applied )
        {
            ( xi_bsp_fwu_notification_callbacks.update_applied )();
        }

        /* reboot the device */
        esp_restart();
    }
    /* Firmware image was not updated */
    xi_bsp_debug_logger( "SFT package download finished. No need to reboot" );
    return;
}

void xi_bsp_fwu_order_resource_downloads( const char* const* resource_names,
                                          uint16_t list_len,
                                          int32_t* download_order )
{
    ( void )( resource_names );
    ( void )( list_len );
    ( void )( download_order );
    /* We don't care about the download order */
}
