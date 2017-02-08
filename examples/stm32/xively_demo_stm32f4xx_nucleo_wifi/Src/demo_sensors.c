#include <assert.h>
#include <string.h>
#include <math.h>

#include "mems_cube_hal.h"
#include "x_nucleo_iks01a1.h"
#include "x_nucleo_iks01a1_accelero.h"
#include "x_nucleo_iks01a1_gyro.h"
#include "x_nucleo_iks01a1_humidity.h"
#include "x_nucleo_iks01a1_magneto.h"
#include "x_nucleo_iks01a1_pressure.h"
#include "x_nucleo_iks01a1_temperature.h"
#include "LSM6DS0_ACC_GYRO_driver_HL.h"

#include "main.h"
#include "demo_sensors.h"
#include "sensor.h"

/* DemoSerial.c externs these and sets them when there's new data */
static void* ACCELERO_handle    = NULL;
static void* GYRO_handle        = NULL;
static void* MAGNETO_handle     = NULL;
static void* HUMIDITY_handle    = NULL;
static void* TEMPERATURE_handle = NULL;
static void* PRESSURE_handle    = NULL;

/* Helpers */
static void floatToInt( float in, int32_t* out_int, int32_t* out_dec, int32_t dec_prec );

/**
 * @brief  Splits a float into two integer values.
 * @param  in the float value as input
 * @param  out_int the pointer to the integer part as output
 * @param  out_dec the pointer to the decimal part as output
 * @param  dec_prec the decimal precision to be used
 * @retval None
 */
static void floatToInt( float in, int32_t* out_int, int32_t* out_dec, int32_t dec_prec )
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
 * @brief  Initialize all sensors
 * @param  None
 * @retval -1 error, 0 success
 * @TODO: This function differs for the IKS01A1 and IKS01A2 sensor boards.
 *        The current implementation was lifted from the IKS01A1 examples
 */
int8_t sensors_init( void )
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
void sensors_enable( void )
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
int8_t sensors_read_gyro( SensorAxes_t* read_values )
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
int8_t sensors_read_accelero( SensorAxes_t* read_values )
{
    read_values->AXIS_X = 42;
    read_values->AXIS_Y = 42;
    read_values->AXIS_Z = 42;
    return 0;
}

/**
 * @brief  Read sensor data
 * @param  read_values is a pre-allocated pointer to a SensorAxes_t struct
 * @retval  0 = Success
 * @retval -1 = Error
 */
int8_t sensors_read_magneto( SensorAxes_t* read_values )
{
    read_values->AXIS_X = 42;
    read_values->AXIS_Y = 42;
    read_values->AXIS_Z = 42;
    return 0;
}

/*********************** RTC ***********************************************/
/* TODO: Move all the RTC logic to a different file, away from the sensors */

/**
 * @brief  Configures the current time and date
 * @param  hh the hour value to be set
 * @param  mm the minute value to be set
 * @param  ss the second value to be set
 * @retval None
 */
void RTC_TimeRegulate( uint8_t hh, uint8_t mm, uint8_t ss )
{
#if 0
    RTC_TimeTypeDef stimestructure;

    stimestructure.TimeFormat     = RTC_HOURFORMAT12_AM;
    stimestructure.Hours          = hh;
    stimestructure.Minutes        = mm;
    stimestructure.Seconds        = ss;
    stimestructure.SubSeconds     = 0;
    stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;

    if ( HAL_RTC_SetTime( &RtcHandle, &stimestructure, FORMAT_BIN ) != HAL_OK )
    {
        /* Initialization Error */
        Error_Handler();
    }
#endif
}
