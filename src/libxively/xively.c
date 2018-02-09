/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xi_allocator.h"
#include "xi_backoff_status_api.h"
#include "xi_common.h"
#include "xi_connection_data.h"
#include "xi_debug.h"
#include "xi_err.h"
#include "xi_event_loop.h"
#include "xi_globals.h"
#include "xi_handle.h"
#include "xi_helpers.h"
#include "xi_internals.h"
#include "xi_layer_api.h"
#include "xi_layer_chain.h"
#include "xi_layer_default_allocators.h"
#include "xi_layer_factory.h"
#include "xi_layer_interface.h"
#include "xi_macros.h"
#include "xi_timed_task.h"
#include "xi_version.h"
#include "xi_list.h"
#include "xively.h"

#include "xi_mqtt_host_accessor.h"

#include "xi_user_sub_call_wrapper.h"

#include <xi_bsp_time.h>
#include <xi_bsp_rng.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * CONSTANTS
 */

/* maximum length of a csv element as dictated by the TimeSeries team */
const size_t xi_max_csv_element_chars = 1024;

const uint16_t xi_major    = XI_MAJOR;
const uint16_t xi_minor    = XI_MINOR;
const uint16_t xi_revision = XI_REVISION;

#define STR_HELPER( x ) #x
#define STR( x ) STR_HELPER( x )

const char xi_cilent_version_str[] = "Xively C Client Version: " STR( XI_MAJOR ) "." STR(
    XI_MINOR ) "." STR( XI_REVISION );


/*
 * INTERNAL FUNCTIONS
 */

/*
 * HELPER FUNCTIONS
 */
void xi_set_network_timeout( uint32_t timeout )
{
    xi_globals.network_timeout = timeout;
}

uint32_t xi_get_network_timeout( void )
{
    return xi_globals.network_timeout;
}

/* indentifies characters that would break the
 * csv format, ie {,\n\r}.  Also checks the length of the string
 * to ensure that it's within the acceptible bounds of the Timeseries
 * parser. */
xi_state_t check_csv_entry( const char* in_string, int* out_num_chars )
{
    if ( NULL == in_string || NULL == out_num_chars )
    {
        return XI_INTERNAL_ERROR;
    }

    static char illegal_chars[] = {',', '\r', '\n'};
    const int num_illegal_chars = sizeof( illegal_chars ) / sizeof( char );

    size_t index = 0;
    /* for each character, make sure it's not an illegal character */
    for ( ; index < xi_max_csv_element_chars; index++ )
    {
        if ( in_string[index] == '\0' )
        {
            break;
        }

        int illegal_char_index = 0;
        for ( ; illegal_char_index < num_illegal_chars; illegal_char_index++ )
        {
            if ( in_string[index] == illegal_chars[illegal_char_index] )
            {
                return XI_SERIALIZATION_ERROR;
            }
        }
    }

    /* safeguard against non terminated strings,
     * and/or strings too long for the service */
    if ( xi_max_csv_element_chars == index )
    {
        return XI_SERIALIZATION_ERROR;
    }

    *out_num_chars = index;

    /* string checks out. Let's do this, kid! */
    return XI_STATE_OK;
}

/*
 * MAIN LIBRARY FUNCTIONS
 */
xi_state_t xi_initialize( const char* account_id, const char* device_unique_id )
{
    xi_bsp_time_init();
    xi_bsp_rng_init();

    if ( NULL != xi_globals.str_account_id || NULL != xi_globals.str_device_unique_id )
    {
        return XI_ALREADY_INITIALIZED;
    }
    else if ( NULL == account_id || NULL == device_unique_id )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_globals.str_account_id       = xi_str_dup( account_id );
    xi_globals.str_device_unique_id = xi_str_dup( device_unique_id );

    if ( NULL == xi_globals.str_device_unique_id || NULL == xi_globals.str_account_id )
    {
        return XI_FAILED_INITIALIZATION;
    }

    return XI_STATE_OK;
}

xi_state_t xi_shutdown()
{
    XI_SAFE_FREE( xi_globals.str_account_id );
    XI_SAFE_FREE( xi_globals.str_device_unique_id );

    xi_bsp_rng_shutdown();

    return XI_STATE_OK;
}

xi_state_t xi_set_device_unique_id( const char* unique_id )
{
    if ( NULL == unique_id )
    {
        return XI_INVALID_PARAMETER;
    }
    else if ( NULL != xi_globals.str_device_unique_id )
    {
        return XI_ALREADY_INITIALIZED;
    }

    int len                         = strlen( unique_id );
    xi_globals.str_device_unique_id = xi_alloc( len );

    if ( NULL == xi_globals.str_device_unique_id )
    {
        return XI_OUT_OF_MEMORY;
    }

    strcpy( xi_globals.str_device_unique_id, unique_id );

    return XI_STATE_OK;
}

const char* xi_get_device_id()
{
    return xi_globals.str_device_unique_id;
}

void xi_default_client_callback( xi_context_handle_t in_context_handle,
                                 void* data,
                                 xi_state_t state )
{
    XI_UNUSED( in_context_handle );
    XI_UNUSED( data );
    XI_UNUSED( state );
}

xi_state_t xi_user_callback_wrapper( void* context,
                                     void* data,
                                     xi_state_t in_state,
                                     void* client_callback )
{
    assert( NULL != client_callback );
    assert( NULL != context );

    xi_state_t state = XI_STATE_OK;
    xi_context_handle_t context_handle;
    XI_CHECK_STATE( state = xi_find_handle_for_object( xi_globals.context_handles_vector,
                                                       context, &context_handle ) );

    ( ( xi_user_callback_t* )( client_callback ) )( context_handle, data, in_state );

err_handling:
    return state;
}

extern uint8_t xi_is_context_connected( xi_context_handle_t xih )
{
    if ( XI_INVALID_CONTEXT_HANDLE == xih )
    {
        return 0;
    }

    xi_context_t* xi =
        ( xi_context_t* )xi_object_for_handle( xi_globals.context_handles_vector, xih );


    if ( NULL == xi || NULL == xi->context_data.connection_data )
    {
        return 0;
    }

    return XI_CONNECTION_STATE_OPENED ==
               xi->context_data.connection_data->connection_state &&
           XI_SHUTDOWN_UNINITIALISED == xi->context_data.shutdown_state;
}

void xi_events_stop()
{
    xi_evtd_stop( xi_globals.evtd_instance );
}

void xi_events_process_blocking()
{
    xi_event_loop_with_evtds( 0, &xi_globals.evtd_instance, 1 );
}

xi_state_t xi_events_process_tick()
{
    if ( xi_evtd_dispatcher_continue( xi_globals.evtd_instance ) == 1 )
    {
        xi_event_loop_with_evtds( 1, &xi_globals.evtd_instance, 1 );
        return XI_STATE_OK;
    }

    return XI_EVENT_PROCESS_STOPPED;
}


xi_state_t xi_set_updateable_files( xi_context_handle_t xih,
                                    const char** filenames,
                                    uint16_t count,
                                    xi_sft_url_handler_callback_t* url_handler )
{
    if ( NULL == filenames || NULL == *filenames || 0 == count )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t state = XI_STATE_OK;
    xi_context_t* xi = xi_object_for_handle( xi_globals.context_handles_vector, xih );
    uint16_t id_file = 0;

    XI_CHECK_CND_DBGMESSAGE( NULL == xi, XI_NULL_CONTEXT, state,
                             "ERROR: NULL context provided" );

    XI_ALLOC_BUFFER_AT( char*, xi->context_data.updateable_files, sizeof( char* ) * count,
                        state );

    xi->context_data.updateable_files_count = count;

    for ( ; id_file < xi->context_data.updateable_files_count; ++id_file )
    {
        xi->context_data.updateable_files[id_file] = xi_str_dup( filenames[id_file] );
    }

    xi->context_data.sft_url_handler_callback = url_handler;

err_handling:
    return state;
}


xi_state_t xi_connect_with_lastwill_to_impl( xi_context_t* xi,
                                             const char* host,
                                             uint16_t port,
                                             const char* username,
                                             const char* password,
                                             uint16_t connection_timeout,
                                             uint16_t keepalive_timeout,
                                             xi_session_type_t session_type,
                                             const char* will_topic,
                                             const char* will_message,
                                             xi_mqtt_qos_t will_qos,
                                             xi_mqtt_retain_t will_retain,
                                             xi_user_callback_t* client_callback )
{
    xi_state_t state               = XI_STATE_OK;
    xi_event_handle_t event_handle = xi_make_empty_event_handle();
    xi_layer_t* input_layer        = NULL;
    uint32_t new_backoff           = 0;

    XI_CHECK_CND_DBGMESSAGE( NULL == host, XI_NULL_HOST, state,
                             "ERROR: NULL host provided" );

    XI_CHECK_CND_DBGMESSAGE( NULL == xi, XI_NULL_CONTEXT, state,
                             "ERROR: NULL context provided" );

    assert( NULL != client_callback );

    event_handle =
        xi_make_threaded_handle( XI_THREADID_THREAD_0, &xi_user_callback_wrapper, xi,
                                 NULL, XI_STATE_OK, ( void* )client_callback );

    /* guard against adding two connection requests */
    if ( NULL != xi->context_data.connect_handler.ptr_to_position )
    {
        xi_debug_format( "Connect could not be performed due to conenction state = %d,"
                         "check if connect operation hasn't been already started.",
                         xi->context_data.connection_data->connection_state );
        return XI_ALREADY_INITIALIZED;
    }

    /* if the connection state isn't one of
     * the final states it means that the connection already has been started */
    if ( NULL != xi->context_data.connection_data &&
         ( XI_CONNECTION_STATE_CLOSED !=
               xi->context_data.connection_data->connection_state &&
           XI_CONNECTION_STATE_OPEN_FAILED !=
               xi->context_data.connection_data->connection_state &&
           XI_CONNECTION_STATE_UNINITIALIZED !=
               xi->context_data.connection_data->connection_state ) )
    {
        xi_debug_format( "Connect could not be performed due to conenction state = %d,"
                         "check if connect operation hasn't been already started.",
                         xi->context_data.connection_data->connection_state );
        return XI_ALREADY_INITIALIZED;
    }

    xi_debug_format( "connecting with username = \"%s\"", username );

    input_layer  = xi->layer_chain.top;
    xi->protocol = XI_MQTT;

    if ( NULL != xi->context_data.connection_data )
    {
        XI_CHECK_STATE( xi_connection_data_update_lastwill(
            xi->context_data.connection_data, host, port, username, password,
            connection_timeout, keepalive_timeout, session_type, will_topic, will_message,
            will_qos, will_retain ) );
    }
    else
    {
        xi->context_data.connection_data = xi_alloc_connection_data_lastwill(
            host, port, username, password, connection_timeout, keepalive_timeout,
            session_type, will_topic, will_message, will_qos, will_retain );

        XI_CHECK_MEMORY( xi->context_data.connection_data, state );
    }

    xi_debug_format( "New host:port [%s]:[%hu]", xi->context_data.connection_data->host,
                     xi->context_data.connection_data->port );

    /* reset the connection state */
    xi->context_data.connection_data->connection_state =
        XI_CONNECTION_STATE_UNINITIALIZED;

    /* reset shutdown state */
    xi->context_data.shutdown_state = XI_SHUTDOWN_UNINITIALISED;

    /* set the connection callback */
    xi->context_data.connection_callback = event_handle;

    new_backoff = xi_get_backoff_penalty();

    xi_debug_format( "new backoff value: %d", new_backoff );

    /* register the execution in next init */
    state = xi_evtd_execute_in(
        xi->context_data.evtd_instance,
        xi_make_handle( input_layer->layer_connection.self->layer_funcs->init,
                        &input_layer->layer_connection, xi->context_data.connection_data,
                        XI_STATE_OK ),
        new_backoff, &xi->context_data.connect_handler );

    XI_CHECK_STATE( state );

    return XI_STATE_OK;

err_handling:
    return state;
}

xi_state_t xi_connect( xi_context_handle_t xih,
                       const char* username,
                       const char* password,
                       uint16_t connection_timeout,
                       uint16_t keepalive_timeout,
                       xi_session_type_t session_type,
                       xi_user_callback_t* client_callback )
{
    xi_context_t* xi = xi_object_for_handle( xi_globals.context_handles_vector, xih );

    return xi_connect_with_lastwill_to_impl(
        xi, XI_MQTT_HOST_ACCESSOR.name, XI_MQTT_HOST_ACCESSOR.port, username, password,
        connection_timeout, keepalive_timeout, session_type, NULL, /* will_topic */
        NULL,                                                      /* will_message */
        ( xi_mqtt_qos_t )0,                                        /* will_qos */
        ( xi_mqtt_retain_t )0,                                     /* will_retain */
        client_callback );
}

xi_state_t xi_connect_to( xi_context_handle_t xih,
                          const char* host,
                          uint16_t port,
                          const char* username,
                          const char* password,
                          uint16_t connection_timeout,
                          uint16_t keepalive_timeout,
                          xi_session_type_t session_type,
                          xi_user_callback_t* client_callback )
{
    xi_context_t* xi = xi_object_for_handle( xi_globals.context_handles_vector, xih );

    return xi_connect_with_lastwill_to_impl( xi, host, port, username, password,
                                             connection_timeout, keepalive_timeout,
                                             session_type, NULL,    /* will_topic */
                                             NULL,                  /* will_message */
                                             ( xi_mqtt_qos_t )0,    /* will_qos */
                                             ( xi_mqtt_retain_t )0, /* will_retain */
                                             client_callback );
}

xi_state_t xi_connect_with_lastwill( xi_context_handle_t xih,
                                     const char* username,
                                     const char* password,
                                     uint16_t connection_timeout,
                                     uint16_t keepalive_timeout,
                                     xi_session_type_t session_type,
                                     const char* will_topic,
                                     const char* will_message,
                                     xi_mqtt_qos_t will_qos,
                                     xi_mqtt_retain_t will_retain,
                                     xi_user_callback_t* client_callback )
{
    /* will_topic is required. */
    if ( NULL == will_topic )
    {
        return XI_NULL_WILL_TOPIC;
    }

    /* will_message is required. */
    if ( NULL == will_message )
    {
        return XI_NULL_WILL_MESSAGE;
    }

    xi_context_t* xi = xi_object_for_handle( xi_globals.context_handles_vector, xih );

    return xi_connect_with_lastwill_to_impl(
        xi, XI_MQTT_HOST_ACCESSOR.name, XI_MQTT_HOST_ACCESSOR.port, username, password,
        connection_timeout, keepalive_timeout, session_type, will_topic, will_message,
        will_qos, will_retain, client_callback );
}

xi_state_t xi_connect_with_lastwill_to( xi_context_handle_t xih,
                                        const char* host,
                                        uint16_t port,
                                        const char* username,
                                        const char* password,
                                        uint16_t connection_timeout,
                                        uint16_t keepalive_timeout,
                                        xi_session_type_t session_type,
                                        const char* will_topic,
                                        const char* will_message,
                                        xi_mqtt_qos_t will_qos,
                                        xi_mqtt_retain_t will_retain,
                                        xi_user_callback_t* client_callback )
{
    /* will_topic is required. */
    if ( NULL == will_topic )
    {
        return XI_NULL_WILL_TOPIC;
    }

    /* will_message is required. */
    if ( NULL == will_message )
    {
        return XI_NULL_WILL_MESSAGE;
    }

    xi_context_t* xi = xi_object_for_handle( xi_globals.context_handles_vector, xih );

    return xi_connect_with_lastwill_to_impl(
        xi, host, port, username, password, connection_timeout, keepalive_timeout,
        session_type, will_topic, will_message, will_qos, will_retain, client_callback );
}

xi_state_t xi_publish_data_impl( xi_context_handle_t xih,
                                 const char* topic,
                                 xi_data_desc_t* data,
                                 const xi_mqtt_qos_t qos,
                                 const xi_mqtt_retain_t retain,
                                 xi_user_callback_t* callback,
                                 void* user_data,
                                 xi_thread_id_t callback_target_thread_id )
{
    /* PRE-CONDITIONS */
    assert( XI_INVALID_CONTEXT_HANDLE < xih );
    xi_context_t* xi =
        ( xi_context_t* )xi_object_for_handle( xi_globals.context_handles_vector, xih );
    assert( NULL != xi );

    if ( NULL == callback )
    {
        callback  = &xi_default_client_callback;
        user_data = NULL;
    }

    xi_event_handle_t event_handle =
        xi_make_threaded_handle( callback_target_thread_id, &xi_user_callback_wrapper, xi,
                                 user_data, XI_STATE_OK, ( void* )callback );

    assert( XI_EVENT_HANDLE_ARGC4 == event_handle.handle_type ||
            XI_EVENT_HANDLE_UNSET == event_handle.handle_type );

    if ( XI_BACKOFF_CLASS_NONE != xi_globals.backoff_status.backoff_class )
    {
        xi_free_desc( &data );
        return XI_BACKOFF_TERMINAL;
    }

    xi_mqtt_qos_t effective_qos = qos;

    xi_mqtt_logic_task_t* task = NULL;
    xi_state_t state           = XI_STATE_OK;
    xi_layer_t* input_layer    = xi->layer_chain.top;

    xi_mqtt_logic_layer_data_t* layer_data =
        ( xi_mqtt_logic_layer_data_t* )input_layer->user_data;

    XI_UNUSED( layer_data );

    task = xi_mqtt_logic_make_publish_task( topic, data, effective_qos, retain,
                                            event_handle );

    XI_CHECK_MEMORY( task, state );

    return XI_PROCESS_PUSH_ON_THIS_LAYER( &input_layer->layer_connection, task,
                                          XI_STATE_OK );

err_handling:
    if ( task )
    {
        xi_mqtt_logic_free_task( &task );
    }

    return state;
}

xi_state_t xi_publish( xi_context_handle_t xih,
                       const char* topic,
                       const char* msg,
                       const xi_mqtt_qos_t qos,
                       const xi_mqtt_retain_t retain,
                       xi_user_callback_t* callback,
                       void* user_data )
{
    /* PRE-CONDITIONS */
    assert( NULL != topic );
    assert( NULL != msg );

    xi_state_t state = XI_STATE_OK;

    xi_data_desc_t* data_desc = xi_make_desc_from_string_copy( msg );

    XI_CHECK_MEMORY( data_desc, state );

    return xi_publish_data_impl( xih, topic, data_desc, qos, retain, callback, user_data,
                                 XI_THREADID_THREAD_0 );

err_handling:
    return state;
}

xi_state_t xi_publish_timeseries( xi_context_handle_t xih,
                                  const char* topic,
                                  const float value,
                                  const xi_mqtt_qos_t qos,
                                  xi_user_callback_t* callback,
                                  void* user_data )
{
    assert( NULL != topic );

    xi_data_desc_t* data_desc = xi_make_desc_from_float_copy( value );

    xi_state_t state = XI_STATE_OK;

    XI_CHECK_MEMORY( data_desc, state );

    return xi_publish_data_impl( xih, topic, data_desc, qos, XI_MQTT_RETAIN_FALSE,
                                 callback, user_data, XI_THREADID_THREAD_0 );

err_handling:
    return state;
}

xi_state_t xi_publish_formatted_timeseries( xi_context_handle_t xih,
                                            const char* topic,
                                            const uint32_t* time,
                                            char* in_category,
                                            char* in_string_value,
                                            const float* in_numeric_value,
                                            const xi_mqtt_qos_t qos,
                                            xi_user_callback_t* callback,
                                            void* user_data )
{
    if ( NULL == topic )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t state = XI_STATE_OK;

    char compute_size_buf[1] = {'\0'};

    /* sizes */
    int num_chars_time          = 0;
    int num_chars_category      = 0;
    int num_chars_numeric_value = 0;
    int num_chars_string_value  = 0;

    static const char* empty_string = "";

    /* used to format the time into a string */
    char* time_string        = NULL;
    char* numeric_string     = NULL;
    const char* string_value = NULL;
    const char* category     = NULL;

    const int num_control_chars = 5; /* 3x Commas
                                      , 1x New Line
                                      , 1x Null Terminator */
    static const char* format_template = "%s,%s,%s,%s\n";
    int final_size                     = 0;
    int total_size                     = 0;

    /* points to the string that we will eventually format using snprintf */
    char* final_buffer = NULL;

    /* points to the memory that we'll provide to the Client
     * for publication */
    xi_data_desc_t* data_desc = NULL;

    /* timestamp is optional */
    if ( NULL == time )
    {
        XI_ALLOC_BUFFER_AT( char, time_string, 1, state );
        time_string[0] = '\0';
        num_chars_time = 0;
    }
    else
    {
        num_chars_time = snprintf( compute_size_buf, 1, "%" PRIu32, *time );
        XI_CHECK_CND_DBGMESSAGE( num_chars_time <= 0, XI_SERIALIZATION_ERROR, state,
                                 "ERROR: snprintf returned error for sizing time" );

        /* allocate +1 becuase snprintf size computation
         * doesn't include null termiantor */
        XI_ALLOC_BUFFER_AT( char, time_string, num_chars_time + 1, state );

        int num_formatted_chars =
            snprintf( time_string, num_chars_time + 1, "%" PRIu32, *time );
        XI_CHECK_CND_DBGMESSAGE( num_formatted_chars != num_chars_time,
                                 XI_SERIALIZATION_ERROR, state,
                                 "ERROR: Final Size / Computed Size Mismatch" );
    }

    /* string value is optional */
    if ( NULL == in_string_value )
    {
        string_value           = empty_string;
        num_chars_string_value = 0;
    }
    else
    {
        /* ensure that our string value is formatted correctly
         * and compute its size. */
        state = check_csv_entry( in_string_value, &num_chars_string_value );
        XI_CHECK_STATE( state );
        string_value = in_string_value;
    }

    /* numeric_value is optional */
    if ( NULL == in_category )
    {
        category           = empty_string;
        num_chars_category = 0;
    }
    else
    {
        /* ensure that our category string is formatted correctly
         * and compute its size. */
        state = check_csv_entry( in_category, &num_chars_category );
        XI_CHECK_STATE( state );
        category = in_category;
    }

    if ( NULL == in_numeric_value )
    {
        XI_ALLOC_BUFFER_AT( char, numeric_string, 1, state );
        numeric_string[0]       = '\0';
        num_chars_numeric_value = 0;
    }
    else
    {
        num_chars_numeric_value =
            snprintf( compute_size_buf, 1, "%g", *in_numeric_value );
        XI_CHECK_CND_DBGMESSAGE(
            num_chars_numeric_value <= 0, XI_SERIALIZATION_ERROR, state,
            "ERROR: snprintf returned error for sizing numeric_value" );

        /* allocate +1 because snprintf size computation
         * doesn't include null termiantor */
        XI_ALLOC_BUFFER_AT( char, numeric_string, num_chars_numeric_value + 1, state );

        int num_formatted_numeric_chars = snprintf(
            numeric_string, num_chars_numeric_value + 1, "%g", *in_numeric_value );
        XI_CHECK_CND_DBGMESSAGE( num_formatted_numeric_chars != num_chars_numeric_value,
                                 XI_SERIALIZATION_ERROR, state,
                                 "ERROR: Final Size / Computed Size Mismatch" );
    }

    /* all of these are optional but we need at least one! */
    XI_CHECK_CND_DBGMESSAGE(
        ( 0 == num_chars_category ) && ( 0 == num_chars_string_value ) &&
            ( 0 == num_chars_numeric_value ),
        XI_INVALID_PARAMETER, state,
        "ERROR: Need at least one of category, string value, or numeric_value" );

    total_size = num_chars_time + num_chars_category + num_chars_string_value +
                 num_chars_numeric_value + num_control_chars;

    XI_ALLOC_BUFFER_AT( char, final_buffer, total_size, state );

    /* format the result */
    final_size = snprintf( final_buffer, total_size, format_template, time_string,
                           category, numeric_string, string_value );

    XI_CHECK_CND_DBGMESSAGE( final_size == total_size, XI_SERIALIZATION_ERROR, state,
                             "ERROR: Final Size / Computed Size Mismatch" );

    data_desc = xi_make_desc_from_string_copy( final_buffer );

    XI_CHECK_MEMORY( data_desc, state );

    state = xi_publish_data_impl( xih, topic, data_desc, qos, XI_MQTT_RETAIN_FALSE,
                                  callback, user_data, XI_THREADID_THREAD_0 );

err_handling:

    XI_SAFE_FREE( time_string );
    XI_SAFE_FREE( numeric_string );
    XI_SAFE_FREE( final_buffer );

    return state;
}

xi_state_t xi_publish_data( xi_context_handle_t xih,
                            const char* topic,
                            const uint8_t* data,
                            size_t data_len,
                            const xi_mqtt_qos_t qos,
                            const xi_mqtt_retain_t retain,
                            xi_user_callback_t* callback,
                            void* user_data )
{
    /* PRE-CONDITIONS */
    assert( NULL != topic );
    assert( NULL != data );
    assert( 0 != data_len );

    xi_state_t state = XI_STATE_OK;

    xi_data_desc_t* data_desc = xi_make_desc_from_buffer_copy( data, data_len );

    XI_CHECK_MEMORY( data_desc, state );

    return xi_publish_data_impl( xih, topic, data_desc, qos, retain, callback, user_data,
                                 XI_THREADID_THREAD_0 );

err_handling:
    return state;
}

xi_state_t xi_subscribe_impl( xi_context_handle_t xih,
                              const char* topic,
                              const xi_mqtt_qos_t qos,
                              xi_user_subscription_callback_t* callback,
                              void* user_data,
                              xi_thread_id_t callback_target_thread_id )
{
    if ( ( XI_INVALID_CONTEXT_HANDLE == xih ) || ( NULL == topic ) ||
         ( NULL == callback ) )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t state               = XI_STATE_OK;
    xi_mqtt_logic_task_t* task     = NULL;
    char* internal_topic           = NULL;
    xi_layer_t* input_layer        = NULL;
    xi_event_handle_t event_handle = xi_make_empty_event_handle();

    xi_context_t* xi =
        ( xi_context_t* )xi_object_for_handle( xi_globals.context_handles_vector, xih );

    XI_CHECK_MEMORY( xi, state );

    event_handle = xi_make_threaded_handle(
        callback_target_thread_id, &xi_user_sub_call_wrapper, xi, NULL, XI_STATE_OK,
        ( void* )callback, ( void* )user_data, ( void* )NULL );

    if ( XI_BACKOFF_CLASS_NONE != xi_globals.backoff_status.backoff_class )
    {
        return XI_BACKOFF_TERMINAL;
    }

    /* copy the topic memory */
    /* Olgierd: I'm not sure whether it should be copied or not ? Ususally people will do
     * the const char* const topic_name = "topic_name"; in such cases copying doesn't make
     * any sense, in other cases like our test driver we will use dynamically allocted
     * memory for topic. */
    internal_topic = xi_str_dup( topic );

    XI_CHECK_MEMORY( internal_topic, state );

    input_layer = xi->layer_chain.top;

    event_handle.handlers.h6.a1 = xi;

    task = xi_mqtt_logic_make_subscribe_task( internal_topic, qos, event_handle );
    XI_CHECK_MEMORY( task, state );

    /* pass the partial ownership of the task data to the handler ( in case of
     * subscription failure it will release the memory ) */
    task->data.data_u->subscribe.handler.handlers.h6.a6 = task->data.data_u;

    return XI_PROCESS_PUSH_ON_THIS_LAYER( &input_layer->layer_connection, task,
                                          XI_STATE_OK );

err_handling:
    if ( task )
    {
        xi_mqtt_logic_free_task( &task );
    }

    XI_SAFE_FREE( internal_topic );

    return state;
}

xi_state_t xi_subscribe( xi_context_handle_t xih,
                         const char* topic,
                         const xi_mqtt_qos_t qos,
                         xi_user_subscription_callback_t* callback,
                         void* user_data )
{
    return xi_subscribe_impl( xih, topic, qos, callback, user_data,
                              XI_THREADID_THREAD_0 );
}

xi_state_t xi_shutdown_connection_impl( xi_context_t* xi )
{
    assert( NULL != xi );

    xi_state_t state           = XI_STATE_OK;
    xi_layer_t* input_layer    = xi->layer_chain.top;
    xi_mqtt_logic_task_t* task = NULL;

    /* check if connect operation has been finished */
    if ( NULL == xi->context_data.connect_handler.ptr_to_position )
    {
        /* check if the connection is not established for any reason */
        if ( NULL == xi->context_data.connection_data ||
             XI_CONNECTION_STATE_OPENED !=
                 xi->context_data.connection_data->connection_state )
        {
            return XI_SOCKET_NO_ACTIVE_CONNECTION_ERROR;
        }
    }
    else
    {
        /* if the connect operation has been scheduled but not executed */
        state = xi_evtd_cancel( xi->context_data.evtd_instance,
                                &xi->context_data.connect_handler );

        XI_CHECK_STATE( state );

        return XI_STATE_OK;
    }

    switch ( xi->context_data.shutdown_state )
    {
        case XI_SHUTDOWN_UNINITIALISED:
            xi->context_data.shutdown_state = XI_SHUTDOWN_STARTED;
            break;
        case XI_SHUTDOWN_STARTED:
            xi_debug_logger( "XI_ALREADY_INITIALIZED" );
            return XI_ALREADY_INITIALIZED;
        default:
            return XI_INTERNAL_ERROR;
    }

    task = xi_mqtt_logic_make_shutdown_task();

    XI_CHECK_MEMORY( task, state );

    return XI_PROCESS_PUSH_ON_THIS_LAYER( &input_layer->layer_connection, task,
                                          XI_STATE_OK );

err_handling:
    if ( task )
    {
        xi_mqtt_logic_free_task( &task );
    }
    return state;
}

xi_state_t xi_shutdown_connection( xi_context_handle_t xih )
{
    assert( XI_INVALID_CONTEXT_HANDLE < xih );

    xi_context_t* xi = xi_object_for_handle( xi_globals.context_handles_vector, xih );

    return xi_shutdown_connection_impl( xi );
}

xi_timed_task_handle_t xi_schedule_timed_task( xi_context_handle_t xih,
                                               xi_user_task_callback_t* callback,
                                               const xi_time_t seconds_from_now,
                                               const uint8_t repeats_forever,
                                               void* data )
{
    return xi_add_timed_task( xi_globals.timed_tasks_container, xi_globals.evtd_instance,
                              xih, callback, seconds_from_now, repeats_forever, data );
}

void xi_cancel_timed_task( xi_timed_task_handle_t timed_task_handle )
{
    xi_remove_timed_task( xi_globals.timed_tasks_container, timed_task_handle );
}

xi_state_t xi_set_maximum_heap_usage( const size_t max_bytes )
{
#ifndef XI_MEMORY_LIMITER_ENABLED
    XI_UNUSED( max_bytes );
    return XI_NOT_SUPPORTED;
#else
    return xi_memory_limiter_set_limit( max_bytes );
#endif
}

xi_state_t xi_get_heap_usage( size_t* const heap_usage )
{
#ifndef XI_MEMORY_LIMITER_ENABLED
    XI_UNUSED( heap_usage );
    return XI_NOT_SUPPORTED;
#else
    if ( NULL == heap_usage )
    {
        return XI_INVALID_PARAMETER;
    }

    *heap_usage = xi_memory_limiter_get_allocated_space();
    return XI_STATE_OK;
#endif
}


#ifdef XI_EXPOSE_FS
xi_state_t xi_set_fs_functions( const xi_fs_functions_t fs_functions )
{
    /* check the size of the passed structure */
    if ( sizeof( xi_fs_functions_t ) != fs_functions.fs_functions_size )
    {
        return XI_INTERNAL_ERROR;
    }

    /* check if any of function pointer is NULL */
    if ( NULL == fs_functions.stat_resource )
    {
        return XI_INVALID_PARAMETER;
    }
    else if ( NULL == fs_functions.open_resource )
    {
        return XI_INVALID_PARAMETER;
    }
    else if ( NULL == fs_functions.read_resource )
    {
        return XI_INVALID_PARAMETER;
    }
    else if ( NULL == fs_functions.write_resource )
    {
        return XI_INVALID_PARAMETER;
    }
    else if ( NULL == fs_functions.remove_resource )
    {
        return XI_INVALID_PARAMETER;
    }

    /* discuss this as this may be potentially dangerous */
    memcpy( &xi_internals.fs_functions, &fs_functions, sizeof( xi_fs_functions_t ) );

    return XI_STATE_OK;
}

#endif /* XI_EXPOSE_FS */

#ifdef __cplusplus
}
#endif
