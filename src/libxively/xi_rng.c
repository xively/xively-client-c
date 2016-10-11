// Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
//
// This is part of the Xively C Client library,
// it is licensed under the BSD 3-Clause license.

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
