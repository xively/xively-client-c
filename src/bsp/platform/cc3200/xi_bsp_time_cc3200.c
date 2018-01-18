/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

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

static void timer_int_handler()
{
    MAP_TimerIntClear( TIMERA0_BASE, MAP_TimerIntStatus( TIMERA0_BASE, true ) );
    uptime++;
}

void xi_bsp_time_init()
{
    Timer_IF_Init( PRCM_TIMERA0, TIMERA0_BASE, TIMER_CFG_PERIODIC, TIMER_A, 0 );
    Timer_IF_IntSetup( TIMERA0_BASE, TIMER_A, timer_int_handler );
    Timer_IF_Start( TIMERA0_BASE, TIMER_A, 1000 );

    xi_bsp_time_sntp_init( NULL );
}

xi_time_t xi_bsp_time_getcurrenttime_seconds()
{
    return xi_bsp_time_sntp_getseconds_posix();
}

xi_time_t xi_bsp_time_getcurrenttime_milliseconds()
{
    /* note this returns seconds and not milliseconds since milliseconds from EPOCH
       do not fit into 32 bits */
    return xi_bsp_time_sntp_getseconds_posix();
}
