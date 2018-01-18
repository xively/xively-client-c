//*****************************************************************************
// Copyright (C) 2016 Texas Instruments Incorporated
//
// All rights reserved. Property of Texas Instruments Incorporated.
// Restricted rights to use, duplicate or disclose this code are
// granted through contract.
// The program may not be used without the written permission of
// Texas Instruments Incorporated or against the terms and conditions
// stipulated in the agreement under which this program has been supplied,
// and under no circumstances can it be used with non-TI connectivity device.
//
//*****************************************************************************


#ifndef __PLATFORM_H
#define __PLATFORM_H

/* POSIX Header files */
#include <pthread.h>
#include <time.h>

#include <ti/drivers/net/wifi/simplelink.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UART_PRINT  Report

extern int Report(const char *pcFormat, ...);

void Platform_TimerInit(void (*timerIntHandler)(sigval val), timer_t *timerId);

void Platform_TimerStart(_u32 asyncEvtTimeoutMsec, timer_t timerId, _u8 periodic);

void Platform_TimerStop(timer_t timerId);

void Platform_TimerInterruptClear();

void cc3220Reboot(void);


#ifdef __cplusplus
}
#endif

#endif /* __PLATFORM_H */
