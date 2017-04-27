/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_crypt.h>
#include <xi_bsp_mem.h>
#include <sha256.h>

int xi_bsp_crypt_sha256_init( void** sha )
{
    *sha = xi_bsp_mem_alloc( sizeof( SHA256_CTX ) );

    sha256_init( ( SHA256_CTX* )*sha );

    return 0;
}

int xi_bsp_crypt_sha256_update( void* sha, const uint8_t* data, uint32_t len )
{
    sha256_update( ( SHA256_CTX* )sha, data, len );

    return 0;
}

int xi_bsp_crypt_sha256_final( void* sha, uint8_t* out )
{
    sha256_final( ( SHA256_CTX* )sha, out );

    xi_bsp_mem_free( sha );

    return 0;
}
