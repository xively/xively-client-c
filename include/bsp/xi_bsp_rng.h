/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_BSP_RNG_H__
#define __XI_BSP_RNG_H__

/**
 * @file xi_bsp_rng.h
 * @brief Xively Client's Board Support Platform (BSP) for Random Number Generator
 *
 * This file defines the Random Number Generator API needed by the Xively C Client.
 *
 * The Xively C Client calls the init function before requesting any random number.
 * The xi_bsp_rng_get function's responsibility is to provide random number each time it
 * is called. The shutdown function will be called in case no further random numbers are
 * required by the library. This happens at library shutdown.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @function
 * @brief Initializes the platform specific RNG prerequisites.
 *
 * Called before requesting first random number. Implementations should perform all
 * initialization actions in this function (generate random seed, create any
 * structures, etc.) required by the platform specific random number generation.
 */
void xi_bsp_rng_init();

/**
 * @function
 * @brief Generates new 32bit random number.
 *
 * Called whenever a random number is required by the Xively C Client.
 *
 * @return 32bit random integer
 */
uint32_t xi_bsp_rng_get();

/**
 * @function
 * @brief Shuts down the platform specific RNG mechanism.
 *
 * Called when no futher random number is required by the Xively Client. Takes place
 * during the shut down process of the Xively Client itself.
 */
void xi_bsp_rng_shutdown();

#ifdef __cplusplus
}
#endif

#endif /* __XI_BSP_RNG_H__ */
