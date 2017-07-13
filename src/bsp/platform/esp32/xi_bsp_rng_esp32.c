/* TODO: Do we need the #if defined( XI_TLS_LIB_WOLFSSL ) implementation from
 * the stm32fx BSP? */
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
