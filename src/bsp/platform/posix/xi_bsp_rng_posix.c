/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_rng.h>

/* No TLS option choosen */
#if !defined( XI_TLS_LIB_MBEDTLS ) && !defined( XI_TLS_LIB_WOLFSSL )

#include <stdlib.h>
#include <time.h>

void xi_bsp_rng_init()
{
    srand( time( NULL ) );
}

uint32_t xi_bsp_rng_get()
{
#ifdef XI_BSP_RNG_16BIT_INT_PLATFORM
    const uint32_t half_a = rand();
    const uint32_t half_b = rand();

    return ( half_a << 16 ) | ( half_b & 0x0000FFFF );
#else
    return rand();
#endif
}

void xi_bsp_rng_shutdown()
{
    /* nothing to do here */
}

#elif defined( XI_TLS_LIB_MBEDTLS ) /* MBEDTLS version of RNG implementation */

#include <mbedtls/config.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>

#include <stdio.h>

static mbedtls_entropy_context entropy;
static mbedtls_ctr_drbg_context ctr_drbg;

void xi_bsp_rng_init()
{
    const char personalization[] = "xi_bsp_mbedtls_more_entropy_pls";

    mbedtls_entropy_init( &entropy );
    mbedtls_ctr_drbg_init( &ctr_drbg );

    const int ret_state = mbedtls_ctr_drbg_seed(
        &ctr_drbg, mbedtls_entropy_func, &entropy,
        ( const unsigned char* )personalization, sizeof( personalization ) );

    if ( ret_state != 0 )
    {
        printf( " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret_state );
        goto exit;
    }

exit:;
}

uint32_t xi_bsp_rng_get()
{
    uint32_t random = 0;

    mbedtls_ctr_drbg_random( &ctr_drbg, ( unsigned char* )&random, 4 );

    return random;
}

void xi_bsp_rng_shutdown()
{
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );
}

#elif defined( XI_TLS_LIB_WOLFSSL ) /* WOLFSSL version of RNG implementation */

#include <cyassl/ctaocrypt/random.h>
#include <cyassl/ctaocrypt/memory.h>
#include <xi_allocator.h>

static WC_RNG wolfcrypt_rng;

void xi_bsp_rng_init()
{
    /* check if already initialized */
    if ( NULL == wolfcrypt_rng.drbg )
    {
        CyaSSL_SetAllocators( xi_alloc_ptr, xi_free_ptr, xi_realloc_ptr );

        wc_InitRng( &wolfcrypt_rng );
    }
}

uint32_t xi_bsp_rng_get()
{
    uint32_t random = 0;

    wc_RNG_GenerateBlock( &wolfcrypt_rng, ( byte* )&random, 4 );

    return random;
}

void xi_bsp_rng_shutdown()
{
    wc_FreeRng( &wolfcrypt_rng );
}

#endif
