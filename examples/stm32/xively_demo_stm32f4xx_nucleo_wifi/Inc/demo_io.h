/**
  *******************************************************************************
  * @file    Projects/Multi/Examples/IKS01A2/DataLog/Inc/DemoSerial.h
  * @author  CL
  * @version V3.0.0
  * @date    12-August-2016
  * @brief   header for DemoSerial.c.
  *******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ********************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
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
