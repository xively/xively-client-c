#include <assert.h>
#include <string.h>
#include <math.h>

#include "x_nucleo_iks01a1.h"
#include "x_nucleo_iks01a1_accelero.h"
#include "x_nucleo_iks01a1_gyro.h"
#include "x_nucleo_iks01a1_humidity.h"
#include "x_nucleo_iks01a1_magneto.h"
#include "x_nucleo_iks01a1_pressure.h"
#include "x_nucleo_iks01a1_temperature.h"
#include "LSM6DS0_ACC_GYRO_driver_HL.h"

#include "main.h"
#include "demo_io.h"
#include "sensor.h"

static void
split_float_to_ints( float in, int32_t* out_int, int32_t* out_dec, int32_t dec_prec );

static void* ACCELERO_handle    = NULL;
static void* GYRO_handle        = NULL;
static void* MAGNETO_handle     = NULL;
static void* HUMIDITY_handle    = NULL;
static void* TEMPERATURE_handle = NULL;
static void* PRESSURE_handle    = NULL;

/******************************************************************************
*                        Nucleo Board IO Implementation                       *
******************************************************************************/
/**
 * @brief  Initialize the Button and LED on the Nucleo board
 * @param  None
 * @retval -1 error, 0 success
 */
int8_t io_nucleoboard_init( void )
{
    BSP_LED_Init( IO_NUCLEO_LED_PIN );
    io_led_off();

    BSP_PB_Init( IO_NUCLEO_BUTTON_PIN, BUTTON_MODE_EXTI );
    return 0;
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
    int32_t pin_state = BSP_PB_GetState( IO_NUCLEO_BUTTON_PIN );
    ( pin_state == 1 ) ? ( pin_state = 0 ) : ( pin_state = 1 );
    return ( int8_t )pin_state;
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
int8_t io_button_exti_debouncer( uint16_t gpio_pin )
{
    static uint32_t button_press_systick = 0;
    uint32_t current_systick = 0;

    if ( IO_NUCLEO_BUTTON_PIN != gpio_pin )
    {
        return -1;
    }

    current_systick = HAL_GetTick();
    if( (current_systick - button_press_systick) < IO_BUTTON_DEBOUNCE_TIME )
    {
        return 0;
    }
    else
    {
        button_press_systick = current_systick;
        return 1;
    }
}

/******************************************************************************
*                        Sensor Board IO Implementation                       *
******************************************************************************/

/**
 * @brief  Initialize all sensors on the sensor board
 * @param  None
 * @retval -1 error, 0 success
 * @TODO: This function differs for the IKS01A1 and IKS01A2 sensor boards.
 *        The current implementation was lifted from the IKS01A1 examples
 */
int8_t io_sensorboard_init( void )
{
    printf( "\r\n>> Initializing sensor board" );
    /* Try to use LSM6DS3 DIL24 if present, otherwise use LSM6DS0 on board */
    printf( "\r\n\tInitializing accelerometer" );
    if ( COMPONENT_OK != BSP_ACCELERO_Init( ACCELERO_SENSORS_AUTO, &ACCELERO_handle ) )
        return -1;
    /* Try to use LSM6DS3 if present, otherwise use LSM6DS0 */
    printf( "\r\n\tInitializing gyroscope" );
    if ( COMPONENT_OK != BSP_GYRO_Init( GYRO_SENSORS_AUTO, &GYRO_handle ) )
        return -1;
    /* Force to use LIS3MDL */
    printf( "\r\n\tInitializing magnetometer" );
    if ( COMPONENT_OK != BSP_MAGNETO_Init( LIS3MDL_0, &MAGNETO_handle ) )
        return -1;
    /* Force to use HTS221 */
    printf( "\r\n\tInitializing humidity sensor" );
    if ( COMPONENT_OK != BSP_HUMIDITY_Init( HTS221_H_0, &HUMIDITY_handle ) )
        return -1;
    /* Force to use HTS221 */
    printf( "\r\n\tInitializing temperature sensor" );
    if ( COMPONENT_OK != BSP_TEMPERATURE_Init( HTS221_T_0, &TEMPERATURE_handle ) )
        return -1;
    /* Try to use LPS25HB DIL24 if present, otherwise use LPS25HB on board */
    printf( "\r\n\tInitializing barometer" );
    if ( COMPONENT_OK != BSP_PRESSURE_Init( PRESSURE_SENSORS_AUTO, &PRESSURE_handle ) )
        return -1;

    printf( "\r\n\tSensor board initialization [OK]" );
    return 0;
}

/**
 * @brief  Enable all sensors
 * @param  None
 * @retval None
 */
void io_sensorboard_enable( void )
{
    printf( "\r\n>> Enabling all sensors" );
    printf( "\r\n\tEnabling accelerometer" );
    BSP_ACCELERO_Sensor_Enable( ACCELERO_handle );
    printf( "\r\n\tEnabling gyroscope" );
    BSP_GYRO_Sensor_Enable( GYRO_handle );
    printf( "\r\n\tEnabling magnetometer" );
    BSP_MAGNETO_Sensor_Enable( MAGNETO_handle );
    printf( "\r\n\tEnabling humidity sensor" );
    BSP_HUMIDITY_Sensor_Enable( HUMIDITY_handle );
    printf( "\r\n\tEnabling temperature sensor" );
    BSP_TEMPERATURE_Sensor_Enable( TEMPERATURE_handle );
    printf( "\r\n\tEnabling barometer" );
    BSP_PRESSURE_Sensor_Enable( PRESSURE_handle );
    printf( "\r\n\tAll sensors enabled" );
}

/**
 * @brief  Read sensor data
 * @param  read_values is a pre-allocated pointer to a SensorAxes_t struct
 * @retval  0 = Success
 * @retval -1 = Error
 */
int8_t io_read_gyro( SensorAxes_t* read_values )
{
    static SensorAxes_t input;
    uint8_t status = 0;

    assert( NULL != read_values );
    if ( COMPONENT_OK != BSP_GYRO_IsInitialized( GYRO_handle, &status ) || status != 1 )
    {
        printf( "\r\n>> Gyroscope initialization check [ERROR] Status: %d", status );
        return -1;
    }
    if ( COMPONENT_OK != BSP_GYRO_Get_Axes( GYRO_handle, &input ) )
    {
        printf( "\r\n>> Gyroscope read [ERROR]" );
        return -1;
    }
    printf( "\r\n>> Gyroscope data read [OK] {'x': %ld, 'y': %ld, 'z': %ld}",
            input.AXIS_X, input.AXIS_Y, input.AXIS_Z );

    read_values->AXIS_X = input.AXIS_X;
    read_values->AXIS_Y = input.AXIS_Y;
    read_values->AXIS_Z = input.AXIS_Z;
    return 0;
}

/**
 * @brief  Read sensor data
 * @param  read_values is a pre-allocated pointer to a SensorAxes_t struct
 * @retval  0 = Success
 * @retval -1 = Error
 */
int8_t io_read_accelero( SensorAxes_t* read_values )
{
    static SensorAxes_t input;
    uint8_t status = 0;

    assert( NULL != read_values );
    if ( COMPONENT_OK != BSP_ACCELERO_IsInitialized( ACCELERO_handle, &status ) ||
         status != 1 )
    {
        printf( "\r\n>> Accelerometer initialization check [ERROR] Status: %d", status );
        return -1;
    }
    if ( COMPONENT_OK != BSP_ACCELERO_Get_Axes( ACCELERO_handle, &input ) )
    {
        printf( "\r\n>> Accelerometer read [ERROR]" );
        return -1;
    }
    printf( "\r\n>> Accelerometer data read [OK] {'x': %ld, 'y': %ld, 'z': %ld}",
            input.AXIS_X, input.AXIS_Y, input.AXIS_Z );

    read_values->AXIS_X = input.AXIS_X;
    read_values->AXIS_Y = input.AXIS_Y;
    read_values->AXIS_Z = input.AXIS_Z;
    return 0;
}

/**
 * @brief  Read sensor data
 * @param  read_values is a pre-allocated pointer to a SensorAxes_t struct
 * @retval  0 = Success
 * @retval -1 = Error
 */
int8_t io_read_magneto( SensorAxes_t* read_values )
{
    static SensorAxes_t input;
    uint8_t status = 0;

    assert( NULL != read_values );
    if ( COMPONENT_OK != BSP_MAGNETO_IsInitialized( MAGNETO_handle, &status ) ||
         status != 1 )
    {
        printf( "\r\n>> Magnetometer initialization check [ERROR] Status: %d", status );
        return -1;
    }
    if ( COMPONENT_OK != BSP_MAGNETO_Get_Axes( MAGNETO_handle, &input ) )
    {
        printf( "\r\n>> Magnetometer read [ERROR]" );
        return -1;
    }
    printf( "\r\n>> Magnetometer data read [OK] {'x': %ld, 'y': %ld, 'z': %ld}",
            input.AXIS_X, input.AXIS_Y, input.AXIS_Z );

    read_values->AXIS_X = input.AXIS_X;
    read_values->AXIS_Y = input.AXIS_Y;
    read_values->AXIS_Z = input.AXIS_Z;
    return 0;
}

/**
 * @brief  Read sensor data
 * @param  read_value is a pre-allocated pointer to a float value
 * @retval  0 = Success
 * @retval -1 = Error
 */
int8_t io_read_pressure( float* read_value )
{
    float input;
    int32_t input_integer, input_fractional;
    uint8_t status = 0;

    assert( NULL != read_value );
    if ( COMPONENT_OK != BSP_PRESSURE_IsInitialized( PRESSURE_handle, &status ) ||
         status != 1 )
    {
        printf( "\r\n>> Barometer initialization check [ERROR] Status: %d", status );
        return -1;
    }
    if ( COMPONENT_OK != BSP_PRESSURE_Get_Press( PRESSURE_handle, &input ) )
    {
        printf( "\r\n>> Barometer read [ERROR]" );
        return -1;
    }
    split_float_to_ints( input, &input_integer, &input_fractional, 2 );
    printf( "\r\n>> Barometer data read [OK] Pressure: %ld.%02ld", input_integer,
            input_fractional );

    *read_value = input;
    return 0;
}

/**
 * @brief  Read sensor data
 * @param  read_value is a pre-allocated pointer to a float value
 * @retval  0 = Success
 * @retval -1 = Error
 */
int8_t io_read_temperature( float* read_value )
{
    float input;
    int32_t input_integer, input_fractional;
    uint8_t status = 0;

    assert( NULL != read_value );
    if ( COMPONENT_OK != BSP_TEMPERATURE_IsInitialized( TEMPERATURE_handle, &status ) ||
         status != 1 )
    {
        printf( "\r\n>> Thermometer initialization check [ERROR] Status: %d", status );
        return -1;
    }
    if ( COMPONENT_OK != BSP_TEMPERATURE_Get_Temp( TEMPERATURE_handle, &input ) )
    {
        printf( "\r\n>> Thermometer read [ERROR]" );
        return -1;
    }
    split_float_to_ints( input, &input_integer, &input_fractional, 2 );
    printf( "\r\n>> Thermometer data read [OK] Temperature: %ld.%02ld", input_integer,
            input_fractional );

    *read_value = input;
    return 0;
}

/**
 * @brief  Read sensor data
 * @param  read_value is a pre-allocated pointer to a float value
 * @retval  0 = Success
 * @retval -1 = Error
 */
int8_t io_read_humidity( float* read_value )
{
    float input;
    int32_t input_integer, input_fractional;
    uint8_t status = 0;

    assert( NULL != read_value );
    if ( COMPONENT_OK != BSP_HUMIDITY_IsInitialized( HUMIDITY_handle, &status ) ||
         status != 1 )
    {
        printf( "\r\n>> Hygrometer initialization check [ERROR] Status: %d", status );
        return -1;
    }
    if ( COMPONENT_OK != BSP_HUMIDITY_Get_Hum( HUMIDITY_handle, &input ) )
    {
        printf( "\r\n>> Hygrometer read [ERROR]" );
        return -1;
    }
    split_float_to_ints( input, &input_integer, &input_fractional, 2 );
    printf( "\r\n>> Hygrometer data read [OK] Humidity: %ld.%02ld", input_integer,
            input_fractional );

    *read_value = input;
    return 0;
}

/******************************************************************************
*                                   Helpers                                   *
******************************************************************************/

/**
 * @brief  Splits a float into two integer values
 * @param  in the float value as input
 * @param  out_int the pointer to the integer part as output
 * @param  out_dec the pointer to the decimal part as output
 * @param  dec_prec the decimal precision to be used
 * @retval None
 */
static void split_float_to_ints( float in, int32_t* out_int, int32_t* out_dec, int32_t dec_prec )
{
    *out_int = ( int32_t )in;
    if ( in >= 0.0f )
    {
        in = in - ( float )( *out_int );
    }
    else
    {
        in = ( float )( *out_int ) - in;
    }
    *out_dec = ( int32_t )trunc( in * pow( 10, dec_prec ) );
}

/**
 * @brief  Create a string representing the float value read from one of the
 *         single-dimention sensors on the sensor board.
 *             ```
 *             char str[IO_FLOAT_BUFFER_MAX_SIZE] = "";
 *             float input;
 *             io_read_humidity( &input );
 *             io_float_to_string( input, str, IO_FLOAT_BUFFER_MAX_SIZE );
 *             ```
 * @param  axes is the input received from an appropriate io_read_*() function
 * @param  *buf points at PREVIOUSLY ALLOCATED string with at least
 *         IO_FLOAT_BUFFER_MAX_SIZE bytes
 * @param  Bytes allocated for *buf
 * @retval  0 = Success
 * @retval -1 = Error
 */
int8_t io_float_to_string( float input, char* buf , int32_t buf_size )
{
    int retv = 0;
    int32_t input_integer = 0, input_fractional = 0;
    split_float_to_ints( input, &input_integer, &input_fractional, 2 );
    retv = snprintf( buf, buf_size, "%ld.%02ld", input_integer, input_fractional );
    if ( retv >= buf_size )
    {
        return -1;
    }
    return 0;
}

/**
 * @brief  Create a JSON string representing the values read from any 3-axis
 *         sensor. Recommeded usage:
 *             ```
 *             char json_str[IO_AXES_JSON_BUFFER_MAX_SIZE] = "";
 *             SensorAxes_t input;
 *             io_read_gyro( &input );
 *             io_axes_to_json( input, json_str, IO_AXES_JSON_BUFFER_MAX_SIZE );
 *             ```
 * @param  axes is the input received from an appropriate io_read_*() function
 * @param  *buf points at PREVIOUSLY ALLOCATED string with at least
 *         IO_AXES_JSON_BUFFER_MAX_SIZE bytes
 * @param  Bytes allocated for *buf
 * @retval  0 = Success
 * @retval -1 = Error
 */
int8_t io_axes_to_json( SensorAxes_t axes, char* buf , int32_t buf_size )
{
    int retv = 0;
    retv = snprintf( buf, buf_size, "{\"x\": %ld, \"y\": %ld, \"z\": %ld}",
                     axes.AXIS_X, axes.AXIS_Y, axes.AXIS_Z );
    if ( retv >= buf_size )
    {
        return -1;
    }
    return 0;
}
