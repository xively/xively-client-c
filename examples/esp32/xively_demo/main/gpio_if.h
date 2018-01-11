/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client codebase,
 * it is licensed under the BSD 3-Clause license.
 */
#ifndef __DEMO_IO_H__
#define __DEMO_IO_H__

#include <stdint.h>

#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IO_LED_PIN GPIO_NUM_17
#define IO_BUTTON_PIN GPIO_NUM_0

/**
 * @brief Initialize device's GPIO: LED output, Button Input interrupts and the
 * queue to notify the handler task of new interrupts
 *
 * @retval -1 Error
 * @retval  0 OK
 */
int8_t io_init( void );

/**
 * @brief When an interrupt occurs, the pin number is pushed into a task-safe queue.
 * This function returns 0 if there was an interrupt in the queue, -1 if it's empty.
 *
 * @retval -1 Queue is empty
 * @retval  0 Interrupt detected
 */
int8_t io_pop_gpio_interrupt( void );

/**
 * @brief Add the button GPIO pin to the list of ISR handlers 
 *
 * @retval -1 Error
 * @retval  0 OK
 */
int8_t io_interrupts_enable( void );

/**
 * @brief Remove the button GPIO pin from the list of ISR handlers 
 *
 * @retval -1 Error
 * @retval  0 OK
 */
int8_t io_interrupts_disable( void );

#define io_button_read( void ) gpio_get_level( IO_BUTTON_PIN )
#define io_led_on( void ) gpio_set_level( IO_LED_PIN, 1 )
#define io_led_off( void ) gpio_set_level( IO_LED_PIN, 0 )
#define io_led_set( level ) gpio_set_level( IO_LED_PIN, level )

#ifdef __cplusplus
}
#endif
#endif /* __DEMO_IO_H__ */
