/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "esp_system.h"

void xi_bsp_rng_init()
{
    return;
}

uint32_t xi_bsp_rng_get()
{
    return esp_random();
}

void xi_bsp_rng_shutdown()
{
    return;
}
