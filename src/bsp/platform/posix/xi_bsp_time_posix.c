/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#include <xi_bsp_time.h>

#include <stddef.h>
#include <sys/time.h>

xi_time_t xi_bsp_time_getcurrenttime_milliseconds()
{
    struct timeval current_time;
    gettimeofday( &current_time, NULL );
    return ( xi_time_t )( ( current_time.tv_sec * 1000 ) +
                          ( current_time.tv_usec + 500 ) /
                              1000 ); /* round the microseconds to milliseconds */
}
