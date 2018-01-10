/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_rng.h>
#include <stdlib.h>
#include <xi_bsp_time.h>

void xi_bsp_rng_init()
{
    srand( xi_bsp_time_getcurrenttime_seconds() );
}

uint32_t xi_bsp_rng_get()
{
    return rand();
}

void xi_bsp_rng_shutdown()
{
    /* nothing to do here */
}
