#ifndef __DEMO_IO_H__
#define __DEMO_IO_H__

#include <stdint.h>

#include "freertos/queue.h"
/**
 * This queue is appended a new element every time the state of the button pin
 * changes (pressed or released). The value appended is the GPIO pin number that
 * caused the interrupt (IO_BUTTON_PIN).
 * One of the RTOS tasks will have to periodically (or permanently) check this
 * queue for new interrupts, and act accordingly.
 * You can also modify the
 * You can implement other queues to handle other interrupts from an RTOS task
 * Queue is NULL until initialized by io_init()
 */
extern xQueueHandle io_button_queue;

/* Nucleo Board */
int8_t io_init( void );

int8_t io_interrupt_debouncer( uint32_t gpio_pin );
int8_t io_read_button( void );

void io_interrupts_enable( void );
void io_interrupts_disable( void );

/*
#define io_led_on( void ) BSP_LED_On( IO_NUCLEO_LED_PIN )
#define io_led_off( void ) BSP_LED_Off( IO_NUCLEO_LED_PIN )
#define io_led_toggle( void ) BSP_LED_Toggle( IO_NUCLEO_LED_PIN )
*/

#endif /* __DEMO_IO_H__ */
