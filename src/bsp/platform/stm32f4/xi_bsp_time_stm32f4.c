/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_time.h>

#include <stm32f4xx_hal.h>
#include <sntp.h>

void xi_bsp_time_init()
{
    sntp_init();
}

xi_time_t xi_bsp_time_getcurrenttime_seconds()
{
    return sntp_time_posix() + HAL_GetTick() / 1000;
}

xi_time_t xi_bsp_time_getcurrenttime_milliseconds()
{
    return HAL_GetTick();
}
