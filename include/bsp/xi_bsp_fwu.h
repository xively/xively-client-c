/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_BSP_FWU_H__
#define __XI_BSP_FWU_H__

/**
 * @file xi_bsp_fwu.h
 * @brief Xively Client's Board Support Platform (BSP) for Firmware Update
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

/**
 * @function
 * @brief Decides whether a resource is firmware.
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
 */
void xi_bsp_fwu_on_new_firmware_ok();

/**
 * @function
 * @brief This is an event notification function called when the Xively C Client
 *        considers the new firmware misfunctional.
 *
 * Called by the updated/new firmware.
 */
void xi_bsp_fwu_on_new_firmware_failure();

/**
 * @function
 * @brief This is an event notification function called when the Xively C Client
 *        detects an error during the download of an update package.
 *
 * Called by the firmware under update.
 */
void xi_bsp_fwu_on_package_download_failure();

/**
 * @function
 * @brief This is an event notification function called when the Xively C Client
 *        successfully downloaded an update package.
 *
 * @param [in] firmware_resource_name the name of the firmware binary found in the
 *                                    update package. NULL if there was no firmware
 *                                    binary in the package.
 *
 * Called by the firmware under update.
 * In this implementation devices usually mark new firmware for test and reboot.
 * Posix implementation may start a new process with the new firmware executable
 * and exit this process.
 */
void xi_bsp_fwu_on_package_download_finished( const char* const firmware_resource_name );


/**
 * @function
 * @brief Initializes checksum calculation for update package files.
 *
 * @param [out] checksum_context the context which will be passed with subsequent
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
 * @param [in] len number of bytes pointed by the data parameter
 */
void xi_bsp_fwu_checksum_update( void* checksum_context,
                                 const uint8_t* data,
                                 uint32_t len );

/**
 * @function
 * @brief Finishes and returns checksum calculated with the context.
 *
 * Memory: this function is also responsible to free up the context. Please not that
 *         the outgoing checksum array's memory isn't handled by the client at all. This
 *         means the implementation should solve the memory management, please check
 *         example implementations, they use a static byte array (32 bytes for SHA256).
 *
 * @param [in] checksum_context the context created by the checksum_init function
 * @param [in] buffer_out outgoing pointer on an array containing the checksum itself.
 * @param [in] buffer_len_out the number of bytes of the outgoing buffer
 */
void xi_bsp_fwu_checksum_final( void* checksum_context,
                                uint8_t** buffer_out,
                                uint16_t* buffer_len_out );

#ifdef __cplusplus
}
#endif

#endif /* __XI_BSP_FWU_H__ */
