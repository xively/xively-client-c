/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_BSP_TIME_CC3200_SNTP_H__
#define __XI_BSP_TIME_CC3200_SNTP_H__

#include <stdint.h>
#include "apps/sntp/sntp.h"

typedef int32_t posix_time_t;
typedef uint32_t ntp_time_t;

#define XI_ESP32_GET_TIME_FROM_RTC( t_ptr ) time( t_ptr )

/**
 * @function
 * @brief Acquires date/time from NTP service.
 *
 * Cycles through multiple NTP servers until date/time is acquired.
 */
int xi_bsp_time_sntp_init( void );

#endif /* __XI_BSP_TIME_CC3200_SNTP_H__ */
