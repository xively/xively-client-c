/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#include <xi_bsp_time.h>
#include <stdint.h>
#include <stdio.h>

#include <hw_types.h>
#include <hw_memmap.h>
#include <rom.h>
#include <rom_map.h>
#include <prcm.h>
#include <timer_if.h>
#include <timer.h>

#include <xi_bsp_time_cc3200_sntp.h>

static void timer_int_handler( )
{
    MAP_TimerIntClear (TIMERA0_BASE, MAP_TimerIntStatus(TIMERA0_BASE, true));
    uptime ++;
}

void xi_bsp_time_init( )
{
    Timer_IF_Init     (PRCM_TIMERA0, TIMERA0_BASE, TIMER_CFG_PERIODIC, TIMER_A, 0);
    Timer_IF_IntSetup (TIMERA0_BASE, TIMER_A, timer_int_handler);
    Timer_IF_Start    (TIMERA0_BASE, TIMER_A, 1000);

    sntp_task( NULL );
}

xi_time_t xi_bsp_time_getcurrenttime_milliseconds()
{
    return start_time_ntp + uptime;
}
