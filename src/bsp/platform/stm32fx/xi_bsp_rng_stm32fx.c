/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_rng.h>
#include <xi_bsp_debug.h>

#if !defined( XI_TLS_LIB_WOLFSSL )

#ifdef STM32F207xx
#include <stm32f2xx_hal.h>
#include "stm32f2xx_hal_rng.h"
#else
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_rng.h"
#endif

RNG_HandleTypeDef xi_stm_rng_handle;

void xi_bsp_rng_init()
{
    // The next 2 lines copied from the STMCube-RNG Example
    /* ##-1- Configure the RNG peripheral ####################################### */
    xi_stm_rng_handle.Instance = RNG;

    const HAL_StatusTypeDef rng_status = HAL_RNG_Init( &xi_stm_rng_handle );
    if ( rng_status != HAL_OK )
    {
        xi_bsp_debug_format( "Can't initialize HAL RNG: %d", rng_status );
    }
}

uint32_t xi_bsp_rng_get()
{
    uint32_t random32 = 0;
    const HAL_StatusTypeDef rng_status =
        HAL_RNG_GenerateRandomNumber( &xi_stm_rng_handle, &random32 );
    if ( rng_status != HAL_OK )
    {
        xi_bsp_debug_format( "Can't obtain random number from HAL RNG: %d", rng_status );
    }
    return random32;
}

void xi_bsp_rng_shutdown()
{
    HAL_RNG_DeInit( &xi_stm_rng_handle );
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
