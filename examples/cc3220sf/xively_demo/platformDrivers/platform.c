/*
 *   Copyright (C) 2015-2016 Texas Instruments Incorporated
 *
 *   All rights reserved. Property of Texas Instruments Incorporated.
 *   Restricted rights to use, duplicate or disclose this code are
 *   granted through contract.
 *
 *   The program may not be used without the written permission of
 *   Texas Instruments Incorporated or against the terms and conditions
 *   stipulated in the agreement under which this program has been supplied,
 *   and under no circumstances can it be used with non-TI connectivity device.
 *
 */


//*****************************************************************************
//
//! \addtogroup xively_example
//! @{
//
//*****************************************************************************


/* driverlib Header files */
#include <ti/devices/cc32xx/inc/hw_ints.h>
#include <ti/devices/cc32xx/inc/hw_types.h>
#include <ti/devices/cc32xx/inc/hw_memmap.h>
#include <ti/devices/cc32xx/driverlib/prcm.h>

/* POSIX Header files */
#include <pthread.h>
#include <time.h>

/* Example/Board Header files */
#include <platformDrivers/platform.h>
#include <board.h>


extern void *InitTerm();

//****************************************************************************
//                            MAIN FUNCTION
//****************************************************************************

//*****************************************************************************
//
//! \brief This function reboot the M4 host processor
//!
//! \param[in]  none
//!
//! \return none
//!
//****************************************************************************
void cc3220Reboot(void)
{
    /* stop network processor activities before reseting the MCU */
    sl_Stop(Board_SL_STOP_TIMEOUT);

    UART_PRINT("[Common] CC3220 MCU reset request\r\n");

    /* Reset the MCU in order to test the bundle */
    PRCMHibernateCycleTrigger();
}


void Platform_TimerInit(void (*timerIntHandler)(sigval val), timer_t *timerId)
{
    sigevent sev;

    /* Create Timer */
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_notify_function = timerIntHandler;
    timer_create(CLOCK_MONOTONIC, &sev, timerId);

}

void Platform_TimerStart(_u32 asyncEvtTimeoutMsec, timer_t timerId, _u8 periodic)
{
    struct itimerspec value;

    /* set the timeout */
    value.it_value.tv_sec = (asyncEvtTimeoutMsec / 1000);
    value.it_value.tv_nsec = (asyncEvtTimeoutMsec % 1000) * 1000000;

    if (periodic)
    {
        /* set as periodic timer */
        value.it_interval.tv_sec = value.it_value.tv_sec;
        value.it_interval.tv_nsec = value.it_value.tv_nsec;
    }
    else
    {
        /* set as one shot timer */
        value.it_interval.tv_sec = 0;
        value.it_interval.tv_nsec = 0;
    }


    /* kick the timer */
    timer_settime(timerId, 0, &value, NULL);

}

void Platform_TimerStop(timer_t timerId)
{
    struct itimerspec value;

    /* stop timer */
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_nsec = 0;
    value.it_value.tv_sec = 0;
    value.it_value.tv_nsec = 0;
    timer_settime(timerId, 0, &value, NULL);

}

void Platform_TimerInterruptClear()
{
    /* Do nothing... */
}
