/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_time.h>

#ifdef STM32F207xx
#include <stm32f2xx_hal.h>
#else
#include <stm32f4xx_hal.h>
#endif
#include <xi_bsp_time_stm32fx_sntp.h>

void xi_bsp_time_init()
{
    xi_bsp_time_sntp_init( NULL );
}

xi_time_t xi_bsp_time_getcurrenttime_seconds()
{
    return xi_bsp_time_sntp_getseconds_posix() + HAL_GetTick() / 1000;
}

xi_time_t xi_bsp_time_getcurrenttime_milliseconds()
{
    return xi_bsp_time_sntp_getseconds_posix() + HAL_GetTick() / 1000;
}
