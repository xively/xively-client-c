/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client codebase,
 * it is licensed under the BSD 3-Clause license.
 */
#ifndef __DEMO_IO_H__
#define __DEMO_IO_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "freertos/queue.h"

#define IO_LED_PIN GPIO_NUM_17

/* Nucleo Board */
int8_t io_init( void );

int8_t io_await_gpio_interrupt( uint32_t timeout_ms );
void io_interrupts_enable( void );
void io_interrupts_disable( void );

#define io_led_on( void ) gpio_set_level( IO_LED_PIN, 1 )
#define io_led_off( void ) gpio_set_level( IO_LED_PIN, 0 )
#define io_led_set( level ) gpio_set_level( IO_LED_PIN, level )

#ifdef __cplusplus
}
#endif
#endif /* __DEMO_IO_H__ */
