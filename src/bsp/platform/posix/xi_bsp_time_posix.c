/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_time.h>

#include <stddef.h>
#include <sys/time.h>

void xi_bsp_time_init()
{
    /* empty */
}

xi_time_t xi_bsp_time_getcurrenttime_seconds()
{
    struct timeval current_time;
    gettimeofday( &current_time, NULL );
    return ( xi_time_t )( ( current_time.tv_sec ) +
                          ( current_time.tv_usec + 500000 ) /
                              1000000 ); /* round the microseconds to seconds */
}

xi_time_t xi_bsp_time_getcurrenttime_milliseconds()
{
    struct timeval current_time;
    gettimeofday( &current_time, NULL );
    return ( xi_time_t )( ( current_time.tv_sec * 1000 ) +
                          ( current_time.tv_usec + 500 ) /
                              1000 ); /* round the microseconds to milliseconds */
}
