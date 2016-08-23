/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#include <xi_bsp_time.h>
#include <stdint.h>

#include <FreeRTOS.h>
#include "task.h"

#define  FIXED_TIME  1462969757  /* Wed May 11 12:29:17 UTC 2016 */
#define  SECONDS_1900_TO_1970  2208988800

// static uint32_t artificial_time_base_seconds = FIXED_TIME + SECONDS_1900_TO_1970;
// static uint32_t artificial_time_base_milliseconds = 0;

xi_time_t xi_bsp_time_getcurrenttime_milliseconds()
{
    /*return 1000 * ( FIXED_TIME + SECONDS_1900_TO_1970 ) +
            ( artificial_time_base_milliseconds += 1000 );*/
    return ( long )xTaskGetTickCount();// * portTICK_RATE_MS;
}
