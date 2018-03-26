#ifndef __DEMO_IO_H__
#define __DEMO_IO_H__

#include "sensor.h"

#define IO_FLOAT_BUFFER_MAX_SIZE     16
#define IO_AXES_JSON_BUFFER_MAX_SIZE 50

#define IO_NUCLEO_LED_PIN LED2
#if ( ( defined( USE_STM32F4XX_NUCLEO ) ) || \
      ( defined( USE_STM32L0XX_NUCLEO ) ) || \
      ( defined( USE_STM32L4XX_NUCLEO ) ) )
#define IO_NUCLEO_BUTTON_PIN KEY_BUTTON_PIN
#elif ( defined( USE_STM32L1XX_NUCLEO ) )
#define IO_NUCLEO_BUTTON_PIN USER_BUTTON_PIN
#endif
#define IO_BUTTON_DEBOUNCE_TIME 100

/* Nucleo Board */
int8_t io_nucleoboard_init( void );

int8_t io_button_exti_debouncer( uint16_t gpio_pin );
int8_t io_read_button( void );

#define io_led_on( void ) BSP_LED_On( IO_NUCLEO_LED_PIN )
#define io_led_off( void ) BSP_LED_Off( IO_NUCLEO_LED_PIN )
#define io_led_toggle( void ) BSP_LED_Toggle( IO_NUCLEO_LED_PIN )

/* Sensor Board */
int8_t io_sensorboard_init( void );
void io_sensorboard_enable( void );

int8_t io_read_gyro( SensorAxes_t* read_values );
int8_t io_read_accelero( SensorAxes_t* read_values );
int8_t io_read_magneto( SensorAxes_t* read_values );
int8_t io_read_pressure( float* read_value );
int8_t io_read_temperature( float* read_value );
int8_t io_read_humidity( float* read_value );

/* Helpers */
int8_t io_axes_to_json( SensorAxes_t axes, char* buf , int32_t buf_size );
int8_t io_float_to_string( float input, char* buf , int32_t buf_size );

#endif /* __DEMO_IO_H__ */
