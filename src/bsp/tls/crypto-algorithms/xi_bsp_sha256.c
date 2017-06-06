/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_crypt.h>
#include <xi_bsp_mem.h>
#include <sha256.h>

int xi_bsp_crypt_sha256_init( void** sha_ctx )
{
    *sha_ctx = xi_bsp_mem_alloc( sizeof( SHA256_CTX ) );

    sha256_init( ( SHA256_CTX* )*sha_ctx );

    return 0;
}

int xi_bsp_crypt_sha256_update( void* sha_ctx, const uint8_t* data, uint32_t len )
{
    sha256_update( ( SHA256_CTX* )sha_ctx, data, len );

    return 0;
}

int xi_bsp_crypt_sha256_final( void* sha_ctx, uint8_t* out )
{
    sha256_final( ( SHA256_CTX* )sha_ctx, out );

    xi_bsp_mem_free( sha_ctx );

    return 0;
}
