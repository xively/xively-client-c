/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_rng.h>

#if !defined( XI_TLS_LIB_WOLFSSL )

#include "stm32f4xx_rng.h"

void xi_bsp_rng_init()
{
    /* Enable RNG clock source */
    RCC->AHB2ENR |= RCC_AHB2ENR_RNGEN;

    /* RNG Peripheral enable */
    RNG->CR |= RNG_CR_RNGEN;
}

uint32_t xi_bsp_rng_get()
{
    /* Wait until one RNG number is ready */
    while ( !( RNG->SR & ( RNG_SR_DRDY ) ) )
        ;

    /* Get a 32-bit Random number */
    return RNG->DR;
}

void xi_bsp_rng_shutdown()
{
    /* Disable RNG peripheral */
    RNG->CR &= ~RNG_CR_RNGEN;

    /* Disable RNG clock source */
    RCC->AHB2ENR &= ~RCC_AHB2ENR_RNGEN;
}

#elif defined( XI_TLS_LIB_WOLFSSL ) /* WOLFSSL version of RNG implementation */

#include <wolfssl/wolfcrypt/random.h>
#include <wolfssl/wolfcrypt/memory.h>
#include <xi_allocator.h>

static WC_RNG wolfcrypt_rng;

void xi_bsp_rng_init()
{
    /* check if already initialized */
    if ( NULL == wolfcrypt_rng.drbg )
    {
        wolfSSL_SetAllocators( xi_alloc_ptr, xi_free_ptr, xi_realloc_ptr );

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
