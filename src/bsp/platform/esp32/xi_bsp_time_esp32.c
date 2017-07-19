/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */
#include "xi_bsp_time_esp32_sntp.h"
#include "xi_bsp_time.h"
#include <time.h> /* For the gmtime() function */

void xi_bsp_time_init()
{
    /* Keep iterating through the SNTP servers until we get a date and time */
    while ( 0 > xi_bsp_time_sntp_init() )
        ;
}

xi_time_t xi_bsp_time_getcurrenttime_seconds()
{
    time_t current_time = 0;
    XI_ESP32_GET_TIME_FROM_RTC( &current_time );
    return ( xi_time_t )current_time;
}

xi_time_t xi_bsp_time_getcurrenttime_milliseconds()
{
    return xi_bsp_time_getcurrenttime_seconds() * 1000;
}

/**
 * Function required by WolfSSL to track the current time.
 */
time_t XTIME( time_t* timer )
{
    time_t current_time = xi_bsp_time_getcurrenttime_seconds();
    if ( timer )
    {
        *timer = current_time;
    }
    return current_time;
}

/**
 * Function required by WolfSSL to track the current time.
 */
struct tm* XGMTIME( const time_t* timer, struct tm* tmp )
{
    ( void )tmp;
    return gmtime( timer );
}
