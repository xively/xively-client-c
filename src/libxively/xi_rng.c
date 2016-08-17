#include "xi_rng.h"

#include <stdlib.h>

#ifdef XI_BSP
#include "xi_bsp_rng.h"
#endif

uint32_t xi_rand()
{
#ifdef XI_BSP
    return xi_bsp_rng_get();
#else
    int r = rand();
    return r;
#endif
}
