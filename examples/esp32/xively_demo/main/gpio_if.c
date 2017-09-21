#include <assert.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "gpio_if.h"

#define IO_BUTTON_DEBOUNCE_TIME 300

#define IO_BUTTON_PIN GPIO_NUM_0
#define IO_BUTTON_INTERRUPT_EDGE GPIO_PIN_INTR_NEGEDGE /* pull-up enabled in io_init */
//#define IO_BUTTON_INTERRUPT_EDGE GPIO_PIN_INTR_ANYEDGE

/**
 * This queue is appended a new element every time the state of the button pin
 * changes (pressed or released). The value appended is the GPIO pin number that
 * caused the interrupt (IO_BUTTON_PIN).
 * One of the RTOS tasks will have to periodically (or permanently) check this
 * queue for new interrupts, and act accordingly.
 * You can implement other queues to handle other interrupts from an RTOS task
 * Queue is NULL until initialized by io_init()
 */
static xQueueHandle io_button_queue = NULL;

void IRAM_ATTR gpio_isr_handler( void* arg )
{
    uint32_t gpio_num = ( uint32_t )arg;
    xQueueSendFromISR( io_button_queue, &gpio_num, NULL );
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

/**
 * @brief  Initialize the GPIO pins we'd like to use
 * @param  None
 * @retval -1 error, 0 success
 */
int8_t io_init( void )
{
    io_inputs_init();
    io_outputs_init();
    /* create a queue to handle gpio event from isr */
    io_button_queue = xQueueCreate( 10, sizeof( uint32_t ) );

    /* Enable the button interrupt */
    io_interrupts_enable();

    return 0;
}

/**
 * @retval -1 Error
 * @retval [0, 1]: Button status changed. retval is the GPIO input level
 */
int8_t io_await_gpio_interrupt( uint32_t timeout_ms )
{
    uint32_t queue_value_io_num;

    /* Wait for button interrupts for up to queue_recv_timeout_ms milliseconds */
    if ( xQueueReceive( io_button_queue, &queue_value_io_num,
                        ( timeout_ms / portTICK_PERIOD_MS ) ) )
    {
        return gpio_get_level( queue_value_io_num );
    }
    return -1;
}

/**
 * @brief Add the button GPIO pin to the list of ISR handlers 
 */
void io_interrupts_enable( void )
{
    /* hook isr handler for specific gpio pin */
    gpio_isr_handler_add( IO_BUTTON_PIN, gpio_isr_handler, ( void* )IO_BUTTON_PIN );
}

/**
 * @brief Remove the button GPIO pin to the list of ISR handlers 
 */
void io_interrupts_disable( void )
{
    /* hook isr handler for specific gpio pin */
    gpio_isr_handler_remove( IO_BUTTON_PIN );
}
