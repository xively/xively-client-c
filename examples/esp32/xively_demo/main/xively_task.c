/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client codebase,
 * it is licensed under the BSD 3-Clause license.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "xively_if.h"

/******************************************************************************
*                          Xively Interface Macros
******************************************************************************/
#define DELAY_MS( t ) vTaskDelay( t / portTICK_PERIOD_MS )

#define XIF_BUTTON_TOPIC_NAME "Button"
#define XIF_LED_TOPIC_NAME "LED"

/******************************************************************************
*                     Xively Interface Structs and Enums
******************************************************************************/
typedef struct
{
    char* xi_account_id;
    char* xi_device_id;
    char* xi_device_pwd;
} xif_device_info_t;

typedef enum
{
    XIF_MQTT_DISCONNECTED = 0,
    XIF_MQTT_CONNECTED
} xif_mqtt_connection_states_t;

typedef enum
{
    XIF_STATE_UNINITIALIZED = 0, /* Initialize the client -- First state in the machine */
    XIF_STATE_LIBRARY_LOOP, /* Client is connected but idle */
    XIF_STATE_HANDLE_ACTION_REQUESTS, /* Check requests to the event loop */
    XIF_STATE_PAUSED, /* Client is disconnected and awaiting an external request to continue */
    XIF_STATE_SHUTDOWN /* Shut down the connection and client */
} xif_machine_states_t;

/******************************************************************************
*                   Xively Interface Function Declarations
******************************************************************************/
static int8_t xif_init( void );
static int8_t xif_build_xively_topic( char* topic_name, char* dst, uint32_t dst_len );

/* Application specific functions */
static int8_t xif_build_all_mqtt_topics( void );
static int8_t xif_subscribe( void );
static int8_t xif_start_timed_tasks( void );
static void xif_cancel_timed_tasks( void );

/* Callbacks */
static void xif_on_connected( xi_context_handle_t in_context_handle,
                              void* data, xi_state_t state );
static void xif_successful_connection_callback( xi_connection_data_t* conn_data );
static void xif_led_topic_callback( xi_context_handle_t in_context_handle,
                                    xi_sub_call_type_t call_type,
                                    const xi_sub_call_params_t* const params,
                                    xi_state_t state,
                                    void* user_data );

/******************************************************************************
*                           Xively Interface Variables
******************************************************************************/
/* MQTT */
static xif_mqtt_connection_states_t xif_mqtt_connection_status = XIF_MQTT_DISCONNECTED;
static xif_device_info_t xif_mqtt_device_info;
xif_mqtt_topics_t xif_mqtt_topics;

/* libxively Handles */
static xi_context_handle_t xif_context_handle = -1;

/* FreeRTOS Handles */
static EventGroupHandle_t xif_requests_event_group_handle = NULL;

/* PUB_INCREASING_COUNTER can be set to create a scheduled task that will publish an
 * increasing counter to the /Button topic every $COUNTER_PUBLISH_PERIOD seconds */
#define PUB_INCREASING_COUNTER 0
#if PUB_INCREASING_COUNTER
#define COUNTER_PUBLISH_PERIOD 10
static void xif_publish_counter( void );
static xi_timed_task_handle_t xif_pubcounter_scheduled_task_handle = -1;
#endif

/*-----------------------------------------------------------------------------
                            Xively Interface Implementation
-----------------------------------------------------------------------------*/
void __attribute__((weak)) xif_state_machine_aborted_callback( void )
{
    printf( "\n[XIF] The Xively IF state machine and RTOS task are about to shut" );
    printf( " down due to an unrecoverable error" );
    printf( "\n[XIF] You should re-implement this function to handle it gracefully" );
}

int8_t xif_set_device_info( char* xi_acc_id, char* xi_dev_id, char* xi_dev_pwd )
{
    xif_mqtt_device_info.xi_account_id = xi_acc_id;
    xif_mqtt_device_info.xi_device_id  = xi_dev_id;
    xif_mqtt_device_info.xi_device_pwd = xi_dev_pwd;
    if( 0 > xif_build_all_mqtt_topics() )
    {
        printf( "\n[XIF] Unrecoverable building MQTT topic strings. Abort" );
        return -1;
    }
    return 0;
}

/*-----------------------------------------------------------------------------
                           FreeRTOS-specific implementation
-----------------------------------------------------------------------------*/
int8_t xif_set_request_bits( xif_action_requests_t requested_action )
{
    BaseType_t higher_priority_task_woken, evt_group_set_result;

    /* xHigherPriorityTaskWoken must be initialised to pdFALSE. */
    higher_priority_task_woken = pdFALSE;

    evt_group_set_result = xEventGroupSetBitsFromISR(
        xif_requests_event_group_handle, requested_action, &higher_priority_task_woken );

    /* Was the message posted successfully? */
    if ( evt_group_set_result == pdFAIL )
    {
        return -1;
    }
    else if( higher_priority_task_woken == pdTRUE )
    {
        /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
        switch should be requested.  The macro used is port specific and will
        be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
        the documentation page for the port being used. */
        portYIELD_FROM_ISR();
    }
    return 0;
}

int8_t xif_clear_request_bits( xif_action_requests_t requested_action )
{
    BaseType_t result;
    result =
        xEventGroupClearBitsFromISR( xif_requests_event_group_handle, requested_action );
    if ( pdPASS != result )
    {
        return -1;
    }
    return 0;
}

int8_t xif_request_machine_state( xif_action_requests_t requested_action )
{
    if ( 0 > xif_context_handle )
    {
        return -1;
    }

    switch( requested_action )
    {
    case XIF_REQUEST_CONTINUE:
        if ( 0 > xif_clear_request_bits( XIF_REQUEST_PAUSE ) )
        {
            return -1;
        }
        if ( 0 > xif_set_request_bits( XIF_REQUEST_CONTINUE ) )
        {
            return -1;
        }
        break;
    case XIF_REQUEST_PAUSE:
        if ( 0 > xif_clear_request_bits( XIF_REQUEST_CONTINUE ) )
        {
            return -1;
        }
        if ( 0 > xif_set_request_bits( XIF_REQUEST_PAUSE ) )
        {
            return -1;
        }
        break;
    case XIF_REQUEST_SHUTDOWN:
        if ( 0 > xif_set_request_bits( XIF_REQUEST_SHUTDOWN ) )
        {
            return -1;
        }
        break;
    case XIF_REQUEST_ALL:
        return -1;
    }
    return 0;
}

/* Reads the thread-safe requests bitmask, gets the next highest priority request
 * (the highest bit in the bitmask), clears that bit and returns its corresponding
 * xif_action_requests_t enum value.
 * The XIF_REQUEST_CONTINUE flag is ignored, and it will be returned whenever
 * the rest of the flags are 0
 */
xif_action_requests_t xif_pop_highest_priority_request( void )
{
    EventBits_t external_request_bitmask = 0x00;
    external_request_bitmask = xEventGroupGetBits( xif_requests_event_group_handle );

    if ( XIF_REQUEST_SHUTDOWN & external_request_bitmask )
    {
        xEventGroupClearBits( xif_requests_event_group_handle, XIF_REQUEST_SHUTDOWN );
        return XIF_REQUEST_SHUTDOWN;
    }
    else if ( XIF_REQUEST_PAUSE & external_request_bitmask )
    {
        xEventGroupClearBits( xif_requests_event_group_handle, XIF_REQUEST_PAUSE );
        return XIF_REQUEST_PAUSE;
    }

    return XIF_REQUEST_CONTINUE;
}

static inline void xif_await_unpause( void )
{
    EventBits_t evt_group_bits = 0x00;
    do
    {
        evt_group_bits = xEventGroupWaitBits( xif_requests_event_group_handle,
                                              XIF_REQUEST_CONTINUE | XIF_REQUEST_SHUTDOWN,
                                              false, true, portMAX_DELAY );
    } while( 0x00 == evt_group_bits );
    /* Break when either _CONTINUE or _SHUTDOWN are requested */
}

void xif_handle_unrecoverable_error( void )
{
    xif_state_machine_aborted_callback();
    xif_request_machine_state( XIF_REQUEST_SHUTDOWN );
}

/*-----------------------------------------------------------------------------
                           Events loop
-----------------------------------------------------------------------------*/
void xif_rtos_task( void* param )
{
    int evt_loop_exit_flag = 0;
    ( void )param;
    assert( NULL != xif_mqtt_device_info.xi_account_id );
    assert( NULL != xif_mqtt_device_info.xi_device_id );
    assert( NULL != xif_mqtt_device_info.xi_device_pwd );

    if ( 0 > xif_init() )
    {
        xif_handle_unrecoverable_error();
    }
    else if ( 0 > xif_clear_request_bits( XIF_REQUEST_ALL ) )
    {
        xif_handle_unrecoverable_error();
    }
    else if ( 0 > xif_connect() )
    {
        xif_handle_unrecoverable_error();
    }

    /* Events loop */
    while ( 0 == evt_loop_exit_flag )
    {
        switch( xif_pop_highest_priority_request() )
        {
        case XIF_REQUEST_CONTINUE:
            xif_request_machine_state( XIF_REQUEST_CONTINUE );
            xi_events_process_tick();
            break;
        case XIF_REQUEST_PAUSE:
            /* Halt this task until we get a XIF_REQUEST_CONTINUE request */
            printf( "\n[XIF] Pausing Xively Interface..." );
            xif_await_unpause();
            break;
        case XIF_REQUEST_SHUTDOWN:
            /* Free all the memory allocated by xif or libxively, shut down the
             * events loop, and delete the RTOS task */
            if( xif_context_handle >= 0 )
            {
                xi_delete_context( xif_context_handle );
            }
            xi_shutdown();
            evt_loop_exit_flag = 1;
            break;
        case XIF_REQUEST_ALL:
            /* This will never happen. xif_pop_highest_priority won't return it.
             * If all bits are set, they will be processed in order (this state
             * machine will shut down)  */
            assert( 0 );
        }
        taskYIELD();
    }

    vTaskDelete( NULL );
    return NULL;
}

int8_t xif_build_all_mqtt_topics( void )
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

int8_t xif_build_xively_topic( char* topic_name, char* dst, uint32_t dst_len )
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

int8_t xif_init( void )
{
    xi_state_t xi_ret_state = XI_STATE_OK;

    xif_requests_event_group_handle = xEventGroupCreate();
    if( NULL == xif_requests_event_group_handle )
    {
        printf( "\n[XIF] Failed to allocate FreeRTOS heap for the requests event group" );
        return -1;
    }

    xi_ret_state = xi_initialize( xif_mqtt_device_info.xi_account_id,
                                  xif_mqtt_device_info.xi_device_id );
    if ( XI_STATE_OK != xi_ret_state )
    {
        printf( "\n[XIF] Failed to initialize MQTT library" );
        return -1;
    }

    xif_context_handle = xi_create_context();
    if ( 0 > xif_context_handle )
    {
        printf( "\n[XIF] Failed to create a valid libxively context. Returned: %d",
                xif_context_handle );
        return -1;
    }
    return 0;
}

/* @retval: -1: xi_connect failed
 *           0: 0 xi_connect OK
 *           1: xi_connect failed because the connection was already initialized
 */
int8_t xif_connect( void )
{
    xi_state_t ret_state = XI_STATE_OK;
    printf( "\n[XIF] Connecting to MQTT broker:" );
    printf( "\n[XIF] \tAccount ID: %s", xif_mqtt_device_info.xi_account_id );
    printf( "\n[XIF] \tDevice  ID: %s", xif_mqtt_device_info.xi_device_id );
    printf( "\n[XIF] \tAccount Password: (REDACTED)" );
    ret_state = xi_connect( xif_context_handle, xif_mqtt_device_info.xi_device_id,
                            xif_mqtt_device_info.xi_device_pwd, 10, 0, XI_SESSION_CLEAN,
                            &xif_on_connected );

    if( XI_ALREADY_INITIALIZED == ret_state )
    {
        return 1;
    }
    else if ( XI_STATE_OK != ret_state )
    {
        printf( "\n[XIF] Error in xi_connect! Retcode: %d", ret_state );
        return -1;
    }
    return 0;
}

int8_t xif_subscribe( void )
{
    xi_state_t ret_state = XI_STATE_OK;
    printf( "\n[XIF] Subscribing to MQTT topics" );
    ret_state            = xi_subscribe( xif_context_handle, xif_mqtt_topics.led_topic,
                              XI_MQTT_QOS_AT_MOST_ONCE, &xif_led_topic_callback, NULL );
    if ( XI_STATE_OK != ret_state )
    {
        return -1;
    }
    return 0;
}

void xif_publish_button_state( int button_state )
{
    char msg_payload[2] = "";
    if( !xif_is_connected() )
    {
        return;
    }

    /* GPIO0 is pulled-up, so the input level is 0 when pressed. If your backend
    expects 1 to mean "button pressed", you can negate the button input here */
    ( button_state == 1 ) ? ( strcpy( msg_payload, "1" ) )
                          : ( strcpy( msg_payload, "0" ) );
    printf( "\n[XIF] Publishing button pressed MQTT message" );
    xi_publish( xif_context_handle, xif_mqtt_topics.button_topic, msg_payload,
                XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

void xif_publish_counter( void )
{
    static unsigned long msg_counter = 0;
    char msg_counter_s[32];
    xi_state_t ret_state = XI_STATE_OK;

    if( !xif_is_connected() )
    {
        return;
    }

    msg_counter++;
    sprintf( msg_counter_s, "%ld", msg_counter );
    printf( "\n[XIF] Publishing MQTT message [%s]", msg_counter_s );

    ret_state =
        xi_publish( xif_context_handle, xif_mqtt_topics.button_topic, msg_counter_s,
                    XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
    if ( ( XI_OUT_OF_MEMORY == ret_state ) || ( XI_INTERNAL_ERROR == ret_state ) )
    {
        xif_handle_unrecoverable_error();
    }
}

int8_t xif_start_timed_tasks( void )
{
#if PUB_INCREASING_COUNTER
    /* Schedule the xif_publish_counter function to publish an increasing counter
     * every $COUNTER_PUBLISH_PERIOD seconds */
    xif_pubcounter_scheduled_task_handle = xi_schedule_timed_task(
        xif_context_handle, ( xi_user_task_callback_t* )xif_publish_counter,
        COUNTER_PUBLISH_PERIOD, 1, NULL );
    if ( XI_INVALID_TIMED_TASK_HANDLE == xif_pubcounter_scheduled_task_handle )
    {
        printf( "\n[XIF] Counter publish scheduled task couldn't be registered" );
        return -1;
    }
#endif /* PUB_INCREASING_COUNTER */
    return 0;
}

void xif_cancel_timed_tasks( void )
{
    xif_cancel_timed_tasks();
#if PUB_INCREASING_COUNTER
    xif_pubcounter_scheduled_task_handle = XI_INVALID_TIMED_TASK_HANDLE;
#endif
}

void xif_successful_connection_callback( xi_connection_data_t* conn_data )
{
    xif_mqtt_connection_status = XIF_MQTT_CONNECTED;
    printf( "\n[XIF] MQTT connected to %s:%d", conn_data->host, conn_data->port );
    if ( 0 > xif_subscribe() )
    {
        xif_handle_unrecoverable_error();
        return;
    }

    xif_start_timed_tasks();
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
 */
void __attribute__( ( weak ) )
xif_recv_mqtt_msg_callback( const xi_sub_call_params_t* const params )
{
    printf( "\n[XIF] New MQTT message received!" );
    printf( "\n[XIF]\tTopic: %s", params->message.topic );
    printf( "\n[XIF]\tMessage size: %d", params->message.temporary_payload_data_length );
    printf( "\n[XIF]\tMessage payload: " );
    for ( size_t i = 0; i < params->message.temporary_payload_data_length; i++ )
    {
        printf( "%c", ( char )params->message.temporary_payload_data[i] );
    }
}

/* @retval 0: Disconnect message sent - Interface will be paused from on_connected
 *            callback
 *         1: MQTT was not connected - Interface shutdown initiated
 */
int8_t xif_disconnect( void )
{
    if ( xif_context_handle > 0 )
    {
        xif_cancel_timed_tasks();
        if ( XI_STATE_OK == xi_shutdown_connection( xif_context_handle ) )
        {
            return 0;
        }
    }
    return 1;
}

int8_t xif_is_connected( void )
{
    return ( XIF_MQTT_CONNECTED == xif_mqtt_connection_status );
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
 */
void xif_on_connected( xi_context_handle_t in_context_handle,
                       void* data,
                       xi_state_t state )
{
    xi_connection_data_t* conn_data = ( xi_connection_data_t* )data;

    switch ( conn_data->connection_state )
    {
        case XI_CONNECTION_STATE_OPEN_FAILED:
            printf( "\n[XIF] Connection to %s:%d has failed reason %d", conn_data->host,
                    conn_data->port, state );
            xif_mqtt_connection_status = XIF_MQTT_DISCONNECTED;
            if( XI_MQTT_BAD_USERNAME_OR_PASSWORD == state )
            {
                printf( "\n[XIF] Bad username or password. Review your credentials" );
                xif_handle_unrecoverable_error();
                return;
            }
            /* Re-attempt to connect until we succeed */
            xif_connect();
            return;

        case XI_CONNECTION_STATE_OPENED:
            xif_successful_connection_callback( conn_data );
            return;

        case XI_CONNECTION_STATE_CLOSED:
            printf( "\n[XIF] Connection closed - reason %d!", state );
            xif_mqtt_connection_status = XIF_MQTT_DISCONNECTED;
            if( XI_STATE_OK == state )
            {
                printf( "\n[XIF]\tDisconnection requested via xi_shutdown_connection()" );
                xif_request_machine_state( XIF_REQUEST_PAUSE );
            }
            return;

        default:
            printf( "\n[XIF] Invalid callback parameter %d", conn_data->connection_state );
            xif_handle_unrecoverable_error();
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
                printf( "\n[XIF] Subscription to LED topic [FAILED] with state [%d]", state );
                xif_handle_unrecoverable_error(); /* Should I just xi_subscribe again? */
                return;
            }
            printf( "\n[XIF] Subscription to LED topic [OK]" );
            return;

        case XI_SUB_CALL_MESSAGE:
            xif_recv_mqtt_msg_callback( params );
            return;

        default:
            return;
    }
}
