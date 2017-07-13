/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_bsp_time_esp32_sntp.h"
#include "xi_bsp_time.h"
#include "FreeRTOS.h"
#include "task.h"

void xi_bsp_time_init()
{
    return;
}

xi_time_t xi_bsp_time_getcurrenttime_seconds()
{
    return xi_bsp_time_sntp_getseconds_posix() + ( xi_time_t )xTaskGetTickCount() / 1000;
}

xi_time_t xi_bsp_time_getcurrenttime_milliseconds()
{
    return xi_bsp_time_sntp_getseconds_posix() + ( xi_time_t )xTaskGetTickCount() / 1000;
}
