#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "xively.h"

/******************************************************************************
*                          Xively Interface Macros
******************************************************************************/
#define DELAY_MS( t ) vTaskDelay( t / portTICK_PERIOD_MS )
#define XIF_MQTT_TOPIC_MAX_LEN 200

#define XIF_BUTTON_TOPIC_NAME "button"
#define XIF_LED_TOPIC_NAME "led"

#define XIF_SECONDS_BETWEEN_PUBLISHES 1

/******************************************************************************
*                   Xively Interface Function Declarations
******************************************************************************/
/* Static functions */
static int xif_init( void );
static int xif_connect( void );
static void xif_publish_counter( void );
static int xif_build_all_mqtt_topics( void );
static int xif_build_xively_topic( char* topic_name, char* dst, uint32_t dst_len );
static int xif_subscribe( void );
// static int xif_disconnect( void );
static int xif_shutdown( void );

/* Callbacks */
static void
xif_on_connected( xi_context_handle_t in_context_handle, void* data, xi_state_t state );
static void xif_successful_connection_callback( xi_connection_data_t* conn_data );
static void xif_led_topic_callback( xi_context_handle_t in_context_handle,
                                    xi_sub_call_type_t call_type,
                                    const xi_sub_call_params_t* const params,
                                    xi_state_t state,
                                    void* user_data );
static void xif_recv_mqtt_msg_callback( const xi_sub_call_params_t* const params );

/******************************************************************************
*                     Xively Interface Structs and Enums
******************************************************************************/
typedef struct
{
    char button_topic[XIF_MQTT_TOPIC_MAX_LEN];
    char led_topic[XIF_MQTT_TOPIC_MAX_LEN];
} xif_mqtt_topics_t;

typedef struct
{
    char* xi_account_id;
    char* xi_device_id;
    char* xi_device_pwd;
} xif_device_info_t;

typedef enum
{
    XIF_MQTT_UNRECOVERABLE_CONNECTION_ERROR = -1,
    XIF_MQTT_DISCONNECTED = 0,
    XIF_MQTT_CONNECTED
} xif_mqtt_connection_states_t;

typedef enum
{
    XIF_STATE_ERROR = -1, /* Something went very wrong */
    XIF_STATE_UNINITIALIZED = 0, /* Initialize the client -- First state in the machine */
    XIF_STATE_DISCONNECTED, /* Client is not connected -- Re-connect ASAP */
    XIF_STATE_LIBRARY_LOOP, /* Client is connected but idle */
    XIF_STATE_SHUTDOWN /* Shut down the connection and client */
} xif_machine_states_t;

/******************************************************************************
*                           Xively Interface Variables
******************************************************************************/
static xif_device_info_t xif_mqtt_device_info;
static xif_machine_states_t xif_machine_state = XIF_STATE_UNINITIALIZED;
static xi_context_handle_t xif_context_handle = -1;
static xi_timed_task_handle_t xif_pubcounter_scheduled_task_handle = -1;
static xif_mqtt_topics_t xif_mqtt_topics;
static xif_mqtt_connection_states_t xif_mqtt_connection_status = XIF_MQTT_DISCONNECTED;

/******************************************************************************
*                           Xively Interface Implementation
******************************************************************************/
void __attribute__((weak)) xif_state_machine_aborted_callback( void )
{
    printf( "\nThe Xively IF state machine shut down because of a problem" );
    printf( "\nYou should re-implement this function to handle it gracefully" );
}

void xif_set_device_info( char* xi_acc_id, char* xi_dev_id, char* xi_dev_pwd )
{
    xif_mqtt_device_info.xi_account_id = xi_acc_id;
    xif_mqtt_device_info.xi_device_id  = xi_dev_id;
    xif_mqtt_device_info.xi_device_pwd = xi_dev_pwd;
}

/* TODO: Handle error return codes properly */
void* xif_rtos_task( void* param )
{
    int xif_exit_flag = 0;
    assert( NULL != xif_mqtt_device_info.xi_account_id );
    assert( NULL != xif_mqtt_device_info.xi_device_id );
    assert( NULL != xif_mqtt_device_info.xi_device_pwd );

    if( 0 > xif_build_all_mqtt_topics() )
    {
        xif_exit_flag = 1;
    }

    while ( 0 == xif_exit_flag )
    {
        printf( "\nXively Interface state: %d", xif_machine_state );
        switch ( xif_machine_state )
        {
        case XIF_STATE_UNINITIALIZED:
            if ( 0 > xif_init() )
                xif_machine_state = XIF_STATE_ERROR;
            else
                xif_machine_state = XIF_STATE_DISCONNECTED;
            break;

        case XIF_STATE_DISCONNECTED:
            xif_mqtt_connection_status = XIF_MQTT_DISCONNECTED;
            if ( 0 > xif_connect() )
                xif_machine_state = XIF_STATE_ERROR;
            else
                xif_machine_state = XIF_STATE_LIBRARY_LOOP;
            break;

        case XIF_STATE_LIBRARY_LOOP:
            xi_events_process_blocking(); /* Loops until aborted from somewhere else */
            xif_machine_state = XIF_STATE_SHUTDOWN;
            break;

        case XIF_STATE_SHUTDOWN:
            xif_shutdown();
            xif_exit_flag = 1;
            break;

        case XIF_STATE_ERROR:
            printf( "\nXively IF: Unrecoverable error. Shutting down state machine" );
            if( xif_context_handle >= 0 )
                xif_shutdown();
            xif_state_machine_aborted_callback();
            xif_exit_flag = 1;
            break;
        }
    }
    vTaskDelete( NULL );
    return NULL;
}

int xif_build_all_mqtt_topics( void )
{
    if ( 0 > xif_build_xively_topic( XIF_BUTTON_TOPIC_NAME, xif_mqtt_topics.button_topic,
                                     XIF_MQTT_TOPIC_MAX_LEN ) )
    {
        return -1;
    }
    if ( 0 > xif_build_xively_topic( XIF_LED_TOPIC_NAME, xif_mqtt_topics.led_topic,
                                     XIF_MQTT_TOPIC_MAX_LEN ) )
    {
        return -1;
    }
    return 0;
}

int xif_build_xively_topic( char* topic_name, char* dst, uint32_t dst_len )
{
    int retval = 0;

    assert( NULL != topic_name );
    assert( NULL != dst );

    retval = snprintf( dst, dst_len, "xi/blue/v1/%.64s/d/%.64s/%.64s",
                       xif_mqtt_device_info.xi_account_id,
                       xif_mqtt_device_info.xi_device_id, topic_name );

    if ( ( retval >= dst_len ) || ( retval < 0 ) )
    {
        return -1;
    }
    return 0;
}

int xif_init( void )
{
    xi_state_t xi_ret_state = XI_STATE_OK;

    xi_ret_state = xi_initialize( xif_mqtt_device_info.xi_account_id,
                                  xif_mqtt_device_info.xi_device_id );
    if ( XI_STATE_OK != xi_ret_state )
    {
        printf( "\nFailed to initialize MQTT library" );
        return -1;
    }

    xif_context_handle = xi_create_context();
    if ( 0 > xif_context_handle )
    {
        printf( "\nFailed to create a valid libxively context. Returned: %d",
                xif_context_handle );
        return -1;
    }
    return 0;
}

int xif_connect( void )
{
    xi_state_t ret_state = xi_connect(
        xif_context_handle, xif_mqtt_device_info.xi_device_id,
        xif_mqtt_device_info.xi_device_pwd, 10, 0, XI_SESSION_CLEAN, &xif_on_connected );
    if ( XI_STATE_OK != ret_state )
    {
        printf( "\nError formatting MQTT connect request! Retcode: %d", ret_state );
        return -1;
    }
    return 0;
}

int xif_subscribe( void )
{
    xi_state_t ret_state = XI_STATE_OK;
    ret_state            = xi_subscribe( xif_context_handle, xif_mqtt_topics.led_topic,
                              XI_MQTT_QOS_AT_MOST_ONCE, &xif_led_topic_callback, NULL );
    if ( XI_STATE_OK != ret_state )
    {
        return -1;
    }
    return 0;
}

static void xif_publish_counter( void )
{
    static unsigned long msg_counter = 0;
    char msg_counter_s[32];
    xi_state_t ret_state = XI_STATE_OK;
    sprintf( msg_counter_s, "%ld", msg_counter );
    printf( "\nPublishing MQTT message [%s]", msg_counter_s );
    ret_state =
        xi_publish( xif_context_handle, xif_mqtt_topics.button_topic, msg_counter_s,
                    XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
    if ( ( XI_OUT_OF_MEMORY == ret_state ) || ( XI_INTERNAL_ERROR == ret_state ) )
    {
        xif_machine_state = XIF_STATE_ERROR;
        return;
    }
    msg_counter++;
}

static int xif_shutdown( void )
{
    /* TODO: Are there multiple retcodes for these functions? Should I use them? */
    xi_delete_context( xif_context_handle );
    xi_shutdown();
    return 0;
}

static void xif_successful_connection_callback( xi_connection_data_t* conn_data )
{
    if ( 0 > xif_subscribe() )
    {
        xif_machine_state = XIF_STATE_ERROR;
        return;
    }

    /* Schedule the xif_publish_counter function to publish an increasing counter
     * every $XIF_SECONDS_BETWEEN_PUBLISHES seconds */
    xif_pubcounter_scheduled_task_handle = xi_schedule_timed_task(
        xif_context_handle, ( xi_user_task_callback_t* )xif_publish_counter,
        XIF_SECONDS_BETWEEN_PUBLISHES, 1, NULL );

    if ( XI_INVALID_TIMED_TASK_HANDLE == xif_pubcounter_scheduled_task_handle )
    {
        printf( "\nCounter publish scheduled task couldn't be registered" );
    }
}

/* @param params:
 * Full declaration of xi_sub_call_params_t is available in xively_types.h. This
 * function will always be called with the 'message' struct of the union:
 * typedef union xi_sub_call_params_u {
 *     [...]
 *     struct
 *     {
 *         const char* topic;
 *         const uint8_t* temporary_payload_data; // free'd when the callbackreturns
 *         size_t temporary_payload_data_length;
 *         xi_mqtt_retain_t retain;
 *         xi_mqtt_qos_t qos;
 *         xi_mqtt_dup_t dup_flag;
 *     } message;
 * } xi_sub_call_params_t;
 * */
static void xif_recv_mqtt_msg_callback( const xi_sub_call_params_t* const params )
{
    printf( "\n>> received message on topic %s", params->message.topic );
    printf( "\n\t Message size: %d", params->message.temporary_payload_data_length );
    printf( "\n\t Message payload: " );
    for ( size_t i = 0; i < params->message.temporary_payload_data_length; i++ )
    {
        // printf( "0x%02x ", params->message.temporary_payload_data[i] );
        printf( "%c", ( char )params->message.temporary_payload_data[i] );
    }
}

/******************************************************************************
*                           xively-client-c callbacks
******************************************************************************/
/* @brief Connection callback.  See xively.h for full signature information
 *
 * @param in_context_handle the xively connection context as was returned from
 * xi_create_context. This denotes which connection is calling this callback and
 * could be used to differentiate connections if you have more than one ongoing.)
 * This value must be used for all Xively C Client operations.
 * @param data an abstract data pointer that is parsed differently depending on the
 * connection state.  See the source below for example usage.
 * @param state status / error code from xi_error.h that might be applicable
 * to the connection state change.
 * */
void xif_on_connected( xi_context_handle_t in_context_handle,
                       void* data,
                       xi_state_t state )
{
    xi_connection_data_t* conn_data = ( xi_connection_data_t* )data;

    switch ( conn_data->connection_state )
    {
        case XI_CONNECTION_STATE_OPEN_FAILED:
            printf( "\nConnection to %s:%d has failed reason %d", conn_data->host,
                    conn_data->port, state );
            xif_mqtt_connection_status = XIF_MQTT_DISCONNECTED;
            /* TODO: If the error is unrecoverable (e.g. invalid username/pwd), set
               XIF_MQTT_UNERCOVERABLE_CONN_ERROR and do not attempt to re-connect */
            /* Re-attempt to connect until we succeed */
            xif_connect();
            return;

        case XI_CONNECTION_STATE_OPENED:
            printf( "\nXively connected to %s:%d", conn_data->host, conn_data->port );
            xif_mqtt_connection_status = XIF_MQTT_CONNECTED;
            xif_successful_connection_callback( conn_data );
            return;

        case XI_CONNECTION_STATE_CLOSED:
            printf( "\nConnection closed - reason %d!", state );
            xif_mqtt_connection_status = XIF_MQTT_DISCONNECTED;
            return;

        default:
            printf( "\nInvalid callback parameter %d", conn_data->connection_state );
            xif_mqtt_connection_status = XIF_MQTT_UNRECOVERABLE_CONNECTION_ERROR;
            return;
    }
}

void xif_led_topic_callback( xi_context_handle_t in_context_handle,
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
                printf( "\n>> Subscription to LED topic [FAILED]" );
                xif_machine_state = XIF_STATE_ERROR;
                break;
            }
            printf( "\n>> Subscription to LED topic [OK]" );
            break;

        case XI_SUB_CALL_MESSAGE:
            xif_recv_mqtt_msg_callback( params );
            break;

        default:
            break;
    }
}
