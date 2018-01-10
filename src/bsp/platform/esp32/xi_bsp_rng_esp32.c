/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

/* TODO: Do we need the #if defined( XI_TLS_LIB_WOLFSSL ) implementation from
 * stm32fx's BSP? Which RNG is best, wc_RNG_GenerateBlock() or esp_random()? */
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
