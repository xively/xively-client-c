/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#ifndef __XI_BSP_TIME_H__
#define __XI_BSP_TIME_H__

/**
 * @file xi_bsp_time.h
 * @brief Xively Client's Board Support Platform (BSP) for Time Functions
 *
 * This file defines the Time Functions API used by the Xively C Client.
 *
 * The Xively C Client determines time using these functions. These time functions
 * drive the exact execution time of scheduled tasks and the delayed reconnection
 * logic which prevents clients from overloading the Xively Server. Implementation
 * may use hardware or even NTP servers in order to maintain an accurate clock.
 */

#include <xively_time.h>

#ifdef __cplusplus
extern "C" {
#endif

void xi_bsp_time_init();

/**
 * @function
 * @brief Returns elapsed milliseconds since Epoch.
 */
xi_time_t xi_bsp_time_getcurrenttime_milliseconds();

#ifdef __cplusplus
}
#endif

#endif /* __XI_BSP_TIME_H__ */
