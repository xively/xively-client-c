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
#include "DemoSerial.h"
#include "sensor.h"

/* Misc */
volatile uint32_t Sensors_Enabled = 0;   /*!< Enable Sensor Flag */
volatile uint32_t DataTxPeriod = 1000;   /*!< TX DATA Period */

/* Set from the _Handler functions in this file */
static SensorAxes_t ACC_Value;  /*!< Acceleration Value */
static SensorAxes_t GYR_Value;  /*!< Gyroscope Value */
static SensorAxes_t MAG_Value;  /*!< Magnetometer Value */
static float PRESSURE_Value;    /*!< Pressure Value */
static float HUMIDITY_Value;    /*!< Humidity Value */
static float TEMPERATURE_Value; /*!< Temperature Value */

static volatile uint32_t Int_Current_Time1 = 0; /*!< Int_Current_Time1 Value */
static volatile uint32_t Int_Current_Time2 = 0; /*!< Int_Current_Time2 Value */

/* Rx Data Handlers - They parse sensor readings-TODO: What else do they do? */
static void RTC_Handler( TMsg* Msg );
static void Accelero_Sensor_Handler( TMsg* Msg );
static void Gyro_Sensor_Handler( TMsg* Msg );
static void Magneto_Sensor_Handler( TMsg* Msg );
static void Pressure_Sensor_Handler( TMsg* Msg );
static void Humidity_Sensor_Handler( TMsg* Msg );
static void Temperature_Sensor_Handler( TMsg* Msg );

/* Helpers */
static void floatToInt( float in, int32_t* out_int, int32_t* out_dec, int32_t dec_prec );

uint8_t SensorsStreamingData = 0;

/**
 * @brief  Splits a float into two integer values.
 * @param  in the float value as input
 * @param  out_int the pointer to the integer part as output
 * @param  out_dec the pointer to the decimal part as output
 * @param  dec_prec the decimal precision to be used
 * @retval None
 */
static void floatToInt(float in, int32_t *out_int, int32_t *out_dec, int32_t dec_prec)
{
  *out_int = (int32_t)in;
  if(in >= 0.0f)
  {
    in = in - (float)(*out_int);
  }
  else
  {
    in = (float)(*out_int) - in;
  }
  *out_dec = (int32_t)trunc(in * pow(10, dec_prec));
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
    DrvStatusTypeDef retval = COMPONENT_ERROR;
    printf( "\r\n>> Initializing all sensors" );

    printf( "\r\n\tInitializing sensor board USART" );
    //USARTConfig();

    /* Try to use LSM6DS3 DIL24 if present, otherwise use LSM6DS0 on board */
    printf( "\r\n\tInitializing accelerometer" );
    if( COMPONENT_OK != BSP_ACCELERO_Init( ACCELERO_SENSORS_AUTO, &ACCELERO_handle ) )
    {
        goto error_out;
    }

    /* Try to use LSM6DS3 if present, otherwise use LSM6DS0 */
    printf( "\r\n\tInitializing gyroscope" );
    if( COMPONENT_OK != BSP_GYRO_Init( GYRO_SENSORS_AUTO, &GYRO_handle ) )
    {
        goto error_out;
    }

    /* Force to use LIS3MDL */
    printf( "\r\n\tInitializing magnetometer" );
    if( COMPONENT_OK != BSP_MAGNETO_Init( LIS3MDL_0, &MAGNETO_handle ) )
    {
        goto error_out;
    }

    /* Force to use HTS221 */
    printf( "\r\n\tInitializing humidity sensor" );
    if( COMPONENT_OK != BSP_HUMIDITY_Init( HTS221_H_0, &HUMIDITY_handle ) )
    {
        goto error_out;
    }

    /* Force to use HTS221 */
    printf( "\r\n\tInitializing temperature sensor" );
    if( COMPONENT_OK != BSP_TEMPERATURE_Init( HTS221_T_0, &TEMPERATURE_handle ) )
    {
        goto error_out;
    }

    /* Try to use LPS25HB DIL24 if present, otherwise use LPS25HB on board */
    printf( "\r\n\tInitializing barometer" );
    if( COMPONENT_OK != BSP_PRESSURE_Init( PRESSURE_SENSORS_AUTO, &PRESSURE_handle ) )
    {
        goto error_out;
    }

    SensorsStreamingData = 1;
    return 0;

error_out:
    printf( " [ERROR] Status code: %d", retval );
    return -1;
}

/**
 * @brief  Enable all sensors
 * @param  None
 * @retval None
 */
void sensors_enable(void)
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
}

int8_t sensors_read_gyro( SensorAxes_t* read_values )
{
    int32_t data[6];
    uint8_t status = 0;
    SensorAxes_t input;

    assert( NULL != read_values );
    if ( BSP_GYRO_IsInitialized( GYRO_handle, &status ) != COMPONENT_OK )
    {
        printf("\r\n>> Gyroscope initialization check [ERROR]");
        return -1;
    }
    if ( status != 1 )
    {
        printf( "\r\n>> Gyroscope read [ERROR] Status code: %d", status );
        return -1;
    }

    printf( "\r\n>> Gyroscope data read [OK]" );
    BSP_GYRO_Get_Axes(GYRO_handle, &GYR_Value);
    read_values->AXIS_X = GYR_Value.AXIS_X;
    read_values->AXIS_Y = GYR_Value.AXIS_Y;
    read_values->AXIS_Z = GYR_Value.AXIS_Z;
    printf( "\r\n\tX: %ld\r\n\tY: %ld\r\n\tY: %ld", read_values->AXIS_X, read_values->AXIS_Y,
            read_values->AXIS_Z );
    return 0;

    #if 0
    if (  == COMPONENT_OK && status == 1 )
    {
        printf( "\r\n>> Reading Gyroscope data" );
        BSP_GYRO_Get_Axes( GYRO_handle, &GYR_Value );
        data[3] = GYR_Value.AXIS_X;
        data[4] = GYR_Value.AXIS_Y;
        data[5] = GYR_Value.AXIS_Z;

        printf( "\r\n\tGYR_X: %d, GYR_Y: %d, GYR_Z: %d", ( int )data[3], ( int )data[4],
                ( int )data[5] );
    }
    #endif
}

SensorAxes_t sensors_read_accelero( void )
{
    SensorAxes_t fakedata;
    fakedata.AXIS_X = 42;
    fakedata.AXIS_Y = 42;
    fakedata.AXIS_Z = 42;
    return fakedata;
}

SensorAxes_t sensors_read_magneto( void )
{
    SensorAxes_t fakedata;
    fakedata.AXIS_X = 42;
    fakedata.AXIS_Y = 42;
    fakedata.AXIS_Z = 42;
    return fakedata;
}


/**
 * @brief  Checks the I2C input buffer for data from the Sensor Extension Board.
 *         
 * @param  None
 * @retval int8_t. 0=Success, -1=Failure
 */
int8_t sensors_tick( void )
{
    TMsg Msg;
    while(1)
    {
        #if 0
        if (UART_ReceivedMSG((TMsg*) &Msg))
        {
            printf( "\r\n>> Got an incoming serial message in the I2C port" );
            if ( Msg.Data[0] == DEV_ADDR )
            {
                printf( "\r\n\tMessage came from the sensor board" );
                if ( HandleMSG( ( TMsg* )&Msg ) != 1 )
                {
                    printf( "\r\n\tHandling of I2C message [FAIL] Contents: " );
                    for ( int i = 0; i < Msg.Len; i++ )
                    {
                        printf( "0x%02x ", Msg.Data[i] );
                    }
                    return -1;
                }
                #if 0
                if ( SensorsStreamingData )
                {
                    AutoInit = 0;
                }
                #endif
            }
        }
        else
        {
            printf( "\r\n>> No new messages in the I2C port" );
        }
        #endif

        RTC_Handler(&Msg);
        Pressure_Sensor_Handler(&Msg);
        Humidity_Sensor_Handler(&Msg);
        Temperature_Sensor_Handler(&Msg);
        Accelero_Sensor_Handler(&Msg);
        Gyro_Sensor_Handler(&Msg);
        Magneto_Sensor_Handler(&Msg);

        if(SensorsStreamingData)
        {
          INIT_STREAMING_HEADER(&Msg);
          Msg.Len = STREAMING_MSG_LENGTH;
          UART_SendMsg(&Msg);
          HAL_Delay(DataTxPeriod);
        }

        #if 0
        if ( AutoInit )
        {
            HAL_Delay( 500 );
        }
        #endif
    }
}

/**
 * @brief  Handles the GYRO axes data getting/sending
 * @param  Msg the GYRO part of the stream
 * @retval None
 */
static void Gyro_Sensor_Handler(TMsg *Msg)
{
    SensorAxes_t read_values;
    int32_t data[6];
    uint8_t status = 0;
    if ( BSP_GYRO_IsInitialized( GYRO_handle, &status ) != COMPONENT_OK )
    {
        printf("\r\n>> Gyroscope initialization check [ERROR]");
        return;
    }
    if ( status != 1 )
    {
        printf( "\r\n>> Gyroscope read [ERROR] Status code: %d", status );
        return;
    }

    BSP_GYRO_Get_Axes( GYRO_handle, &GYR_Value );

    //if ( SensorsStreamingData )
    //{
        //if ( Sensors_Enabled & GYROSCOPE_SENSOR )
        //{
        //    Serialize_s32( &Msg->Data[27], GYR_Value.AXIS_X, 4 );
        //    Serialize_s32( &Msg->Data[31], GYR_Value.AXIS_Y, 4 );
        //    Serialize_s32( &Msg->Data[35], GYR_Value.AXIS_Z, 4 );
        //}
    //}
    //else
    //{
        printf( "\r\n>> Gyroscope data read [OK]" );
        BSP_GYRO_Get_Axes(GYRO_handle, &GYR_Value);
        read_values.AXIS_X = GYR_Value.AXIS_X;
        read_values.AXIS_Y = GYR_Value.AXIS_Y;
        read_values.AXIS_Z = GYR_Value.AXIS_Z;
        printf( "\r\n\tX: %ld\r\n\tY: %ld\r\n\tY: %ld", read_values.AXIS_X,
                read_values.AXIS_Y, read_values.AXIS_Z );
    //}

    #if 0
    else if ( AutoInit )
    {
        data[3] = GYR_Value.AXIS_X;
        data[4] = GYR_Value.AXIS_Y;
        data[5] = GYR_Value.AXIS_Z;

        sprintf( dataOut, "GYR_X: %d, GYR_Y: %d, GYR_Z: %d\r\n", ( int )data[3],
                 ( int )data[4], ( int )data[5] );
        HAL_UART_Transmit( &UartHandle, ( uint8_t* )dataOut, strlen( dataOut ),
                           5000 );
    }
    #endif
}


/**
 * @brief  Handles the MAGNETO axes data getting/sending
 * @param  Msg the MAGNETO part of the stream
 * @retval None
 */
static void Magneto_Sensor_Handler(TMsg *Msg)
{
    #if 0
    int32_t data[3];
    uint8_t status = 0;

    if ( BSP_MAGNETO_IsInitialized( MAGNETO_handle, &status ) == COMPONENT_OK &&
         status == 1 )
    {
        BSP_MAGNETO_Get_Axes( MAGNETO_handle, &MAG_Value );

        if ( SensorsStreamingData )
        {
            if ( Sensors_Enabled & MAGNETIC_SENSOR )
            {
                Serialize_s32( &Msg->Data[39], ( int32_t )MAG_Value.AXIS_X, 4 );
                Serialize_s32( &Msg->Data[43], ( int32_t )MAG_Value.AXIS_Y, 4 );
                Serialize_s32( &Msg->Data[47], ( int32_t )MAG_Value.AXIS_Z, 4 );
            }
        }

        else if ( AutoInit )
        {
            data[0] = MAG_Value.AXIS_X;
            data[1] = MAG_Value.AXIS_Y;
            data[2] = MAG_Value.AXIS_Z;

            sprintf( dataOut, "MAG_X: %d, MAG_Y: %d, MAG_Z: %d\r\n", ( int )data[0],
                     ( int )data[1], ( int )data[2] );
            HAL_UART_Transmit( &UartHandle, ( uint8_t* )dataOut, strlen( dataOut ),
                               5000 );
        }
    }
    #endif
}
/**
 * @brief  Handles the PRESSURE sensor data getting/sending
 * @param  Msg the PRESSURE part of the stream
 * @retval None
 */
static void Pressure_Sensor_Handler(TMsg *Msg)
{
}
/**
 * @brief  Handles the HUMIDITY sensor data getting/sending
 * @param  Msg the HUMIDITY part of the stream
 * @retval None
 */
static void Humidity_Sensor_Handler(TMsg *Msg)
{
}
/**
 * @brief  Handles the TEMPERATURE sensor data getting/sending
 * @param  Msg the TEMPERATURE part of the stream
 * @retval None
 */
static void Temperature_Sensor_Handler(TMsg *Msg)
{
}
/**
 * @brief  Handles the time+date getting/sending
 * @param  Msg the time+date part of the stream
 * @retval None
 */
static void RTC_Handler( TMsg* Msg )
{
}
/**
 * @brief  Handles the ACCELERO axes data getting/sending
 * @param  Msg the ACCELERO part of the stream
 * @retval None
 */
static void Accelero_Sensor_Handler( TMsg* Msg )
{
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
void RTC_TimeRegulate(uint8_t hh, uint8_t mm, uint8_t ss)
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
