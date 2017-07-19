/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */
#include "xi_bsp_time_esp32_sntp.h"
#include "xi_bsp_debug.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define DEVELOPMENT_DATETIME ( 1500312469UL )
#define XI_SNTP_MAX_RETRIES 10

#ifndef SNTP_MAX_SERVERS
  #error "SNTP_MAX_SERVERS must be set from the build system to get it in sntp.c and Xi's BSP"
#else
  #if SNTP_MAX_SERVERS!=4
    #error "SNTP_MAX_SERVERS mismatch between the compiler option and the source code"
  #endif /* SNTP_MAX_SERVERS!=4 */
#endif /* SNTP_MAX_SERVERS */

char* sntp_servers[SNTP_MAX_SERVERS] = {"pool.ntp.org", "time-a.nist.gov",
                                        "time-b.nist.gov", "time-c.nist.gov"};

static uint32_t sntp_retrieved_time = 0;

/* On success, the SDK's sntp.c will set accurate datetime in the ESP's RTC */
int xi_bsp_time_sntp_init( void )
{
    const int retry_count = 10;
    int retry             = 0;

    for ( uint8_t i = 0; i < SNTP_MAX_SERVERS; i++ )
    {
        sntp_setservername( i, sntp_servers[i] );
    }

    sntp_init();

    /* Wait for time to be set in the RTC module */
    while ( sntp_retrieved_time < DEVELOPMENT_DATETIME )
    {
        if( ++retry > retry_count )
        {
            return -1;
        }
        xi_bsp_debug_format( "Waiting for system time to be set... [%d/%d]", retry,
                             retry_count );
        vTaskDelay( 2000 / portTICK_PERIOD_MS );
        XI_ESP32_GET_TIME_FROM_RTC( ( time_t* )&sntp_retrieved_time );
    }
    xi_bsp_debug_format( "Current datetime successfully configured in the RTC: %d",
                         sntp_retrieved_time );
    return 0;
}
