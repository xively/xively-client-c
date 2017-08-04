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

uint8_t xi_bsp_fwu_is_this_firmware( const char* const resource_name );

xi_state_t xi_bsp_fwu_on_new_firmware_ok();

xi_state_t xi_bsp_fwu_on_new_firmware_failure();

xi_state_t xi_bsp_fwu_on_firmware_package_download_finished(
    const char* const firmware_resource_name );


void xi_bsp_fwu_checksum_init( void** checksum_context );

void xi_bsp_fwu_checksum_update( void* checksum_context,
                                 const uint8_t* data,
                                 uint32_t len );

void xi_bsp_fwu_checksum_final( void* checksum_context, uint8_t* buffer_out );

#ifdef __cplusplus
}
#endif

#endif /* __XI_BSP_FWU_H__ */
