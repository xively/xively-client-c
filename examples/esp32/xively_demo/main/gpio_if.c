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

xQueueHandle io_button_queue = NULL;

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

/**
 * @brief  Read the status of the Nucleo board's button. The pin is normally
 *         pulled up to 1, and and becomes 0 when pressed.
 *         This function negates the electrical value of the pin, and returns a
 *         1 for pressed and a 0 for not pressed.
 *         Call it after io_nucleoboard_init()
 * @param  None
 * @retval 0 Button NOT pressed, 0 success
 * @retval 1 Button pressed
 */
int8_t io_read_button( void )
{
    int8_t pressed = 0;
    ( gpio_get_level( IO_BUTTON_PIN ) == 1 ) ? ( pressed = 0 ) : ( pressed = 1 );
    return pressed;
}

/**
 * @brief  The user must implement their own HAL_GPIO_EXTI_Callback(uint16_t)
 *         interrupt handler and call this function from there. It will identify
 *         whether the interrupt came from the Nucleo board's button, and
 *         whether it was a real button press or a bounce.
 *         All interrupts within a 300ms window after the first one will be
 *         considered bounces
 * @param  GPIO_Pin the pin connected to EXTI line
 * @retval -1 if the interrupt was not caused by the Nucleo's button
 * @retval  0 if the interrupt was caused by a button bounce and should be ignored
 * @retval  1 if the interrupt was a legitimate button press
 */
int8_t io_interrupt_debouncer( uint32_t gpio_pin )
{
    static uint32_t button_press_systime = 0;
    uint32_t current_systime = 0;

    if ( IO_BUTTON_PIN != gpio_pin )
    {
        return -1;
    }

    current_systime = portTICK_RATE_MS * ( int32_t )xTaskGetTickCountFromISR();
    if( (current_systime - button_press_systime) < IO_BUTTON_DEBOUNCE_TIME )
    {
        return 0;
    }
    else
    {
        button_press_systime = current_systime;
        return 1;
    }
}
