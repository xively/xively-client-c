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
extern ntp_time_t sntp_time_ntp( void ); // Seconds since 1900/01/01 00:00:00

/**
 * @function
 * @brief Returns seconds since 1970/01/01 00:00:00
 */
extern posix_time_t sntp_time_posix( void ); // Seconds since 1970/01/01 00:00:00

/**
 * @function
 * @brief Acquires date/time from NTP service.
 *
 * @param pvParameters ?
 *
 * Cycles through multiple NTP servers until date/time is acquired.
 */
void sntp_task( void* pvParameters );

static uint32_t start_time_ntp = 0;
static uint32_t uptime         = 0;

#endif /* __XI_BSP_TIME_CC3200_SNTP_H__ */
