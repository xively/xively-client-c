/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_sha256.h>
#include <xi_bsp_mem.h>
#include <mbedtls/sha256.h>

int xi_bsp_sha256_init( void** sha )
{
    *sha = xi_bsp_mem_alloc( sizeof( mbedtls_sha256_context ) );

    mbedtls_sha256_init( ( mbedtls_sha256_context* )*sha );

    return 0;
}

int xi_bsp_sha256_update( void* sha, const uint8_t* data, uint32_t len )
{
    mbedtls_sha256_starts( ( mbedtls_sha256_context* )sha, 0 );

    mbedtls_sha256_update( ( mbedtls_sha256_context* )sha, data, len );

    return 0;
}

int xi_bsp_sha256_final( void* sha, uint8_t* out )
{
    mbedtls_sha256_finish( ( mbedtls_sha256_context* )sha, out );

    xi_bsp_mem_free( sha );

    return 0;
}
