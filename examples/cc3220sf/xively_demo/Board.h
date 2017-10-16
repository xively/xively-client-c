/*
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __BOARD_H
#define __BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "platformDrivers/CC3220SF_LAUNCHXL.h"

#define Board_initGeneral            CC3220SF_LAUNCHXL_initGeneral

/*
 * These macros are provided for backwards compatability.
 * Please use the <Driver>_init functions directly rather
 * than Board_init<Driver>.
 */
#define Board_initGPIO               GPIO_init

/* GPIO and I2C defines */
#define Board_BUTTON0                CC3220SF_LAUNCHXL_SW2
#define Board_BUTTON1                CC3220SF_LAUNCHXL_SW3
#define Board_LED_ON                 CC3220SF_LAUNCHXL_LED_ON
#define Board_LED_OFF                CC3220SF_LAUNCHXL_LED_OFF
#define Board_I2C_TMP                CC3220SF_LAUNCHXL_I2C0
#define Board_LED0                   CC3220SF_LAUNCHXL_LED_D7
#define Board_LED1                   CC3220SF_LAUNCHXL_LED_D6
#define Board_LED2                   CC3220SF_LAUNCHXL_LED_D5
#define Board_TMP006_ADDR            (0x41)
#define Board_SL_STOP_TIMEOUT        (200)
#define Board_UART0                  CC3220SF_LAUNCHXL_UART0

#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H */
