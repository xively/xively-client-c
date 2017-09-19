/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_BSP_FWU_H__
#define __XI_BSP_FWU_H__

#include <xively_error.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef xi_bsp_io_fwu_state_e
 * @brief Return value of the BSP NET API functions.
 *
 * The implementation reports internal status to Xively Client through these values.
 */
 typedef enum xi_bsp_io_fwu_state_e {
    /** operation finished successfully */
    XI_BSP_IO_FWU_STATE_OK = 0,
    /** operation failed on generic error */
    XI_BSP_IO_FWU_ERROR = 1,
    /** operation encountered and unexpected state or control path */
    XI_BSP_IO_FWU_INTERNAL_ERROR = 2,
    /** operation is not supported or not implemented */
    XI_BSP_IO_FWU_NOT_IMPLEMENTED = 3,
    /** parameter provided to BSP is invalid */
    XI_BSP_IO_FWU_INVALID_PARAMETER = 4,
} xi_bsp_io_fwu_state_t;


uint8_t xi_bsp_fwu_is_this_firmware( const char* const resource_name );

xi_bsp_io_fwu_state_t xi_bsp_fwu_on_new_firmware_ok();

xi_bsp_io_fwu_state_t xi_bsp_fwu_on_new_firmware_failure();

xi_bsp_io_fwu_state_t xi_bsp_fwu_on_firmware_package_download_failure();

xi_bsp_io_fwu_state_t xi_bsp_fwu_on_firmware_package_download_finished(
    const char* const firmware_resource_name );


void xi_bsp_fwu_checksum_init( void** checksum_context );

void xi_bsp_fwu_checksum_update( void* checksum_context,
                                 const uint8_t* data,
                                 uint32_t len );

void xi_bsp_fwu_checksum_final( void* checksum_context,
                                uint8_t** buffer_out,
                                uint16_t* buffer_len_out );

#ifdef __cplusplus
}
#endif

#endif /* __XI_BSP_FWU_H__ */
