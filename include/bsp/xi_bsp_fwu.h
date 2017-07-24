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

uint8_t xi_bsp_fwu_is_firmware( const char* const resource_name );

xi_state_t xi_bsp_fwu_commit();

xi_state_t xi_bsp_fwu_test();

xi_state_t xi_bsp_fwu_reboot();

#ifdef __cplusplus
}
#endif

#endif /* __XI_BSP_FWU_H__ */
