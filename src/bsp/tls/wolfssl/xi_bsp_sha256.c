/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_sha256.h>
#include <wolfssl/wolfcrypt/sha256.h>

int xi_bsp_sha256_init( xi_bsp_sha256_t* sha )
{
    return wc_InitSha256( ( Sha256* )sha );
}

int xi_bsp_sha256_update( xi_bsp_sha256_t* sha, const uint8_t* data, uint32_t len )
{
    return wc_Sha256Update( ( Sha256* )sha, data, len );
}

int xi_bsp_sha256_final( xi_bsp_sha256_t* sha, uint8_t* out )
{
    return wc_Sha256Final( ( Sha256* )sha, out );
}
