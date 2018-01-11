/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <stdint.h>
#include <xi_bsp_fwu.h>
#include <xi_bsp_mem.h>
#include <sha256.h>

static uint8_t checksum_cryptoalgs_sha256[SHA256_BLOCK_SIZE] = {0};

void xi_bsp_fwu_checksum_init( void** sha_ctx )
{
    *sha_ctx = xi_bsp_mem_alloc( sizeof( SHA256_CTX ) );

    sha256_init( ( SHA256_CTX* )*sha_ctx );
}

void xi_bsp_fwu_checksum_update( void* sha_ctx, const uint8_t* data, uint32_t len )
{
    sha256_update( ( SHA256_CTX* )sha_ctx, data, len );
}

void xi_bsp_fwu_checksum_final( void** sha_ctx,
                                uint8_t** buffer_out,
                                uint16_t* buffer_len_out )
{
    sha256_final( ( SHA256_CTX* )*sha_ctx, checksum_cryptoalgs_sha256 );

    *buffer_out     = checksum_cryptoalgs_sha256;
    *buffer_len_out = sizeof( checksum_cryptoalgs_sha256 );

    xi_bsp_mem_free( *sha_ctx );
    *sha_ctx = NULL;
}
