/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <stdint.h>
#include <xi_bsp_fwu.h>
#include <xi_bsp_mem.h>
#include <mbedtls/sha256.h>

static uint8_t checksum_mbedtls_sha256[32] = {0};

void xi_bsp_fwu_checksum_init( void** sha_ctx )
{
    *sha_ctx = xi_bsp_mem_alloc( sizeof( mbedtls_sha256_context ) );

    mbedtls_sha256_init( ( mbedtls_sha256_context* )*sha_ctx );

    mbedtls_sha256_starts( ( mbedtls_sha256_context* )*sha_ctx, 0 );
}

void xi_bsp_fwu_checksum_update( void* sha_ctx, const uint8_t* data, uint32_t len )
{
    mbedtls_sha256_update( ( mbedtls_sha256_context* )sha_ctx, data, len );
}

void xi_bsp_fwu_checksum_final( void** sha_ctx,
                                uint8_t** buffer_out,
                                uint16_t* buffer_len_out )
{
    mbedtls_sha256_finish( ( mbedtls_sha256_context* )*sha_ctx, checksum_mbedtls_sha256 );

    *buffer_out     = checksum_mbedtls_sha256;
    *buffer_len_out = sizeof( checksum_mbedtls_sha256 );

    xi_bsp_mem_free( *sha_ctx );
    *sha_ctx = NULL;
}
