/******************************************************************************
 *                                                                            *
 *  xively_client.c                                                           *
 *                                                                            *
 ******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "cmsis_os.h"

#include "xively.h"
#include "xively_client.h"
#include "xi_bsp_rng.h"
#include "xi_bsp_time.h"
#include "demo_io.h"
#include "user_data.h"
#include "provisioning.h"

/******************************************************************************
 *                                                                            *
 *  Macros                                                                    *
 *                                                                            *
 ******************************************************************************/

#ifndef USE_HARDCODED_CREDENTIALS
/* Default workflow uses runtime provisioning and flash storage for user data */
#define USE_HARDCODED_CREDENTIALS 0
#endif /* USE_HARDCODED_CREDENTIALS */

#if USE_HARDCODED_CREDENTIALS
#define USER_CONFIG_XI_ACCOUNT_ID "Xively Account ID"
#define USER_CONFIG_XI_DEVICE_ID "Xively Device ID"
#define USER_CONFIG_XI_DEVICE_PWD "Xively Device Password"
#endif /* USE_HARDCODED_CREDENTIALS */

static user_data_t user_config;

/*
 *  Application Specific
 */
#define XC_CONNECT_TO 0    /* Seconds. Client/Broker connection wait time.   */
#define XC_KEEPALIVE_TO 10 /* Seconds. Client/Broker keep-alive period.      */
#define XC_CHECK_PERIOD 1  /* Seconds. Check period for publication changes. */

#define XC_TASK_STACK ( 512 * 6 )

#define XI_TOPIC_MAX_LEN 256

#define XC_NULL_CONTEXT ( ( xi_context_handle_t )-1 )
#define XC_NULL_T_HANDLE ( ( xi_timed_task_handle_t )-1 )

/******************************************************************************
 *                                                                            *
 *  Definitions                                                               *
 *                                                                            *
 ******************************************************************************/

/*
 *  Subscribe callback function prototype.
 */
typedef xi_state_t( subscribe_cb_t )( const xi_context_handle_t ctx,
                                      void* data,
                                      xi_state_t state );
static uint8_t virtual_switch_state = 0; /* Pressing the user button flips this value */
#define is_burst_mode virtual_switch_state

/******************************************************************************
 *                                                                            *
 *  Variables                                                                 *
 *                                                                            *
 ******************************************************************************/

/*
 *  web control
 */
int xc_control;

/*
 *  Xively context handle
 */
static xi_context_handle_t xc_ctx = XC_NULL_CONTEXT;

/*
 *  Monitoring event handle
 */
static xi_timed_task_handle_t monitor_cb_hdl = XC_NULL_T_HANDLE;

/*
 *  MQTT topics
 */
/* RAM can be saved by building each of these when needed and free()ing them after use */
/* Xively channel names - These weill be used to build the MQTT topic addresses */
#define ACCELEROMETER_CHANNEL_NAME "Accelerometer"
#define GYROSCOPE_CHANNEL_NAME "Gyroscope"
#define MAGNETOMETER_CHANNEL_NAME "Magnetometer"
#define BAROMETER_CHANNEL_NAME "Barometer"
#define HUMIDITY_CHANNEL_NAME "Humidity"
#define TEMPERATURE_CHANNEL_NAME "Temperature"
#define BUTTON_CHANNEL_NAME "Button"
#define LED_CHANNEL_NAME "LED"

typedef struct mqtt_topics_s
{
    char accelerometer[XI_TOPIC_MAX_LEN];
    char gyroscope[XI_TOPIC_MAX_LEN];
    char magnetometer[XI_TOPIC_MAX_LEN];
    char barometer[XI_TOPIC_MAX_LEN];
    char hygrometer[XI_TOPIC_MAX_LEN];
    char temperature[XI_TOPIC_MAX_LEN];
    char button[XI_TOPIC_MAX_LEN];
    char led[XI_TOPIC_MAX_LEN];
} mqtt_topics_t;

static mqtt_topics_t mqtt_topics;

/*
 *  Data publication vars
 */

volatile int8_t button_pressed_interrupt_flag = 2;

/* the interval for the time function */
#define XI_PUBLISH_INTERVAL_SEC 5

typedef struct topic_descr_s
{
    const char* const name;
} mqtt_topic_descr_t;

/* declaration of fn pointer for the msg handler */
typedef void ( *on_msg_handler_fn_t )( const uint8_t* const msg, size_t msg_size );

/* declaration of fn pointer for the polling push mechanism */
typedef xi_state_t ( *on_msg_push_fn_t )(
    const mqtt_topic_descr_t* const mqtt_topic_descr );

/* each topic will have it's descriptor and the handler in case of the message */
typedef struct mqtt_topic_configuration_s
{
    mqtt_topic_descr_t topic_descr;
    on_msg_handler_fn_t on_msg_pull;
    on_msg_push_fn_t on_msg_push;
} mqtt_topic_configuration_t;

static xi_state_t pub_accelerometer( void );
static xi_state_t pub_magnetometer( void );
static xi_state_t pub_barometer( void );
// static xi_state_t pub_button( void );
static xi_state_t pub_gyroscope( void );
static xi_state_t pub_humidity( void );
static xi_state_t pub_temperature( void );

/* topic's initialisation function */
static xi_state_t init_xively_topics( xi_context_handle_t in_context_handle );
static int8_t update_all_mqtt_topics( char* xi_account_id, char* xi_device_id );
static int8_t build_xively_topic(
    char* topic_name, char* account_id, char* device_id, char* dst, uint32_t dst_len );

// xi_context_handle_t xc_ctx      = XI_INVALID_CONTEXT_HANDLE;
xi_timed_task_handle_t gXivelyTimedTaskHandle = XI_INVALID_TIMED_TASK_HANDLE;

/**
 * @brief This function receives a pointer to a pre-allocated buffer to be filled in.
 *        We recommend that string is allocated XI_TOPIC_MAX_LEN bytes
 */
static int8_t build_xively_topic(
    char* topic_name, char* account_id, char* device_id, char* dst, uint32_t dst_len )
{
    int retval = 0;

    assert( NULL != topic_name );
    assert( NULL != account_id );
    assert( NULL != device_id );
    assert( NULL != dst );

    retval = snprintf( dst, dst_len, "xi/blue/v1/%.64s/d/%.64s/%.64s", account_id,
                       device_id, topic_name );

    if ( ( retval >= dst_len ) || ( retval < 0 ) )
    {
        return -1;
    }
    return 0;
}

/**
 * @brief  Build the MQTT topic addresses with the given account_id and device_id
 * @param
 * @retval -1: Error
 * @retval 0: OK
 */
static int8_t update_all_mqtt_topics( char* xi_account_id, char* xi_device_id )
{
    assert( strlen( xi_account_id ) > 0 );
    assert( strlen( xi_device_id ) > 0 );

    if ( 0 > build_xively_topic( ACCELEROMETER_CHANNEL_NAME, xi_account_id, xi_device_id,
                                 mqtt_topics.accelerometer, XI_TOPIC_MAX_LEN ) )
        return -1;

    if ( 0 > build_xively_topic( GYROSCOPE_CHANNEL_NAME, xi_account_id, xi_device_id,
                                 mqtt_topics.gyroscope, XI_TOPIC_MAX_LEN ) )
        return -1;

    if ( 0 > build_xively_topic( MAGNETOMETER_CHANNEL_NAME, xi_account_id, xi_device_id,
                                 mqtt_topics.magnetometer, XI_TOPIC_MAX_LEN ) )
        return -1;

    if ( 0 > build_xively_topic( BAROMETER_CHANNEL_NAME, xi_account_id, xi_device_id,
                                 mqtt_topics.barometer, XI_TOPIC_MAX_LEN ) )
        return -1;

    if ( 0 > build_xively_topic( HUMIDITY_CHANNEL_NAME, xi_account_id, xi_device_id,
                                 mqtt_topics.hygrometer, XI_TOPIC_MAX_LEN ) )
        return -1;

    if ( 0 > build_xively_topic( TEMPERATURE_CHANNEL_NAME, xi_account_id, xi_device_id,
                                 mqtt_topics.temperature, XI_TOPIC_MAX_LEN ) )
        return -1;

    if ( 0 > build_xively_topic( BUTTON_CHANNEL_NAME, xi_account_id, xi_device_id,
                                 mqtt_topics.button, XI_TOPIC_MAX_LEN ) )
        return -1;

    if ( 0 > build_xively_topic( LED_CHANNEL_NAME, xi_account_id, xi_device_id,
                                 mqtt_topics.led, XI_TOPIC_MAX_LEN ) )
        return -1;

    return 0;
}

/* handler for led msg */
void on_led_msg( xi_context_handle_t in_context_handle,
                 xi_sub_call_type_t call_type,
                 const xi_sub_call_params_t* const params,
                 xi_state_t state,
                 void* user_data )
{
    switch ( call_type )
    {
        case XI_SUB_CALL_SUBACK:
            if ( XI_MQTT_SUBACK_FAILED == params->suback.suback_status )
            {
                printf( "\r\n>> Subscription to LED topic [FAIL]" );
            }
            else
            {
                printf( "\r\n>> Subscription to LED topic [OK]" );
            }
            return;
        case XI_SUB_CALL_MESSAGE:
            printf( "\r\n>> received message on LED topic" );
            printf( "\r\n\t Message size: %d",
                    params->message.temporary_payload_data_length );
            printf( "\r\n\t Message payload: " );
            for ( size_t i = 0; i < params->message.temporary_payload_data_length; i++ )
            {
                printf( "0x%02x ", params->message.temporary_payload_data[i] );
            }

            if ( params->message.temporary_payload_data_length != 1 )
            {
                printf( "\r\n\tUnrecognized LED message [IGNORED] Expected 1 or 0" );
                return;
            }

            if ( params->message.temporary_payload_data[0] == '0' )
            {
                io_led_off();
            }
            else
            {
                io_led_on();
            }
            break;
        default:
            return;
    }
}

xi_state_t pub_accelerometer( void )
{
    char out_msg[IO_AXES_JSON_BUFFER_MAX_SIZE] = "";
    SensorAxes_t sensor_input;

    if ( io_read_accelero( &sensor_input ) < 0 )
    {
        return -1;
    }
    if ( io_axes_to_json( sensor_input, out_msg, IO_AXES_JSON_BUFFER_MAX_SIZE ) < 0 )
    {
        return -1;
    }
    return xi_publish( xc_ctx, mqtt_topics.accelerometer, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_magnetometer( void )
{
    char out_msg[IO_AXES_JSON_BUFFER_MAX_SIZE] = "";
    SensorAxes_t sensor_input;

    if ( io_read_magneto( &sensor_input ) < 0 )
    {
        return -1;
    }
    if ( io_axes_to_json( sensor_input, out_msg, IO_AXES_JSON_BUFFER_MAX_SIZE ) < 0 )
    {
        return -1;
    }
    return xi_publish( xc_ctx, mqtt_topics.magnetometer, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_gyroscope( void )
{
    char out_msg[IO_AXES_JSON_BUFFER_MAX_SIZE] = "";
    SensorAxes_t sensor_input;

    if ( io_read_gyro( &sensor_input ) < 0 )
    {
        return -1;
    }
    if ( io_axes_to_json( sensor_input, out_msg, IO_AXES_JSON_BUFFER_MAX_SIZE ) < 0 )
    {
        return -1;
    }
    return xi_publish( xc_ctx, mqtt_topics.gyroscope, out_msg, XI_MQTT_QOS_AT_MOST_ONCE,
                       XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_barometer( void )
{
    char out_msg[IO_FLOAT_BUFFER_MAX_SIZE] = "";
    float sensor_input                     = 0;
    if ( io_read_pressure( &sensor_input ) < 0 )
    {
        return -1;
    }
    if ( io_float_to_string( sensor_input, out_msg, IO_FLOAT_BUFFER_MAX_SIZE ) )
    {
        return -1;
    }
    return xi_publish( xc_ctx, mqtt_topics.barometer, out_msg, XI_MQTT_QOS_AT_MOST_ONCE,
                       XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_humidity( void )
{
    char out_msg[IO_FLOAT_BUFFER_MAX_SIZE] = "";
    float sensor_input                     = 0;
    if ( io_read_humidity( &sensor_input ) < 0 )
    {
        return -1;
    }
    if ( io_float_to_string( sensor_input, out_msg, IO_FLOAT_BUFFER_MAX_SIZE ) )
    {
        return -1;
    }
    return xi_publish( xc_ctx, mqtt_topics.hygrometer, out_msg, XI_MQTT_QOS_AT_MOST_ONCE,
                       XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_temperature( void )
{
    char out_msg[IO_FLOAT_BUFFER_MAX_SIZE] = "";
    float sensor_input                     = 0;
    if ( io_read_temperature( &sensor_input ) < 0 )
    {
        return -1;
    }
    if ( io_float_to_string( sensor_input, out_msg, IO_FLOAT_BUFFER_MAX_SIZE ) )
    {
        return -1;
    }
    return xi_publish( xc_ctx, mqtt_topics.temperature, out_msg, XI_MQTT_QOS_AT_MOST_ONCE,
                       XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_button( xi_context_handle_t context_handle,
                       xi_timed_task_handle_t timed_task_handle,
                       void* user_data )
{
    return xi_publish( xc_ctx, mqtt_topics.button, user_data, XI_MQTT_QOS_AT_MOST_ONCE,
                       XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

void pub_button_interrupt( void )
{
    button_pressed_interrupt_flag = button_pressed_interrupt_flag / 2;
}

void publish_mqtt_topics()
{
    if ( !is_burst_mode )
    {
        pub_gyroscope();
        pub_accelerometer();
    }

    // pub_virtual_switch();
    pub_magnetometer();
    pub_barometer();
    pub_humidity();
    pub_temperature();
}

/******************************************************************************
 *                                                                            *
 *  connect_cb ()                                                             *
 *                                                                            *
 *  Callback handler for "xi_connect"                                         *
 *                                                                            *
 ******************************************************************************/

static xi_state_t
connect_cb( const xi_context_handle_t ctx, void* data, xi_state_t state )
{
    xi_connection_data_t* conn_data = ( xi_connection_data_t* )data;
    xi_state_t rval;

    ( void )ctx;

    switch ( conn_data->connection_state )
    {
        /*
         *  Connection attempt FAILED
         */
        case XI_CONNECTION_STATE_OPEN_FAILED:
        {
            printf( "[%ld] Xively: Connection failed to %s:%d, error %d\n",
                    xi_bsp_time_getcurrenttime_seconds(), conn_data->host,
                    conn_data->port, state );

            xi_connect( ctx, conn_data->username, conn_data->password,
                        conn_data->connection_timeout, conn_data->keepalive_timeout,
                        conn_data->session_type, ( xi_user_callback_t* )&connect_cb );

            rval = XI_STATE_OK;
            break;
        }


        /*
         *  Preiviously open connection has CLOSED
         */
        case XI_CONNECTION_STATE_CLOSED:
        {
            printf( "[%ld] Xively: Connection closed, error %d\n",
                    xi_bsp_time_getcurrenttime_seconds(), state );

            /* Connection closed */
            xi_events_stop();

            rval = XI_STATE_OK;
            break;
        }


        /*
         *  New connection opened successfully
         */
        case XI_CONNECTION_STATE_OPENED:
        {
            printf( "[%ld] Xively: Connected %s:%d\n",
                    xi_bsp_time_getcurrenttime_seconds(), conn_data->host,
                    conn_data->port );
            rval = XI_STATE_OK;

            init_xively_topics( ctx );

            break;
        }


        default:
        {
            printf( "[%ld] Xively: Connection invalid, error %d\n",
                    xi_bsp_time_getcurrenttime_seconds(), conn_data->connection_state );
            rval = XI_INVALID_PARAMETER;
            break;
        }
    }

    return rval;
}

/*
 * Initializes Xively's demo application topics
 */
xi_state_t init_xively_topics( xi_context_handle_t in_context_handle )
{
    xi_state_t ret = XI_STATE_OK;

    if ( 0 >
         update_all_mqtt_topics( user_config.xi_account_id, user_config.xi_device_id ) )
    {
        printf( "\r\n>> Topic composition [ERROR]" );
        return XI_FAILED_INITIALIZATION;
    }

    printf( "\r\n>> Subscribing to MQTT topic [%s]", mqtt_topics.led );
    ret = xi_subscribe( in_context_handle, mqtt_topics.led, XI_MQTT_QOS_AT_MOST_ONCE,
                        &on_led_msg, NULL );

    if ( XI_STATE_OK != ret )
    {
        printf( "\r\n>> Xively subscription [ERROR]" );
        return XI_MQTT_SUBSCRIPTION_FAILED;
    }

    /* registration of the publish function */
    gXivelyTimedTaskHandle = xi_schedule_timed_task(
        in_context_handle, &publish_mqtt_topics, XI_PUBLISH_INTERVAL_SEC, 1, NULL );

    if ( XI_INVALID_TIMED_TASK_HANDLE == gXivelyTimedTaskHandle )
    {
        printf( "\r\n>> Libxively timed task registration [ERROR]" );
        return XI_FAILED_INITIALIZATION;
    }

    return ret;
}

/**
 * @brief Function that is required by TLS library to track the current time.
 */
time_t XTIME( time_t* timer )
{
    return xi_bsp_time_getcurrenttime_seconds();
}

/**
 * @brief Function required by the TLS library.
 */
uint32_t xively_ssl_rand_generate()
{
    return xi_bsp_rng_get();
}


/******************************************************************************
 *                                                                            *
 *  xc_main ()                                                                *
 *                                                                            *
 ******************************************************************************/

static int xc_main( void )
{
    int rval;
    xi_state_t xi_rc;

/*
 * Set identity values
 */
#if USE_HARDCODED_CREDENTIALS
    user_data_set_xi_account_id( ( &user_config ), USER_CONFIG_XI_ACCOUNT_ID );
    user_data_set_xi_device_id( ( &user_config ), USER_CONFIG_XI_DEVICE_ID );
    user_data_set_xi_device_password( ( &user_config ), USER_CONFIG_XI_DEVICE_PWD );
#else
    if ( user_data_flash_init() < 0 )
    {
        printf( "\r\n>> User data flash initialization [ERROR]" );
        return -1;
    }
    /* Read user data from flash */
    if ( user_data_copy_from_flash( &user_config ) < 0 )
    {
        printf( "\r\n>> [ERROR] trying to copy user data from flash. Abort" );
        while ( 1 )
            ;
    }
    printf( "\r\n>> User data retrieved from flash:" );
    user_data_printf( &user_config );
    fflush( stdout );

    /* Provision the device if requested or if flash data is missing||corrupt */
    const int8_t provisioning_bootmode = io_read_button();
    if ( ( 1 == provisioning_bootmode ) ||
         ( 0 > user_data_validate_checksum( &user_config ) ) )
    {
        ( 1 == provisioning_bootmode )
            ? printf( "\r\n>> User requested device reprovisioning" )
            : printf( "\r\n>> [ERROR] Invalid credentials recovered from flash" );
        if ( 0 > provisioning_gather_user_data( &user_config ) )
        {
            printf( "\r\n>> Device provisioning [ERROR]. Abort" );
            while ( 1 )
                ;
        }
    }
#endif /* USE_HARDCODED_CREDENTIALS */

    rval = 0;

    while ( 1 )
    {
        printf( "Xively: ...continue.\n" );
        //        test_alloc_report (-1);

        /*
         *  Initialize xively client library.
         */
        if ( XI_STATE_OK == ( xi_rc = xi_initialize( user_config.xi_account_id ) ) )
        {
            if ( ( xc_ctx = xi_create_context() ) < 0 )
            {
                xi_rc = ( xi_state_t )( 0 - xc_ctx );
                printf( "Xively: xi_create_context(): FATAL: RC: %d\n", xi_rc );
                rval = -1;
                break;
            }
        }
        else
        {
            printf( "Xively: xi_initialize(): FATAL: RC: %d\n", xi_rc );
            rval = -1;
            break;
        }

        /*
         *  Connect to the Xively broker.
         *  "device_id" functions as the user name.
         */
        printf( "Xively: Connect pending\n" );
        printf( " deviceId \n  %s\n", user_config.xi_device_id );
        printf( " accountId\n  %s\n", user_config.xi_account_id );


        xi_connect( xc_ctx, user_config.xi_device_id, user_config.xi_device_password,
                    XC_CONNECT_TO, XC_KEEPALIVE_TO, XI_SESSION_CLEAN,
                    ( xi_user_callback_t* )&connect_cb );

        /*  Loop forever */
        while ( XI_STATE_OK == xi_events_process_tick() )
        {
            if ( 1 == abs( button_pressed_interrupt_flag ) )
            {
                xi_schedule_timed_task( xc_ctx, ( xi_user_task_callback_t* )pub_button, 0,
                                        0, ( 1 == button_pressed_interrupt_flag ) ? "1"
                                                                                  : "0" );

                /* turn 1 to -2 or -1 to 2 */
                button_pressed_interrupt_flag -= 3 * button_pressed_interrupt_flag;
            }
        }

        printf( "Xively: Stopped\n" );

        if ( monitor_cb_hdl >= 0 )
        {
            xi_cancel_timed_task( monitor_cb_hdl );
        }

        if ( ( xi_rc = xi_delete_context( xc_ctx ) ) != XI_STATE_OK )
        {
            printf( "Xively: xi_delete_context(): FATAL: XRC: %d\n", xi_rc );
        }

        if ( ( xi_rc = xi_shutdown() ) != XI_STATE_OK )
        {
            printf( "Xively: xi_shutdown(): FATAL: XRC: %d\n", xi_rc );
        }

        monitor_cb_hdl = XC_NULL_T_HANDLE;
        xc_ctx         = XC_NULL_CONTEXT;

        osDelay( 1000 ); /* Pause for 1 second before attempting to reconnect */
    }

    /* We only get here on fatal error */
    return rval;
}


/******************************************************************************
 *                                                                            *
 *  xc_task ()                                                                *
 *                                                                            *
 ******************************************************************************/

static void xc_task( void const* args )
{
    ( void )args;

    printf( "\nXively: Pausing...\n" );

    /*
     * Sleep for 10 seconds to let the network start
     */
    osDelay( 2000 );

    /*
     * Call the Xively "main" loop. This will never return.
     */
    xc_main();

    while ( 1 )
    {
        osDelay( 1000 );
    }
}


/******************************************************************************
 *                                                                            *
 *  xively_client_start ()                                                    *
 *                                                                            *
 ******************************************************************************/

void xively_client_start( void )
{
    osThreadDef( XIVELY, xc_task, osPriorityNormal, 0, XC_TASK_STACK );
    osThreadCreate( osThread( XIVELY ), NULL );
}
