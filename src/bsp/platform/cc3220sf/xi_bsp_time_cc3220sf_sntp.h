/* Copyright (c) 2003-2018, Xively All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_BSP_TIME_CC3200_SNTP_H__
#define __XI_BSP_TIME_CC3200_SNTP_H__

#include <stdint.h>

/******************************************************************************
 *                                                                            *
 *  Definitions                                                               *
 *                                                                            *
 ******************************************************************************/

typedef int32_t posix_time_t;
typedef uint32_t ntp_time_t;


/******************************************************************************
 *                                                                            *
 * Externs                                                                    *
 *                                                                            *
 ******************************************************************************/

/**
 * @function
 * @brief Returns seconds since 1900/01/01 00:00:00
 */
extern ntp_time_t
xi_bsp_time_sntp_getseconds_ntp( void ); // Seconds since 1900/01/01 00:00:00

/**
 * @function
 * @brief Returns seconds since 1970/01/01 00:00:00
 */
extern posix_time_t
xi_bsp_time_sntp_getseconds_posix( void ); // Seconds since 1970/01/01 00:00:00

/**
 * @function
 * @brief Acquires date/time from NTP service.
 *
 * @param pvParameters ?
 *
 * Cycles through multiple NTP servers until date/time is acquired.
 */
void xi_bsp_time_sntp_init( void* pvParameters );

extern uint32_t uptime;

#endif /* __XI_BSP_TIME_CC3200_SNTP_H__ */
