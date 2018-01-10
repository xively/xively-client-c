/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client codebase,
 * it is licensed under the BSD 3-Clause license.
 */
#include <assert.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "gpio_if.h"

#define IO_BUTTON_DEBOUNCE_TIME 150
#define IO_BUTTON_INTERRUPT_EDGE GPIO_PIN_INTR_NEGEDGE /* pull-up enabled in io_init */

/**
 * This queue is appended a new element every time the state of the button pin
 * changes (pressed or released). The value appended is the GPIO pin number that
 * caused the interrupt (IO_BUTTON_PIN).
 * One of the RTOS tasks will have to periodically (or continuously) check this
 * queue for new interrupts, and act accordingly.
 * You can implement other queues to handle other interrupts from an RTOS task.
 * Queue is NULL until initialized by io_init()
 */
static xQueueHandle io_button_queue = NULL;

/**
 * @brief  Identify whether the interrupt was a real button press or a bounce.
 *         All interrupts within a IO_BUTTON_DEBOUNCE_TIME window after the
 *         first one will be considered bounces
 * @retval  0 if the interrupt was caused by a button bounce and should be ignored
 * @retval  1 if the interrupt was a legitimate button press
 */
static inline IRAM_ATTR int8_t gpio_button_isr_debouncer( void )
{
    static uint32_t button_press_systick = 0;
    const uint32_t current_systick = xTaskGetTickCountFromISR();

    if ( ( current_systick - button_press_systick ) * portTICK_PERIOD_MS <
         IO_BUTTON_DEBOUNCE_TIME )
    {
        return 0;
    }
    button_press_systick = current_systick;
    return 1;
}

void IRAM_ATTR gpio_isr_handler( void* arg )
{
    const uint32_t gpio_num = ( uint32_t )arg;

    if( NULL != io_button_queue )
    {
        if( gpio_button_isr_debouncer() )
        {
            xQueueSendFromISR( io_button_queue, &gpio_num, NULL );
        }
    }
}

inline void io_inputs_init( void )
{
    gpio_config_t io_conf;

    /* interrupt on falling edge */
    io_conf.intr_type = IO_BUTTON_INTERRUPT_EDGE;
    /* bit mask of the pins */
    io_conf.pin_bit_mask = ( 1 << IO_BUTTON_PIN );
    /* set as input mode */
    io_conf.mode = GPIO_MODE_INPUT;
    /* enable pull-up mode */
    io_conf.pull_up_en = 1;

    gpio_config( &io_conf );

    /* install gpio isr service */
    gpio_install_isr_service( 0 ); /* 0 is used by the SDK's gpio_example_main.c */
}

inline void io_outputs_init( void )
{
    gpio_config_t io_conf;

    /* interrupt on falling edge */
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    /* bit mask of the pins */
    io_conf.pin_bit_mask = ( 1 << IO_LED_PIN );
    /* set as input mode */
    io_conf.mode = GPIO_MODE_OUTPUT;
    /* disable pull-up mode */
    io_conf.pull_up_en = 0;
    /* disable pull-down mode */
    io_conf.pull_down_en = 0;

    gpio_config( &io_conf );
}

int8_t io_init( void )
{
    io_inputs_init();
    io_outputs_init();
    /* create a queue to handle gpio event from isr */
    io_button_queue = xQueueCreate( 10, sizeof( uint32_t ) );

    if ( NULL == io_button_queue )
    {
        printf( "\n[ERROR] Failed to create FreeRTOS queue for button interrupts" );
        return -1;
    }

    /* Enable the button interrupt */
    if ( 0 > io_interrupts_enable() )
    {
        return -1;
    }

    return 0;
}

/* Returns 0 if there's an interrupt in the queue, -1 if the queue is empty */
int8_t io_pop_gpio_interrupt( void )
{
    uint32_t queue_value_io_num;

    /* Wait for button interrupts for up to queue_recv_timeout_ms milliseconds */
    if ( xQueueReceive( io_button_queue, &queue_value_io_num, 0 ) )
    {
        return 0;
    }
    return -1;
}

int8_t io_interrupts_enable( void )
{
    esp_err_t r = ESP_OK;
    r = gpio_isr_handler_add( IO_BUTTON_PIN, gpio_isr_handler, ( void* )IO_BUTTON_PIN );
    /* hook isr handler for specific gpio pin */
    if ( ESP_OK != r )
    {
        printf( "\n[ERROR] adding GPIO ISR handler: [%d]. Interrupts NOT enabled", r );
        return -1;
    }
    return 0;
}

int8_t io_interrupts_disable( void )
{
    esp_err_t retval = ESP_OK;
    retval = gpio_isr_handler_remove( IO_BUTTON_PIN );
    /* hook isr handler for specific gpio pin */
    if ( ESP_OK != retval )
    {
        printf( "\n[ERROR] removing GPIO ISR handler: [%d]", retval );
        return -1;
    }
    return 0;
}
