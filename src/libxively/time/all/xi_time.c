/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_time.h"

#include <limits.h>

#ifdef XIVELY_MICROCHIP_SUPPORT
#include <time.h>
#endif

#ifdef XI_PLATFORM_BASE_EWARM
#include <time.h>
#endif

#ifdef XI_PLATFORM_BASE_WMSDK
#include <wmtime.h>
#endif

#ifdef XI_PLATFORM_BASE_POSIX
#include <time.h>
#include <sys/time.h>
#endif

#ifdef XI_PLATFORM_BASE_WMSDK
#include <wm_os.h>
#endif
#if defined( XI_PLATFORM_BASE_FREERTOS ) || defined( XI_PLATFORM_BASE_EWARM )
#include "FreeRTOS.h"
#include "task.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


xi_time_t xi_getcurrenttime_seconds()
{
#ifdef XIVELY_MICROCHIP_SUPPORT
    return ( long )TickConvertToMilliseconds( TickGet() ) / 1000;
#elif defined( XI_PLATFORM_BASE_WMSDK ) || defined( XI_PLATFORM_BASE_FREERTOS ) ||       \
    defined( XI_PLATFORM_BASE_EWARM )
    return ( long )xTaskGetTickCount() / configTICK_RATE_HZ;
#elif defined( XI_PLATFORM_BASE_POSIX )
    /* assume linux for now. */
    struct timeval current_time;
    gettimeofday( &current_time, NULL );
    return ( long )( current_time.tv_sec +
                     ( current_time.tv_usec + 500000 ) /
                         1000000 ); /* round the microseconds to seconds */
#elif defined( XI_PLATFORM_BASE_DUMMY )
    return 1;
#else
#error There is no implementation of xi_getcurrenttime_seconds() for chosen platform
#endif
}

xi_time_t xi_getcurrenttime_milliseconds()
{
#ifdef XIVELY_MICROCHIP_SUPPORT
    return ( long )TickConvertToMilliseconds( TickGet() );
#elif defined( XI_PLATFORM_BASE_WMSDK ) || defined( XI_PLATFORM_BASE_FREERTOS ) ||       \
    defined( XI_PLATFORM_BASE_EWARM )
    return ( long )xTaskGetTickCount() * portTICK_RATE_MS;
#elif defined( XI_PLATFORM_BASE_POSIX )
    /* assume linux for now */
    struct timeval current_time;
    gettimeofday( &current_time, NULL );
    return ( long )( ( current_time.tv_sec * 1000 ) +
                     ( current_time.tv_usec + 500 ) /
                         1000 ); /* round the microseconds to milliseconds */
#elif defined( XI_PLATFORM_BASE_DUMMY )
    return 1;
#else
#error There is no implementation of xi_getcurrenttime_milliseconds() for chosen platform
#endif
}


#ifdef __cplusplus
}
#endif
