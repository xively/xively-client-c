/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_BSP_FWU_H__
#define __XI_BSP_FWU_H__

/**
 * @file xi_bsp_fwu.h
 * @brief Xively Client's Board Support Package (BSP) for Firmware Updates
 *
 * This file defines the Firmware Update (FWU) API used by the Xively C Client.
 *
 * The Xively C Client calls these functions during the Secure File Transfer (SFT) and
 * Firmware Update (FWU) processes. Some of the functions are event notifications
 * reflecting the status of the update process, some are device specific action requests
 * others are checksum generators.
 */

#include <xively_error.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum xi_bsp_fwu_state_e {
    XI_BSP_FWU_STATE_OK               = 0,
    XI_BSP_FWU_ACTUAL_COMMIT_HAPPENED = 1,
} xi_bsp_fwu_state_t;

/**
 * @function
 * @brief Determines whether a resource is firmware.
 *
 * In Xively's Firmware Update service the application has to be aware of how
 * a firmware resource is named. If the Xively C Client encounters a resource
 * which is considered as firmware it will activate the FWU process on it.
 *
 * @param [in] resource_name name of the resource the decision has to be based on
 * @return 1 if the resource is firmware, 0 otherwise
 */
uint8_t xi_bsp_fwu_is_this_firmware( const char* const resource_name );

/**
 * @function
 * @brief This is an event notification function called when the Xively C Client
 *        considers the new firmare functional.
 *
 * Called by the updated/new firmware.
 *
 * @return XI_BSP_FWU_ACTUAL_COMMIT_HAPPENED if the function actually committed the
 *         firmware. This means the new firmware runs first time.
 *         XI_BSP_FWU_STATE_OK otherwise.
 */
xi_bsp_fwu_state_t xi_bsp_fwu_on_new_firmware_ok();

/**
 * @function
 * @brief This is an event notification function called when the Xively C Client
 *        considers the new firmware misfunctional.
 *
 * This function is called when the client explicitly detects an error.
 * Currently this function is only called when the client is unable to connect to Xively
 * Service through Control Topic. Since this would disable further update capability.
 * It's up to the application what to do in such situation. For reference please
 * check the TI CC3200 implementation in `xi_bsp_fwu_cc3200.c` file. This
 * implementation reboots itself which results in a rollback to previous firmware.
 *
 * Called by the updated/new firmware.
 */
void xi_bsp_fwu_on_new_firmware_failure();

/**
 * @function
 * @brief This is an event notification function called when the Xively C Client
 *        detects an error during the download of resources.
 *
 * Called by the firmware under update.
 */
void xi_bsp_fwu_on_package_download_failure();

/**
 * @function
 * @brief This is an event notification function called when the Xively C Client
 *        has successfully downloaded all of the resources of an update package.
 *
 * @param [in] firmware_resource_name the name of the firmware binary found in the
 *                                    update package. NULL if there was no firmware
 *                                    binary in the package.
 *
 * Called by the firmware under update.
 *
 * Implement your platform's reaction to a full update being installed. In some
 * implementations, devices may mark new firmware for test and reboot. In our POSIX
 * reference implementation, we start the new client in a new process and kill this
 * process, for instance.
 */
void xi_bsp_fwu_on_package_download_finished( const char* const firmware_resource_name );


/**
 * @function
 * @brief Initializes the checksum calculation for update package files.
 *
 * @param [out] checksum_context the context which will be passed to subsequent
 *                               checksum function calls. The implementation should
 *                               store the context pointer on *checksum_context.
 */
void xi_bsp_fwu_checksum_init( void** checksum_context );

/**
 * @function
 * @brief Handles incremental updates during checksum calculation for update package
 *        files.
 *
 * @param [in] checksum_context the context created by the checksum_init function
 * @param [in] data the checksum should be calculated on this data
 * @param [in] len number of bytes pointed to the data parameter
 */
void xi_bsp_fwu_checksum_update( void* checksum_context,
                                 const uint8_t* data,
                                 uint32_t len );

/**
 * @function
 * @brief Finishes and returns the checksum value calculated with the context.
 *
 * Note: this function is also responsible to free up the context. Please note that
 *       the outgoing checksum array's memory isn't handled by the client at all. This
 *       means the implementation should solve the memory management, please check
 *       example implementations, they use a static byte array (32 bytes for SHA256).
 *
 * @param [in]  checksum_context the context created by the checksum_init function
 * @param [out] buffer_out outgoing pointer on an array containing the checksum itself.
 * @param [out] buffer_len_out the number of bytes of the outgoing buffer
 */
void xi_bsp_fwu_checksum_final( void* checksum_context,
                                uint8_t** buffer_out,
                                uint16_t* buffer_len_out );

#ifdef __cplusplus
}
#endif

#endif /* __XI_BSP_FWU_H__ */
