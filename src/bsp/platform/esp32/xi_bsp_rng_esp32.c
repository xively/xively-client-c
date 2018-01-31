/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <wolfssl/wolfcrypt/types.h>
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

#if defined( XI_TLS_LIB_WOLFSSL )
/* WolfSSL API. This function is set via CUSTOM_RAND_GENERATE_SEED in the makefile */
int xi_bsp_rng_generate_wolfssl_seed( byte* output, word32 sz )
{
    word32 i;
    for (i = 0; i < sz; i++ )
    {
        output[i] = ( byte )xi_bsp_rng_get();
    }

    return 0;
}
#endif /* XI_TLS_LIB_WOLFSSL */
