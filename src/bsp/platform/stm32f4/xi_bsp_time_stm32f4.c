/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_time.h>

#include <lwip/apps/sntp.h>
#include <FreeRTOS.h>
#include <task.h>

static xi_time_t timer = 0;

void xi_bsp_time_init()
{
    /* empty */
    // sntp_init();
}

xi_time_t xi_bsp_time_getcurrenttime_seconds()
{
    return xTaskGetTickCount() / 1000;
}

xi_time_t xi_bsp_time_getcurrenttime_milliseconds()
{
    return xTaskGetTickCount();
}
