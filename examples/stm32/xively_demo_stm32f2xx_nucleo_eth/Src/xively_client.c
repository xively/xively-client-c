/******************************************************************************
 *                                                                            *
 *  xively_client.c                                                           *
 *                                                                            *
 ******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "cmsis_os.h"
#include "xively.h"
#include "xively_client.h"
#include "xi_bsp_rng.h"
#include "xi_bsp_time.h"
#include "demo_io.h"

/******************************************************************************
 *                                                                            *
 *  Macros                                                                    *
 *                                                                            *
 ******************************************************************************/
/*
 *  Application Specific
 */
#define XC_STR_LEN 64
#define XC_CONNECT_TO 0    /* Seconds. Client/Broker connection wait time.   */
#define XC_KEEPALIVE_TO 10 /* Seconds. Client/Broker keep-alive period.      */
#define XC_CHECK_PERIOD 1  /* Seconds. Check period for publication changes. */
#define XC_MAX_PUBS_IN_FLIGHT 1

#define XC_TASK_STACK ( 512 * 6 )
#define XC_TASK_NAME ( ( signed char* )"XC Thread" )

#define XI_ACCOUNT_ID "Xively Account ID"
#define XI_DEVICE_ID "Xively Device ID"
#define XI_PASSWORD "Xively Device Password"

/*
 *  Xively Specific
 */
#define XC_TOPIC_LEN 128

/*
 *  XC_TOPIC_FORMAT
 *
 *  snprintf format for fully qualified topic strings.
 *    arg[0]: account_id
 *    arg[1]: device_id
 *    arg[2]: topic_name
 */
#define XC_TOPIC_FORMAT "xi/blue/v1/%s/d/%s/%s"


#define XC_NULL_CONTEXT ( ( xi_context_handle_t )-1 )
#define XC_NULL_T_HANDLE ( ( xi_timed_task_handle_t )-1 )


/******************************************************************************
 *                                                                            *
 *  Topic config table                                                        *
 *                                                                            *
 *  This table is used to generate the following:                             *
 *                                                                            *
 *    A per topic enum used to identify and access data for each topic.       *
 *                                                                            *
 *    A subscritpion callback wrapper function for each subscribe topic       *
 *    (via XC_TOPIC_SUB_ENTRY). Each wrapper function will call the common    *
 *    subscritpion handler ("subscribe_callback_common") providing its        *
 *    specific topic enum.                                                    *
 *                                                                            *
 *    "xc_topic_info" with an entry for each topic with: topic string,        *
 *     QOS setting, retain setting, and subscription callback function.       *
 *                                                                            *
 ******************************************************************************/

/*                    TOPIC_ENUM,       TOPIC_NAME,      QOS,                       RETAIN
 */

#define XC_TOPIC_TABLE                                                                   \
    XC_TOPIC_PUB_ENTRY( BATTERY_VOLTAGE, "BatteryVoltage", XI_MQTT_QOS_AT_LEAST_ONCE,    \
                        XI_MQTT_RETAIN_FALSE )                                           \
    XC_TOPIC_PUB_ENTRY( PANEL_VOLTAGE, "PanelVoltage", XI_MQTT_QOS_AT_LEAST_ONCE,        \
                        XI_MQTT_RETAIN_FALSE )                                           \
    XC_TOPIC_PUB_ENTRY( CHARGE_CURRENT, "ChargeCurrent", XI_MQTT_QOS_AT_LEAST_ONCE,      \
                        XI_MQTT_RETAIN_FALSE )                                           \
    XC_TOPIC_PUB_ENTRY( STATUS_FLAGS, "StatusFlags", XI_MQTT_QOS_AT_LEAST_ONCE,          \
                        XI_MQTT_RETAIN_FALSE )                                           \
    XC_TOPIC_SUB_ENTRY( QUERY_ALL, "QueryAll", XI_MQTT_QOS_AT_LEAST_ONCE,                \
                        XI_MQTT_RETAIN_FALSE )


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


/******************************************************************************
 *                                                                            *
 *  xc_topic_e                                                                *
 *                                                                            *
 *  Topic enums (TOPIC_ENUM). Generated from XC_TOPIC_TABLE                   *
 *                                                                            *
 ******************************************************************************/

#define XC_TOPIC_SUB_ENTRY( ENUM, NAME, QOS, RETAIN ) ENUM,

#define XC_TOPIC_PUB_ENTRY( ENUM, NAME, QOS, RETAIN ) ENUM,

typedef enum {
    XC_TOPIC_START   = 0,
    XC_TOPIC_INVALID = -1,

    /* Invoke table macro */
    XC_TOPIC_TABLE XC_TOPIC_COUNT,
    XC_PUB_START = BATTERY_VOLTAGE,
    XC_PUB_COUNT = ( STATUS_FLAGS - BATTERY_VOLTAGE + 1 )
} xc_topic_e;


#undef XC_TOPIC_SUB_ENTRY
#undef XC_TOPIC_PUB_ENTRY


/******************************************************************************
 *                                                                            *
 *  xc_topic_info_t                                                           *
 *                                                                            *
 *  Xively topic configuraton and handling info.                              *
 *  Generated from XC_TOPIC_TABLE.                                            *
 *                                                                            *
 *  name                                                                      *
 *    String, name part of fully-qualifed-topic. "TOPIC_NAME"                 *
 *    from "XC_TOPIC_TABLE".                                                  *
 *                                                                            *
 *  qos                                                                       *
 *    QOS setting, "QOS" from "XC_TOPIC_TABLE".                               *
 *                                                                            *
 *  retain                                                                    *
 *    Retain setting, "RETAIN" from "XC_TOPIC_TABLE". Not supported yet.      *
 *                                                                            *
 *  subscribe_cb                                                              *
 *    Generated subcribe callback wrapper function. For "PUB" topics, the     *
 *    value is NULL.                                                          *
 *                                                                            *
 ******************************************************************************/

typedef struct
{
    char* name;
    xi_mqtt_qos_t qos;
    xi_mqtt_retain_t retain;
    subscribe_cb_t* subscribe_cb;
} xc_topic_info_t;


/******************************************************************************
 *                                                                            *
 *  Forward Reference                                                         *
 *                                                                            *
 ******************************************************************************/

static void force_report( void );
static void xively_reset( const xi_context_handle_t ctx );
static void xc_subscribe_next( const xi_context_handle_t ctx );

static xi_state_t subscribe_cb_common( const xi_context_handle_t ctx,
                                       void* data,
                                       xi_state_t state,
                                       xc_topic_e topic_e );


/******************************************************************************
 *                                                                            *
 *  Per topic "subscribe_cb" forward refs (subscribe_cb_ ## ENUM)             *
 *                                                                            *
 ******************************************************************************/

#define XC_TOPIC_SUB_ENTRY( ENUM, NAME, QOS, RETAIN )                                    \
    static xi_state_t subscribe_cb_##ENUM( const xi_context_handle_t ctx, void* data,    \
                                           xi_state_t state );

/* Empty */
#define XC_TOPIC_PUB_ENTRY( ENUM, NAME, QOS, RETAIN )


/* Invoke table macro */
XC_TOPIC_TABLE


#undef XC_TOPIC_SUB_ENTRY
#undef XC_TOPIC_PUB_ENTRY


/******************************************************************************
 *                                                                            *
 *  xc_topic_info[]                                                           *
 *                                                                            *
 *  Topic Info Declaration/Initialization. Generated from XC_TOPIC_TABLE      *
 *                                                                            *
 ******************************************************************************/

#define XC_TOPIC_SUB_ENTRY( ENUM, NAME, QOS, RETAIN )                                    \
    {                                                                                    \
        .name = NAME, .qos = QOS, .retain = RETAIN, .subscribe_cb = subscribe_cb_##ENUM, \
    },

#define XC_TOPIC_PUB_ENTRY( ENUM, NAME, QOS, RETAIN )                                    \
    {.name = NAME, .qos = QOS, .retain = RETAIN, .subscribe_cb = NULL},


/* Invoke table macro */
static xc_topic_info_t xc_topic_info[] = {XC_TOPIC_TABLE};


#undef XC_TOPIC_SUB_ENTRY
#undef XC_TOPIC_PUB_ENTRY


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
 *  Credential values
 */
static char* account_id;
static char* device_id;
static char* password;

/*
 *  Concurrent publication control
 */
static int xc_pubs_in_flight = 0;

/*
 *  Subscription control
 */
static xc_topic_e xc_subscribe_next_topic = XC_TOPIC_START;


/*
 *  Data publication vars
 */
static int poll_countdown;
// static float  report_data[MS_VAL_COUNT];
static int report_flags;
static int report_pending[XC_PUB_COUNT];


/******************************************************************************
 *                                                                            *
 *  Per topic "subscribe_cb" functions (subscribe_cb_ ## ENUM)                *
 *  Generated from XC_TOPIC_TABLE                                             *
 *                                                                            *
 ******************************************************************************/

#define XC_TOPIC_SUB_ENTRY( ENUM, NAME, QOS, RETAIN )                                    \
    static xi_state_t subscribe_cb_##ENUM( const xi_context_handle_t ctx, void* data,    \
                                           xi_state_t state )                            \
    {                                                                                    \
        return subscribe_cb_common( ctx, data, state, ENUM );                            \
    }

#define XC_TOPIC_PUB_ENTRY( ENUM, NAME, QOS, RETAIN ) /* Empty */

/* Invoke table macro */
XC_TOPIC_TABLE

#undef XC_TOPIC_SUB_ENTRY
#undef XC_TOPIC_PUB_ENTRY


/******************************************************************************
 *                                                                            *
 *  subscribe_cb_common ()                                                    *
 *                                                                            *
 *  Single common subscription callback function. The specific topic being    *
 *  handled is specified by "topic_e".                                        *
 *                                                                            *
 ******************************************************************************/

static xi_state_t subscribe_cb_common( const xi_context_handle_t ctx,
                                       void* data,
                                       xi_state_t state,
                                       xc_topic_e topic_e )
{
    xc_topic_info_t* t_info = xc_topic_info + topic_e;
    xi_mqtt_suback_status_t status;
    //    char* msg_ptr;
    //    int msg_val;
    xi_state_t rval;

    ( void )ctx;

    switch ( state )
    {
        case XI_MQTT_SUBSCRIPTION_FAILED:
        {
            printf( "Xively: Subscribe failed \"%s\".\n", t_info->name );
            rval = XI_STATE_OK;
            xc_subscribe_next( ctx ); /* Subscribe to next topic */
            break;
        }

        case XI_MQTT_SUBSCRIPTION_SUCCESSFULL:
        {
            status = ( xi_mqtt_suback_status_t )( intptr_t )data;
            printf( "Xively: Subscribe OK \"%s\", QoS %d\n", t_info->name, status );
            rval = XI_STATE_OK;
            xc_subscribe_next( ctx ); /* Subscribe to next topic */
            break;
        }

        case XI_STATE_OK:
        {
            rval = XI_STATE_OK;

#if 0
            xi_mqtt_message_t *msg = (xi_mqtt_message_t*)data;

            if (!msg->publish.content || !msg->publish.content->data_ptr)
              {
                break;
              }

            msg_ptr = (char *)msg->publish.content->data_ptr;

              /*
               *  When "QueryAll" received, force publication.
               */
            if (topic_e == QUERY_ALL)
              {

                sscanf (msg_ptr, "%d", &msg_val);

                switch (msg_val)
                  {
                    case 1:
                      force_report ();
                      break;

                    case 2:
                      test_alloc_report (-1);
                      break;

                    case 3:
                      xively_reset (ctx);
                      break;

                    default:
                      break;
                  }
              }

            xi_mqtt_message_free (&msg);
#endif
            break;
        }

        default:
            rval = state;
            break;
    }

    return rval;
}


/******************************************************************************
 *                                                                            *
 *  force_report ()                                                           *
 *                                                                            *
 *  Force a report of all parameters, by seting "report_pending".             *
 *                                                                            *
 ******************************************************************************/

static void force_report( void )
{
    xc_topic_e e;

    for ( e = XC_PUB_START; e < XC_PUB_COUNT; e++ )
    {
        report_pending[e] = 1;
    }
}


/******************************************************************************
 *                                                                            *
 *  xively_reset
 *                                                                            *
 ******************************************************************************/

static void xively_reset( const xi_context_handle_t ctx )
{
    printf( "Xively: Reset\n" );
    xi_shutdown_connection( ctx );
}


/******************************************************************************
 *                                                                            *
 *  publish_cb ()                                                             *
 *                                                                            *
 ******************************************************************************/

static xi_state_t
publish_cb( const xi_context_handle_t ctx, void* data, xi_state_t state )
{
    xc_topic_e cb_topic;
    const xc_topic_info_t* t_info;

    ( void )ctx;

    cb_topic = ( xc_topic_e )( ( long )data );
    t_info   = xc_topic_info + cb_topic;
    xc_pubs_in_flight--;

    if ( state == XI_STATE_OK )
    {
        printf( "Xively: Publish OK %s\n", t_info->name );
    }
    else
    {
        printf( "xively: Publish Error(%d) %s\n", state, t_info->name );
    }

    return XI_STATE_OK;
}

/******************************************************************************
 *                                                                            *
 *  monitor_cb ()                                                             *
 *                                                                            *
 *  Monitoring callback function                                              *
 *                                                                            *
 ******************************************************************************/

static void
monitor_cb( const xi_context_handle_t ctx, const xi_timed_task_handle_t task, void* data )
{
    ( void )task;
    ( void )data;

    if ( xc_control > 0 )
    {
        if ( xc_control == 1 )
        {
            xively_reset( ctx );
        }

        xc_control = 0;
    }

    monitor_cb_hdl = xi_schedule_timed_task( ctx, monitor_cb, XC_CHECK_PERIOD, 0, NULL );
}

/******************************************************************************
 *                                                                            *
 *  xc_subscribe_next ()                                                      *
 *                                                                            *
 *  Subscribe to one topic at a time.                                         *
 *                                                                            *
 ******************************************************************************/

static void xc_subscribe_next( const xi_context_handle_t ctx )
{
    const xc_topic_info_t* t_info;
    char fq_topic[XC_TOPIC_LEN];
    xc_topic_e e;

    /*
     *  Subscribe to topics
     */
    for ( e = xc_subscribe_next_topic; e < XC_TOPIC_COUNT; e++ )
    {
        t_info = xc_topic_info + e;

        if ( t_info->subscribe_cb )
        {
            snprintf( fq_topic, XC_TOPIC_LEN, XC_TOPIC_FORMAT, account_id, device_id,
                      t_info->name );

            printf( "Xively: Subscribe Pending \"%s\".\n", t_info->name );
            xi_subscribe( ctx, fq_topic, t_info->qos, t_info->subscribe_cb, 0 );
            e++;
            xc_subscribe_next_topic = e;
            break;
        }
    }
}

#if 0 // sensor feature

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

/* combines name with the account and device id */
#define XI_TOPIC_NAME_MANGLE( name )                                                     \
    "xi/blue/v1/" XI_ACCOUNT_ID "/d/" XI_DEVICE_ID "/" name

/* declaration of topic handlers for SUB'ed topics */
static void on_led_msg( const uint8_t* const msg, size_t msg_size );

/* declaration of topic handlers for PUB'ed topics */
static xi_state_t pub_accelerometer( const mqtt_topic_descr_t* const mqtt_topic_descr );
static xi_state_t pub_magnetometer( const mqtt_topic_descr_t* const mqtt_topic_descr );
static xi_state_t pub_barometer( const mqtt_topic_descr_t* const mqtt_topic_descr );
static xi_state_t pub_button( const mqtt_topic_descr_t* const mqtt_topic_descr );
static xi_state_t pub_gyroscope( const mqtt_topic_descr_t* const mqtt_topic_descr );
static xi_state_t pub_humidity( const mqtt_topic_descr_t* const mqtt_topic_descr );
static xi_state_t pub_temperature( const mqtt_topic_descr_t* const mqtt_topic_descr );

/* topic's initialisation function */
static xi_state_t init_xively_topics( xi_context_handle_t in_context_handle );

/* table of topics used for this demo */
static const mqtt_topic_configuration_t topics_array[] = {
    {{XI_TOPIC_NAME_MANGLE( "Accelerometer" )}, NULL, pub_accelerometer},
    {{XI_TOPIC_NAME_MANGLE( "Magnetometer" )}, NULL, pub_magnetometer},
    {{XI_TOPIC_NAME_MANGLE( "Barometer" )}, NULL, pub_barometer},
    {{XI_TOPIC_NAME_MANGLE( "Gyroscope" )}, NULL, pub_gyroscope},
    {{XI_TOPIC_NAME_MANGLE( "Humidity" )}, NULL, pub_humidity},
    {{XI_TOPIC_NAME_MANGLE( "Button" )}, NULL, pub_button},
    {{XI_TOPIC_NAME_MANGLE( "LED" )}, &on_led_msg, NULL},
    {{XI_TOPIC_NAME_MANGLE( "Temperature" )}, NULL, pub_temperature},
};

/* store the length of the array */
static const size_t topics_array_length =
    sizeof( topics_array ) / sizeof( topics_array[0] );

xi_context_handle_t gXivelyContextHandle      = XI_INVALID_CONTEXT_HANDLE;
xi_timed_task_handle_t gXivelyTimedTaskHandle = XI_INVALID_TIMED_TASK_HANDLE;

/* handler for led msg */
void on_led_msg( const uint8_t* const msg, size_t msg_size )
{
    printf( "\r\n>> Got a new MQTT message in the LED topic" );
    printf( "\r\n\t Message size: %d", msg_size );
    printf( "\r\n\t Message payload: " );
    for ( size_t i = 0; i < msg_size; i++ )
    {
        printf( "0x%02x ", msg[i] );
    }
    if ( msg_size != 1 )
    {
        printf( "\r\n\tUnrecognized LED message [IGNORED] Expected 1 or 0" );
        return;
    }
    ( msg[0] == '0' ) ? io_led_off() : io_led_on();
}

xi_state_t pub_accelerometer( const mqtt_topic_descr_t* const mqtt_topic_descr )
{
    char out_msg[IO_AXES_JSON_BUFFER_MAX_SIZE] = "";
    SensorAxes_t sensor_input;

    if ( io_read_accelero( &sensor_input ) < 0 )
    {
        printf( "\r\n\t[ERROR] trying to read accelerometer input" );
        return -1;
    }
    if ( io_axes_to_json( sensor_input, out_msg, IO_AXES_JSON_BUFFER_MAX_SIZE ) < 0 )
    {
        printf( "\r\n\t[ERROR] trying to create JSON string from sensor input" );
        return -1;
    }
    return xi_publish( gXivelyContextHandle, mqtt_topic_descr->name, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_magnetometer( const mqtt_topic_descr_t* const mqtt_topic_descr )
{
    char out_msg[IO_AXES_JSON_BUFFER_MAX_SIZE] = "";
    SensorAxes_t sensor_input;

    if ( io_read_magneto( &sensor_input ) < 0 )
    {
        printf( "\r\n\t[ERROR] trying to read magnetometer input" );
        return -1;
    }
    if ( io_axes_to_json( sensor_input, out_msg, IO_AXES_JSON_BUFFER_MAX_SIZE ) < 0 )
    {
        printf( "\r\n\t[ERROR] trying to create JSON string from sensor input" );
        return -1;
    }
    return xi_publish( gXivelyContextHandle, mqtt_topic_descr->name, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_gyroscope( const mqtt_topic_descr_t* const mqtt_topic_descr )
{
    char out_msg[IO_AXES_JSON_BUFFER_MAX_SIZE] = "";
    SensorAxes_t sensor_input;

    if ( io_read_gyro( &sensor_input ) < 0 )
    {
        printf( "\r\n\t[ERROR] trying to read gyroscope input" );
        return -1;
    }
    if ( io_axes_to_json( sensor_input, out_msg, IO_AXES_JSON_BUFFER_MAX_SIZE ) < 0 )
    {
        printf( "\r\n\t[ERROR] trying to create JSON string from sensor input" );
        return -1;
    }
    return xi_publish( gXivelyContextHandle, mqtt_topic_descr->name, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_barometer( const mqtt_topic_descr_t* const mqtt_topic_descr )
{
    char out_msg[IO_FLOAT_BUFFER_MAX_SIZE] = "";
    float sensor_input                     = 0;
    if ( io_read_pressure( &sensor_input ) < 0 )
    {
        printf( "\r\n\t[ERROR] trying to read barometer input" );
        return -1;
    }
    if ( io_float_to_string( sensor_input, out_msg, IO_FLOAT_BUFFER_MAX_SIZE ) )
    {
        printf( "\r\n\t[ERROR] trying to create string from sensor input" );
        return -1;
    }
    return xi_publish( gXivelyContextHandle, mqtt_topic_descr->name, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_humidity( const mqtt_topic_descr_t* const mqtt_topic_descr )
{
    char out_msg[IO_FLOAT_BUFFER_MAX_SIZE] = "";
    float sensor_input                     = 0;
    if ( io_read_humidity( &sensor_input ) < 0 )
    {
        printf( "\r\n\t[ERROR] trying to read hygrometer input" );
        return -1;
    }
    if ( io_float_to_string( sensor_input, out_msg, IO_FLOAT_BUFFER_MAX_SIZE ) )
    {
        printf( "\r\n\t[ERROR] trying to create string from sensor input" );
        return -1;
    }
    return xi_publish( gXivelyContextHandle, mqtt_topic_descr->name, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_temperature( const mqtt_topic_descr_t* const mqtt_topic_descr )
{
    char out_msg[IO_FLOAT_BUFFER_MAX_SIZE] = "";
    float sensor_input                     = 0;
    if ( io_read_temperature( &sensor_input ) < 0 )
    {
        printf( "\r\n\t[ERROR] trying to read thermometer input" );
        return -1;
    }
    if ( io_float_to_string( sensor_input, out_msg, IO_FLOAT_BUFFER_MAX_SIZE ) )
    {
        printf( "\r\n\t[ERROR] trying to create string from sensor input" );
        return -1;
    }
    return xi_publish( gXivelyContextHandle, mqtt_topic_descr->name, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_button( const mqtt_topic_descr_t* const mqtt_topic_descr )
{
    char* payload;
    ( io_read_button() ) ? ( payload = "1" ) : ( payload = "0" );
    return xi_publish( gXivelyContextHandle, mqtt_topic_descr->name, payload,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

void pub_button_interrupt( void )
{
    /* TODO: Verify we are connected to Xively before calling xi_publish */
    const char payload[] = "1";
    xi_publish( gXivelyContextHandle, XI_TOPIC_NAME_MANGLE( "Button" ), payload,
                XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

#endif


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
            printf( "[%lu] Xively: Connection failed to %s:%d, error %d\n",
                    xi_bsp_time_getcurrenttime_seconds(), conn_data->host,
                    conn_data->port, state );

            xi_connect( ctx, conn_data->username, conn_data->password,
                        conn_data->connection_timeout, conn_data->keepalive_timeout,
                        conn_data->session_type, &connect_cb );

            rval = XI_STATE_OK;
            break;
        }


        /*
         *  Preiviously open connection has CLOSED
         */
        case XI_CONNECTION_STATE_CLOSED:
        {
            printf( "[%lu] Xively: Connection closed, error %d\n",
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
            printf( "[%lu] Xively: Connected %s:%d\n",
                    xi_bsp_time_getcurrenttime_seconds(), conn_data->host,
                    conn_data->port );
            rval = XI_STATE_OK;

            //            init_xively_topics( ctx );

            break;
        }


        default:
        {
            printf( "[%lu] Xively: Connection invalid, error %d\n",
                    xi_bsp_time_getcurrenttime_seconds(), conn_data->connection_state );
            rval = XI_INVALID_PARAMETER;
            break;
        }
    }

    return rval;
}

#if 0 // sensor feature

/* handler for xively mqtt topic subscription */
void on_mqtt_topic( xi_context_handle_t in_context_handle,
                    xi_sub_call_type_t call_type,
                    const xi_sub_call_params_t* const params,
                    xi_state_t state,
                    void* user_data )
{
    /* sanity checks */
//    assert( NULL != user_data );

    /* user data is the mqtt_topic_configuration_t */
    const mqtt_topic_configuration_t* topic_conf =
        ( const mqtt_topic_configuration_t* )user_data;

    switch ( call_type )
    {
        case XI_SUB_CALL_SUBACK:
            if ( XI_MQTT_SUBACK_FAILED == params->suback.suback_status )
            {
                printf( "Subscription failed for %s topic\r\n", params->suback.topic );
            }
            else
            {
                printf( "Subscription successful for %s topic\r\n",
                        params->suback.topic );
            }
            return;
        case XI_SUB_CALL_MESSAGE:
            printf( "received message on %s topic\r\n", params->suback.topic );
            /* if there is a handler call it with the message payload */
            if ( NULL != topic_conf->on_msg_pull )
            {
                topic_conf->on_msg_pull( params->message.temporary_payload_data,
                                         params->message.temporary_payload_data_length );
            }
            break;
        default:
            return;
    }
}

void push_mqtt_topics()
{
    int i = 0;
    for ( ; i < topics_array_length; ++i )
    {
        const mqtt_topic_configuration_t* const topic_config = &topics_array[i];

        if ( NULL != topic_config->on_msg_push )
        {
            topic_config->on_msg_push( &topic_config->topic_descr );
        }
    }
}

/*
 * Initializes Xively's demo application topics
 */
xi_state_t init_xively_topics( xi_context_handle_t in_context_handle )
{
    xi_state_t ret = XI_STATE_OK;

    int i = 0;
    for ( ; i < topics_array_length; ++i )
    {
        const mqtt_topic_configuration_t* const topic_config = &topics_array[i];

        if ( NULL != topic_config->on_msg_pull )
        {
            ret = xi_subscribe( in_context_handle, topic_config->topic_descr.name,
                                XI_MQTT_QOS_AT_MOST_ONCE, &on_mqtt_topic,
                                ( void* )topic_config );

            if ( XI_STATE_OK != ret )
            {
                return ret;
            }
        }
    }

    /* registration of the publish function */
    gXivelyTimedTaskHandle = xi_schedule_timed_task( in_context_handle, &push_mqtt_topics,
                                                     XI_PUBLISH_INTERVAL_SEC, 1, NULL );

    if ( XI_INVALID_TIMED_TASK_HANDLE == gXivelyTimedTaskHandle )
    {
        printf( "timed task couldn't been registered\r\n" );
        return XI_FAILED_INITIALIZATION;
    }

    return ret;
}

#endif // sensor feature

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
    device_id  = XI_DEVICE_ID;
    password   = XI_PASSWORD;
    account_id = XI_ACCOUNT_ID;

    rval = 0;

    while ( 1 )
    {
        printf( "\nXively: ...continue.\n" );
        //        test_alloc_report (-1);

        /*
         *  Initialize xively client library.
         */
        if ( ( xi_rc = xi_initialize( account_id, device_id ) ) == XI_STATE_OK )
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
        printf( " deviceId \n  %s\n", device_id );
        printf( " accountId\n  %s\n", account_id );


        xi_rc = xi_connect( xc_ctx, device_id, password, XC_CONNECT_TO, XC_KEEPALIVE_TO,
                            XI_SESSION_CLEAN, &connect_cb );

        /*  Loop forever */
        xi_events_process_blocking();

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

    printf( "Xively: Pausing...\n" );

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
