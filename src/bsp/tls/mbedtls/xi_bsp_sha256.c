/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <stdint.h>
#include <xi_bsp_mem.h>
#include <mbedtls/sha256.h>

void xi_bsp_fwu_checksum_init( void** sha_ctx )
{
    *sha_ctx = xi_bsp_mem_alloc( sizeof( mbedtls_sha256_context ) );

    mbedtls_sha256_init( ( mbedtls_sha256_context* )*sha_ctx );
}

void xi_bsp_fwu_checksum_update( void* sha_ctx, const uint8_t* data, uint32_t len )
{
    mbedtls_sha256_starts( ( mbedtls_sha256_context* )sha_ctx, 0 );

    mbedtls_sha256_update( ( mbedtls_sha256_context* )sha_ctx, data, len );
}

void xi_bsp_fwu_checksum_final( void* sha_ctx, uint8_t* out )
{
    mbedtls_sha256_finish( ( mbedtls_sha256_context* )sha_ctx, out );

    xi_bsp_mem_free( sha_ctx );
}
