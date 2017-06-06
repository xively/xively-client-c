/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_crypt.h>
#include <xi_bsp_mem.h>
#include <wolfssl/wolfcrypt/sha256.h>

int xi_bsp_crypt_sha256_init( void** sha_ctx )
{
    *sha_ctx = xi_bsp_mem_alloc( sizeof( Sha256 ) );

    return wc_InitSha256( ( Sha256* )*sha_ctx );
}

int xi_bsp_crypt_sha256_update( void* sha_ctx, const uint8_t* data, uint32_t len )
{
    return wc_Sha256Update( ( Sha256* )sha_ctx, data, len );
}

int xi_bsp_crypt_sha256_final( void* sha_ctx, uint8_t* out )
{
    const int sha256_result = wc_Sha256Final( ( Sha256* )sha_ctx, out );

    xi_bsp_mem_free( sha_ctx );

    return sha256_result;
}
