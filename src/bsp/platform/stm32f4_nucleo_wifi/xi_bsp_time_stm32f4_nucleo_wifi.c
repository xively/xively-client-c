/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_time.h>
#include "includes/xi_bsp_time_stm32f4_nucleo_wifi_sntp.h"

uint8_t* sntp_sock_id_ptr = NULL;

void xi_bsp_time_init()
{
    uint8_t sock_id = -1;
    posix_time_t epoch_time = 0;
    sntp_sock_id = &sock_id;
    while( xi_bsp_time_sntp_init(&sock_id, &epoch_time) < 0 )
    {
        printf("\r\n>>SNTP Failed");
        //TODO: 1. pass server as a parameter to sntp_init
        //TODO: 2. cycle through multiple servers until we succeed
    }
    printf("\r\n>>SNTP datetime update [OK] Epoch time: %ld", epoch_time);
}

xi_time_t xi_bsp_time_getcurrenttime_seconds()
{
    return xi_bsp_time_sntp_getseconds_posix() + HAL_GetTick() / 1000;
}

xi_time_t xi_bsp_time_getcurrenttime_milliseconds()
{
    return xi_bsp_time_sntp_getseconds_posix() + HAL_GetTick() / 1000;
}
