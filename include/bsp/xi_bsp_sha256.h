/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_BSP_SHA256_H__
#define __XI_BSP_SHA256_H__

#include <stdint.h>

/* SHA256 Xively BSP API is based on wolfSSL's API */

enum
{
    // SHA256              =  2,   /* hash type unique */
    XI_SHA256_BLOCK_SIZE  = 64,
    XI_SHA256_DIGEST_SIZE = 32,
    // SHA256_PAD_SIZE     = 56
};

/* Sha256 digest */
typedef struct xi_sha256_s
{
    uint32_t buffLen; /* in bytes          */
    uint32_t loLen;   /* length in bytes   */
    uint32_t hiLen;   /* length in bytes   */
    uint32_t digest[XI_SHA256_DIGEST_SIZE / sizeof( uint32_t )];
    uint32_t buffer[XI_SHA256_BLOCK_SIZE / sizeof( uint32_t )];
} xi_sha256_t;

int xi_bsp_sha256_init( xi_sha256_t* );
int xi_bsp_sha256_update( xi_sha256_t*, const uint8_t*, uint32_t );
int xi_bsp_sha256_final( xi_sha256_t*, uint8_t* );

#endif // __XI_BSP_SHA256_H__
