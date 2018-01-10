/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_time.h>
#include <wmtime.h>
#include <wm_os.h>

void xi_bsp_time_init()
{
}

xi_time_t xi_bsp_time_getcurrenttime_seconds()
{
    return 1;
}

xi_time_t xi_bsp_time_getcurrenttime_milliseconds()
{
    return ( xi_time_t )xTaskGetTickCount() * portTICK_RATE_MS;
}
