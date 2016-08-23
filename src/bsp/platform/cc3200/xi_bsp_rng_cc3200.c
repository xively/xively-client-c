/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#include <xi_bsp_rng.h>

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
