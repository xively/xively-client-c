/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
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

#include "xively_task.h"

#include "gpio_if.h"

/******************************************************************************
 *                          Xively Task Macros
 ******************************************************************************/
#define DELAY_MS( t ) vTaskDelay( t / portTICK_PERIOD_MS )

#define XT_BUTTON_TOPIC_NAME "Button"
#define XT_LED_TOPIC_NAME "LED"

#define XT_CONNECT_TIMEOUT_S   10
#define XT_KEEPALIVE_TIMEOUT_S 30

#define INTERRUPT_POLLING_PERIOD 1
/******************************************************************************
 *                     Xively Task Structs and Enums
 ******************************************************************************/
typedef struct
{
    char* xi_account_id;
    char* xi_device_id;
    char* xi_device_pwd;
} xt_device_info_t;

typedef enum {
    XT_MQTT_DISCONNECTED = 0,
    XT_MQTT_CONNECTED
} xt_mqtt_connection_states_t;

/******************************************************************************
 *                   Xively Task Function Declarations
 ******************************************************************************/
int8_t xt_connect( void );
int8_t xt_disconnect( void );
static int8_t xt_build_xively_topic( char* topic_name, char* dst, uint32_t dst_len );

/* RTOS specific functions */
static int8_t xt_clear_request_bits( xt_action_requests_t requested_action );
static int8_t xt_set_request_bits( xt_action_requests_t requested_action );
static inline void xt_await_unpause( void );
static xt_action_requests_t xt_pop_highest_priority_request( void );

/* Application specific functions */
static int8_t xt_configure_fw_updates( void );
static int8_t xt_subscribe( void );
static int8_t xt_start_timed_tasks( void );
static void xt_cancel_timed_tasks( void );
static void xt_pop_gpio_interrupt_queue( void );

/* Callbacks */
static void xt_on_connected( xi_context_handle_t in_context_handle,
                             void* data,
                             xi_state_t state );
static void xt_led_topic_callback( xi_context_handle_t in_context_handle,
                                   xi_sub_call_type_t call_type,
                                   const xi_sub_call_params_t* const params,
                                   xi_state_t state,
                                   void* user_data );

/******************************************************************************
 *                                Static Variables
 ******************************************************************************/
/* MQTT */
static xt_mqtt_connection_states_t xt_mqtt_connection_status = XT_MQTT_DISCONNECTED;
static xt_device_info_t xt_mqtt_device_info;
xt_mqtt_topics_t xt_mqtt_topics;

/* Xively Client Handles */
static xi_context_handle_t xt_context_handle = -1;
static xi_timed_task_handle_t xt_gpiohandler_scheduled_task_handle = -1;

/* FreeRTOS Handles */
static EventGroupHandle_t xt_requests_event_group_handle = NULL;

/* PUB_INCREASING_COUNTER can be set to create a scheduled task that will publish an
 * increasing counter to the /Button topic every $COUNTER_PUBLISH_PERIOD seconds */
#define PUB_INCREASING_COUNTER 0
#if PUB_INCREASING_COUNTER
#define COUNTER_PUBLISH_PERIOD 10
static void xt_publish_counter( void );
static xi_timed_task_handle_t xt_pubcounter_scheduled_task_handle = -1;
#endif

/******************************************************************************
 *                          Xively Task Implementation
 *****************************************************************************/
void xt_handle_unrecoverable_error( void )
{
    if ( !xt_ready_for_requests() )
    {
        return; /* Already shut down */
    }
    xt_request_machine_state( XT_REQUEST_SHUTDOWN );
}

/******************************************************************************
 *                               Events loop
 *****************************************************************************/
void xt_rtos_task( void* param )
{
    int evt_loop_exit_flag = 0;
    ( void )param;
    assert( NULL != xt_mqtt_device_info.xi_account_id );
    assert( NULL != xt_mqtt_device_info.xi_device_id );
    assert( NULL != xt_mqtt_device_info.xi_device_pwd );

    if ( XI_STATE_OK != xi_initialize( xt_mqtt_device_info.xi_account_id,
                                       xt_mqtt_device_info.xi_device_id ) )
    {
        printf( "\n[XT] Failed to initialize MQTT library" );
        xt_handle_unrecoverable_error();
    }
    else if ( 0 > ( xt_context_handle = xi_create_context() ) )
    {
        printf( "\n[XT] Failed to create xi context. Retval: %d", xt_context_handle );
        xt_handle_unrecoverable_error();
    }
    else if ( 0 > xt_configure_fw_updates() )
    {
        printf( "\n[XT] Failed to configure OTA firmware updates" );
    }
    else if ( 0 > xt_clear_request_bits( XT_REQUEST_ALL ) )
    {
        printf( "\n[XT] Failed to clear request bits during initialization" );
        xt_handle_unrecoverable_error();
    }
    else if ( 0 > xt_request_machine_state( XT_REQUEST_CONNECT ) )
    {
        xt_handle_unrecoverable_error();
    }

    /* Events loop */
    while ( 0 == evt_loop_exit_flag )
    {
        switch ( xt_pop_highest_priority_request() )
        {
            case XT_REQUEST_CONTINUE:
                if ( 0 > xt_request_machine_state( XT_REQUEST_CONTINUE ) )
                {
                    xt_handle_unrecoverable_error();
                }
                xi_events_process_tick();
                break;
            case XT_REQUEST_DISCONNECT:
                printf( "\n[XT] MQTT Disconnection requested" );
                /* FSM will be paused from the xt_on_connected callback */
                if ( 0 > xt_request_machine_state( XT_REQUEST_CONTINUE ) )
                {
                    xt_handle_unrecoverable_error();
                }
                xt_disconnect();
                break;
            case XT_REQUEST_CONNECT:
                printf( "\n[XT] MQTT Connection requested" );
                if ( 0 > xt_request_machine_state( XT_REQUEST_CONTINUE ) )
                {
                    xt_handle_unrecoverable_error();
                }
                if ( 0 > xt_connect() )
                {
                    xt_handle_unrecoverable_error();
                }
                break;
            case XT_REQUEST_PAUSE:
                /* Halt this task until we get a XT_REQUEST_CONTINUE request */
                printf( "\n[XT] Pausing Xively Task..." );
                xt_await_unpause();
                if ( 0 > xt_request_machine_state( XT_REQUEST_CONTINUE ) )
                {
                    xt_handle_unrecoverable_error();
                }
                break;
            case XT_REQUEST_SHUTDOWN:
                /* Free all the memory allocated by xt or the Xively Client, shut
                 * down the events loop, and delete the RTOS task */
                printf( "\n[XT] Shutting down Xively Task..." );
                if ( xt_context_handle >= 0 )
                {
                    xi_delete_context( xt_context_handle );
                }
                xi_shutdown();
                evt_loop_exit_flag = 1;
                break;
            case XT_REQUEST_ALL:
                /* This will never happen. xt_pop_highest_priority won't return it.
                 * If all bits are set, they will be processed in order (this state
                 * machine will shut down)  */
                assert( 0 );
        }
        taskYIELD();
    }

    xt_context_handle = -1;
    vEventGroupDelete( xt_requests_event_group_handle );
    xt_requests_event_group_handle = NULL;
    memset( &xt_mqtt_topics, 0x00, sizeof( xt_mqtt_topics_t ) );
    memset( &xt_mqtt_device_info, 0x00, sizeof( xt_device_info_t ) );
    printf( "\n[XT] Xively Task has been shut down" );
    vTaskDelete( NULL );
    return NULL;
}

int8_t xt_build_xively_topic( char* topic_name, char* dst, uint32_t dst_len )
{
    int retval = 0;

    printf("\n[xt_build_xively_topic][topic_name: %s]", topic_name);
    assert( NULL != topic_name );
    assert( NULL != dst );

    retval = snprintf( dst, dst_len, "xi/blue/v1/%.64s/d/%.64s/%.64s",
                       xt_mqtt_device_info.xi_account_id,
                       xt_mqtt_device_info.xi_device_id, topic_name );
    printf("\n[xt_build_xively_topic][topic: %s]", dst);

    if ( ( retval >= dst_len ) || ( retval < 0 ) )
    {
        return -1;
    }
    return 0;
}

int8_t xt_init( char* xi_acc_id, char* xi_dev_id, char* xi_dev_pwd )
{
    printf( "\n[XT] Initializing Xively Task variables" );

    printf("\nsetting device variables");
    xt_mqtt_device_info.xi_account_id = xi_acc_id;
    xt_mqtt_device_info.xi_device_id  = xi_dev_id;
    xt_mqtt_device_info.xi_device_pwd = xi_dev_pwd;

    xt_requests_event_group_handle = xEventGroupCreate();
    if ( NULL == xt_requests_event_group_handle )
    {
        printf( "\n[XT] Failed to allocate FreeRTOS heap for the requests event group" );
        return -1;
    }

    if ( 0 > xt_build_xively_topic( XT_BUTTON_TOPIC_NAME, xt_mqtt_topics.button_topic,
                                    XT_MQTT_TOPIC_MAX_LEN ) )
    {
        printf( "\n[XT] Unrecoverable building MQTT topic strings. Abort" );
        return -1;
    }
    if ( 0 > xt_build_xively_topic( XT_LED_TOPIC_NAME, xt_mqtt_topics.led_topic,
                                    XT_MQTT_TOPIC_MAX_LEN ) )
    {
        printf( "\n[XT] Unrecoverable building MQTT topic strings. Abort" );
        return -1;
    }
    return 0;
}

/* @retval: -1: xi_connect failed
 *           0: 0 xi_connect OK
 *           1: xi_connect failed because the connection was already initialized
 */
int8_t xt_connect( void )
{
    xi_state_t ret_state = XI_STATE_OK;
    printf( "\n[XT] Connecting to MQTT broker:" );
    printf( "\n[XT] \tAccount ID: %s", xt_mqtt_device_info.xi_account_id );
    printf( "\n[XT] \tDevice  ID: %s", xt_mqtt_device_info.xi_device_id );
    printf( "\n[XT] \tAccount Password: (REDACTED)" );
    ret_state = xi_connect( xt_context_handle, xt_mqtt_device_info.xi_device_id,
                            xt_mqtt_device_info.xi_device_pwd, XT_CONNECT_TIMEOUT_S,
                            XT_KEEPALIVE_TIMEOUT_S, XI_SESSION_CLEAN, &xt_on_connected );

    if ( XI_ALREADY_INITIALIZED == ret_state )
    {
        return 1;
    }
    else if ( XI_STATE_OK != ret_state )
    {
        printf( "\n[XT] Error in xi_connect! Retcode: %d", ret_state );
        return -1;
    }
    return 0;
}

static int8_t xt_configure_fw_updates( void )
{
    const char** updateable_files = ( const char* [] ){"firmware.bin"};
    const uint16_t files_num      = 1;

    xi_state_t ret = XI_STATE_OK;

    printf( "\n[XT] Registering updateable files list:" );
    for ( uint16_t i = 0; i < files_num; i++ )
    {
        printf( "\n[XT] \t* %s", updateable_files[i] );
    }

    ret = xi_set_updateable_files( xt_context_handle, updateable_files, files_num, NULL );
    if ( XI_STATE_OK != ret )
    {
        printf( "\n[XT] Error [%d] configuring FW updates", ret );
        return -1;
    }
    return 0;
}

int8_t xt_subscribe( void )
{
    xi_state_t ret_state = XI_STATE_OK;
    printf( "\n[XT] Subscribing to MQTT topics" );
    ret_state = xi_subscribe( xt_context_handle, xt_mqtt_topics.led_topic,
                              XI_MQTT_QOS_AT_MOST_ONCE, &xt_led_topic_callback, NULL );
    if ( XI_STATE_OK != ret_state )
    {
        return -1;
    }
    return 0;
}

/* This function will pop 1 item from the gpio_if.h's interrupt handler queue,
 * and publish it */
void xt_pop_gpio_interrupt_queue( void )
{
    static int virtual_switch = 0; /* Switches between 1 and 0 on each button press */
    char msg_payload[2] = "";

    if ( !xt_is_connected() )
    {
        return;
    }

    if ( io_pop_gpio_interrupt() == 0 )
    {
        virtual_switch = !virtual_switch;
        ( virtual_switch == 1 ) ? ( strcpy( msg_payload, "1" ) )
                                : ( strcpy( msg_payload, "0" ) );
        printf( "\n[XT] Button pressed! Virtual switch [%d]", virtual_switch );
        printf( "\n[XT]\tPublishing MQTT message" );
        xi_publish( xt_context_handle, xt_mqtt_topics.button_topic, msg_payload,
                    XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
    }
}

#if PUB_INCREASING_COUNTER
void xt_publish_counter( void )
{
    static unsigned long msg_counter = 0;
    char msg_counter_s[32];
    xi_state_t ret_state = XI_STATE_OK;

    if ( !xt_is_connected() )
    {
        return;
    }

    msg_counter++;
    sprintf( msg_counter_s, "%ld", msg_counter );
    printf( "\n[XT] Publishing MQTT message [%s]", msg_counter_s );

    ret_state = xi_publish( xt_context_handle, xt_mqtt_topics.button_topic, msg_counter_s,
                            XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
    if ( ( XI_OUT_OF_MEMORY == ret_state ) || ( XI_INTERNAL_ERROR == ret_state ) )
    {
        xt_handle_unrecoverable_error();
    }
}
#endif /* PUB_INCREASING_COUNTER */

int8_t xt_start_timed_tasks( void )
{
    /* Register a timed task to poll the gpio_if's interrupt queue every
     * $INTERRUPT_POLLING_PERIOD seconds */
    xt_gpiohandler_scheduled_task_handle = xi_schedule_timed_task(
        xt_context_handle, ( xi_user_task_callback_t* )xt_pop_gpio_interrupt_queue,
        INTERRUPT_POLLING_PERIOD, 1, NULL );
    if ( XI_INVALID_TIMED_TASK_HANDLE == xt_gpiohandler_scheduled_task_handle )
    {
        printf( "\n[XT] Error: GPIO interrupt polling scheduled task couldn't be registered" );
        return -1;
    }
    printf( "\n[XT] GPIO interrupt polling scheduled task started" );

#if PUB_INCREASING_COUNTER
    /* Schedule the xt_publish_counter function to publish an increasing counter
     * every $COUNTER_PUBLISH_PERIOD seconds */
    xt_pubcounter_scheduled_task_handle = xi_schedule_timed_task(
        xt_context_handle, ( xi_user_task_callback_t* )xt_publish_counter,
        COUNTER_PUBLISH_PERIOD, 1, NULL );
    if ( XI_INVALID_TIMED_TASK_HANDLE == xt_pubcounter_scheduled_task_handle )
    {
        printf( "\n[XT] Counter publish scheduled task couldn't be registered" );
        return -1;
    }
#endif /* PUB_INCREASING_COUNTER */

    return 0;
}

void xt_cancel_timed_tasks( void )
{
    xi_cancel_timed_task( xt_gpiohandler_scheduled_task_handle );
    xt_gpiohandler_scheduled_task_handle = XI_INVALID_TIMED_TASK_HANDLE;
#if PUB_INCREASING_COUNTER
    xi_cancel_timed_task( xt_pubcounter_scheduled_task_handle );
    xt_pubcounter_scheduled_task_handle = XI_INVALID_TIMED_TASK_HANDLE;
#endif
}

/*
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
xt_recv_mqtt_msg_callback( const xi_sub_call_params_t* const params )
{
    printf( "\n[XT] New MQTT message received!" );
    printf( "\n[XT]\tTopic: %s", params->message.topic );
    printf( "\n[XT]\tMessage size: %d", params->message.temporary_payload_data_length );
    printf( "\n[XT]\tMessage payload: " );
    for ( size_t i = 0; i < params->message.temporary_payload_data_length; i++ )
    {
        printf( "%c", ( char )params->message.temporary_payload_data[i] );
    }
}

/*
 * Cancel timed tasks and call xi_shutdown_connection() to disconnect from the
 * broker
 *
 * @retval 0: Disconnect message sent - Task will be paused from on_connected callback
 * @retval 1: MQTT was not connected - Task shutdown initiated
 */
int8_t xt_disconnect( void )
{
    if ( xt_context_handle > 0 )
    {
        xt_cancel_timed_tasks();
        if ( XI_STATE_OK == xi_shutdown_connection( xt_context_handle ) )
        {
            return 0;
        }
    }
    return 1;
}

int8_t xt_is_connected( void )
{
    return ( XT_MQTT_CONNECTED == xt_mqtt_connection_status );
}

/******************************************************************************
 *                           xively-client-c callbacks
 *****************************************************************************/
/**
 * @brief Called by the Xively Client whenever there is a change in the MQTT
 * connection status. See xively.h for full signature information
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
void xt_on_connected( xi_context_handle_t in_context_handle,
                      void* data,
                      xi_state_t state )
{
    xi_connection_data_t* conn_data = ( xi_connection_data_t* )data;

    switch ( conn_data->connection_state )
    {
        case XI_CONNECTION_STATE_OPENED:
            xt_mqtt_connection_status = XT_MQTT_CONNECTED;
            printf( "\n[XT] MQTT connected to %s:%d", conn_data->host, conn_data->port );
            if ( 0 > xt_subscribe() )
            {
                xt_handle_unrecoverable_error();
                return;
            }

            xt_start_timed_tasks();
            return;

        case XI_CONNECTION_STATE_OPEN_FAILED:
            printf( "\n[XT] Connection to %s:%d has failed reason %d", conn_data->host,
                    conn_data->port, state );
            xt_mqtt_connection_status = XT_MQTT_DISCONNECTED;
            if ( XI_MQTT_BAD_USERNAME_OR_PASSWORD == state )
            {
                printf( "\n[XT] Bad username or password. Review your credentials" );
                xt_handle_unrecoverable_error();
            }
            else
            {
                /* Re-attempt to connect until we succeed */
                printf( "\n[XT]\tAttempting to reconnect..." );
                if ( 0 > xt_request_machine_state( XT_REQUEST_CONNECT ) )
                {
                    xt_handle_unrecoverable_error();
                }
            }
            return;

        case XI_CONNECTION_STATE_CLOSED:
            printf( "\n[XT] Connection closed - reason %d!", state );
            xt_mqtt_connection_status = XT_MQTT_DISCONNECTED;
            if ( XI_STATE_OK == state )
            {
                printf( "\n[XT]\tRequested via xi_shutdown_connection(). Pausing task" );
                if ( 0 > xt_request_machine_state( XT_REQUEST_PAUSE ) )
                {
                    xt_handle_unrecoverable_error();
                }
            }
            else
            {
                printf( "\n[XT]\tAttempting to reconnect..." );
                if ( 0 > xt_request_machine_state( XT_REQUEST_CONNECT ) )
                {
                    xt_handle_unrecoverable_error();
                }
            }
            return;

        default:
            printf( "\n[XT] Invalid callback parameter %d", conn_data->connection_state );
            xt_handle_unrecoverable_error();
            return;
    }
}

/**
 * @brief Called by the Xively Client whenever there is an update on the LED
 * topic. Handle Subscription ACKs and pass incoming messages to the logic layer
 * with the xt_recv_mqtt_msg_callback function
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
void xt_led_topic_callback( xi_context_handle_t in_context_handle,
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
                printf( "\n[XT] Subscription to LED topic [FAILED] with state [%d]",
                        state );
                xt_handle_unrecoverable_error(); /* Should I just xi_subscribe again? */
                return;
            }
            printf( "\n[XT] Subscription to LED topic [OK]" );
            return;

        case XI_SUB_CALL_MESSAGE:
            xt_recv_mqtt_msg_callback( params );
            return;

        default:
            return;
    }
}

/******************************************************************************
 *                         FreeRTOS-specific implementation
 *****************************************************************************/
int8_t xt_set_request_bits( xt_action_requests_t requested_action )
{
    if ( !xt_ready_for_requests() )
    {
        return -1;
    }
    EventBits_t bits_after_set = 0x00;

    xEventGroupSetBits( xt_requests_event_group_handle, requested_action );
    bits_after_set = xEventGroupGetBits( xt_requests_event_group_handle );

    if ( ( bits_after_set & requested_action ) != requested_action )
    {
        printf( "\n[XT] Error: Failed to set request bits [0x%02x]", requested_action );
        return -1;
    }
    return 0;
}

int8_t xt_clear_request_bits( xt_action_requests_t requested_action )
{
    if ( !xt_ready_for_requests() )
    {
        return -1;
    }
    EventBits_t bits_after_clear = 0x00;

    xEventGroupClearBits( xt_requests_event_group_handle, requested_action );
    bits_after_clear = xEventGroupGetBits( xt_requests_event_group_handle );

    if ( ( bits_after_clear & requested_action ) != 0x00 )
    {
        printf( "\n[XT] Error: Failed to clear request bits [0x%02x]", requested_action );
        return -1;
    }
    return 0;
}

int8_t xt_request_machine_state( xt_action_requests_t requested_action )
{
    if ( !xt_ready_for_requests() )
    {
        printf( "\n[XT] Error: Xively Task not ready for requests. Ignored" );
        return -1;
    }

    switch ( requested_action )
    {
        case XT_REQUEST_CONNECT:
        case XT_REQUEST_CONTINUE:
        case XT_REQUEST_DISCONNECT:
        case XT_REQUEST_SHUTDOWN:
            if ( 0 > xt_set_request_bits( requested_action ) )
            {
                return -1;
            }
            break;
        case XT_REQUEST_PAUSE:
            if ( 0 > xt_clear_request_bits( XT_REQUEST_CONTINUE ) )
            {
                return -1;
            }
            if ( 0 > xt_set_request_bits( requested_action ) )
            {
                return -1;
            }
            break;
        case XT_REQUEST_ALL:
            return -1;
    }
    return 0;
}

/**
 * Reads the thread-safe requests bitmask, gets the next highest priority request
 * (the highest bit in the bitmask), clears that bit and returns its corresponding
 * xt_action_requests_t enum value.
 *
 * The XT_REQUEST_CONTINUE flag is ignored, and it will be returned whenever
 * the rest of the flags are 0
 */
xt_action_requests_t xt_pop_highest_priority_request( void )
{
    EventBits_t external_request_bitmask = 0x00;
    external_request_bitmask = xEventGroupGetBits( xt_requests_event_group_handle );

    if ( XT_REQUEST_SHUTDOWN & external_request_bitmask )
    {
        xt_clear_request_bits( XT_REQUEST_SHUTDOWN );
        return XT_REQUEST_SHUTDOWN;
    }
    else if ( XT_REQUEST_PAUSE & external_request_bitmask )
    {
        xt_clear_request_bits( XT_REQUEST_PAUSE );
        return XT_REQUEST_PAUSE;
    }
    else if ( XT_REQUEST_DISCONNECT & external_request_bitmask )
    {
        xt_clear_request_bits( XT_REQUEST_DISCONNECT );
        return XT_REQUEST_DISCONNECT;
    }
    else if ( XT_REQUEST_CONNECT & external_request_bitmask )
    {
        xt_clear_request_bits( XT_REQUEST_CONNECT );
        return XT_REQUEST_CONNECT;
    }

    return XT_REQUEST_CONTINUE;
}

inline void xt_await_unpause( void )
{
    const TickType_t wait_timeout = portMAX_DELAY;
    EventBits_t evt_group_bits = 0x00;
    do /* Wait (forever) until either _CONTINUE or _SHUTDOWN are requested */
    {
        evt_group_bits = xEventGroupWaitBits( xt_requests_event_group_handle,
                                              XT_REQUEST_CONTINUE | XT_REQUEST_SHUTDOWN,
                                              false, true, wait_timeout );
    } while ( 0x00 == evt_group_bits );
}

int8_t xt_ready_for_requests( void )
{
    if ( NULL == xt_requests_event_group_handle )
    {
        return 0;
    }
    return 1;
}
