/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */
#include <stdint.h>

#include "xi_bsp_time.h"
#include "xi_bsp_debug.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "apps/sntp/sntp.h"

#define XI_ESP32_GET_TIME_FROM_RTC( t_ptr ) time( t_ptr )

#define DEVELOPMENT_DATETIME ( ( xi_time_t )1500312469UL )
#define XI_SNTP_TIMEOUT_MS 1000000

#ifndef SNTP_MAX_SERVERS
#error "SNTP_MAX_SERVERS must be set from sdkconfig"
#endif
#if SNTP_MAX_SERVERS != 4
#error "SNTP_MAX_SERVERS mismatch between the compiler option and the xi BSP"
#endif

char* sntp_servers[SNTP_MAX_SERVERS] = {"pool.ntp.org", "time-a.nist.gov",
                                        "time-b.nist.gov", "time-c.nist.gov"};

void xi_bsp_time_init()
{
    const int32_t sntp_timeout_step_ms = 100; /* ms */
    int32_t sntp_task_timeout          = XI_SNTP_TIMEOUT_MS;
    xi_time_t sntp_retrieved_time      = 0;

    for ( uint8_t i = 0; i < SNTP_MAX_SERVERS; i++ )
    {
        sntp_setservername( i, sntp_servers[i] );
    }

    sntp_setoperatingmode( SNTP_OPMODE_POLL );
    sntp_init();

    xi_bsp_debug_format( "Awaiting SNTP response for %d ms", sntp_task_timeout );
    while ( sntp_retrieved_time < DEVELOPMENT_DATETIME )
    {
        if ( 0 > ( sntp_task_timeout -= sntp_timeout_step_ms ) )
        {
            xi_bsp_debug_logger( "Error: SNTP timed out! TLS validation will fail" );
            goto exit;
        }
        vTaskDelay( sntp_timeout_step_ms / portTICK_PERIOD_MS );
        XI_ESP32_GET_TIME_FROM_RTC( &sntp_retrieved_time );
    }

    xi_bsp_debug_format( "RTC updated to current datetime: %d",
                         ( int32_t )sntp_retrieved_time );
exit:
    sntp_stop();
}

xi_time_t xi_bsp_time_getcurrenttime_seconds()
{
    xi_time_t current_time = 0;
    XI_ESP32_GET_TIME_FROM_RTC( &current_time );
    return current_time;
}

xi_time_t xi_bsp_time_getcurrenttime_milliseconds()
{
    /* note this returns seconds and not milliseconds since milliseconds from EPOCH
       do not fit into the variable type */
    return xi_bsp_time_getcurrenttime_seconds();
}
