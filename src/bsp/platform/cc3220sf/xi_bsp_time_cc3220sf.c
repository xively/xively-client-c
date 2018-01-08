/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_time.h>
#include <xi_bsp_time_cc3220sf_sntp.h>
#include <stdint.h>
#include <stdio.h>

#include <hw_types.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>

Clock_Struct clk0Struct;
void clockSecondTick( UArg arg0 );


void xi_bsp_time_init()
{
    Clock_Params clkParams;

    Clock_Params_init( &clkParams );
    clkParams.period    = 1000; // 1 second
    clkParams.startFlag = TRUE;

    /* Construct a periodic Clock Instance */
    Clock_construct( &clk0Struct, ( Clock_FuncPtr )clockSecondTick, clkParams.period,
                     &clkParams );

    xi_bsp_time_sntp_init( NULL );
}


/* tracks uptime */
void clockSecondTick( UArg arg0 )
{
    ++uptime;
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
