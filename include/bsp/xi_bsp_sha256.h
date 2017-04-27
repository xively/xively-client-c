/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_BSP_SHA256_H__
#define __XI_BSP_SHA256_H__

#include <stdint.h>

int xi_bsp_sha256_init( void** );
int xi_bsp_sha256_update( void*, const uint8_t*, uint32_t );
int xi_bsp_sha256_final( void*, uint8_t* );

#endif // __XI_BSP_SHA256_H__
