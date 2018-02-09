/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_control_topic_layer.h"
#ifdef XI_CONTROL_TOPIC_ENABLED
#include "xi_control_topic_layer_data.h"
#include "xi_handle.h"
#include "xi_user_sub_call_wrapper.h"
#ifdef XI_SECURE_FILE_TRANSFER_ENABLED
#include "xi_cbor_codec_ct.h"
#endif
#include "xi_control_message.h"
#endif
#include "xi_macros.h"
#include "xi_layer_api.h"
#include "xi_types.h"
#include "xi_event_thread_dispatcher.h"
#include "xi_globals.h"
#include "xi_event_handle.h"
#include "xi_mqtt_logic_layer_data.h"
#include "xi_control_topic_name.h"
#include "xi_mqtt_message.h"
#include "xi_layer_macros.h"
#include "xi_list.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef XI_CONTROL_TOPIC_ENABLED

#ifdef XI_SECURE_FILE_TRANSFER_ENABLED
/**
 * @brief xi_control_topic_publish_on_topic
 *
 * This function sends the given buffer through the control topic
 */
static xi_state_t
xi_control_topic_publish_on_topic( void* context, xi_control_message_t* control_message );
#endif

/**
 * @brief xi_topic_name_is_control_and_release
 *
 * Predicate helper for control topics from subscription list
 */
static int8_t xi_topic_name_is_control_and_release( union xi_vector_selector_u* data );

/**
 * @brief xi_on_control_message
 *
 * This is the entry point of all other operations related to XI_CONTEXT_DATA
 *decoding
 * and working with control topic messages
 */
static xi_state_t xi_on_control_message( xi_context_handle_t in_context_handle,
                                         xi_sub_call_type_t call_type,
                                         const xi_sub_call_params_t* const params,
                                         xi_state_t state,
                                         void* user_data );

/**
 * @brief xi_control_topic_subscribe
 *
 * Helper function that sends subscription requests and setts up control topic
 * handler
 */
static xi_state_t
xi_control_topic_subscribe( void* context, char* subscribe_control_topic_name );

/**
 * @brief xi_control_topic_connection_state_changed
 *
 * This is helper for handling notification to the user callback
 * about changes of the connection states
 */
static xi_state_t
xi_control_topic_connection_state_changed( void* context, xi_state_t state );

#ifdef XI_SECURE_FILE_TRANSFER_ENABLED
static xi_state_t
xi_control_topic_publish_on_topic( void* context, xi_control_message_t* control_message )
{
    if ( NULL == control_message )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t local_state                = XI_STATE_OK;
    xi_mqtt_logic_task_t* mqtt_logic_task = NULL;

    xi_control_topic_layer_data_t* layer_data =
        ( xi_control_topic_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    assert( NULL != layer_data );

    xi_data_desc_t* encoded_message_data_desc = NULL;
    { /* CBOR encoding */
        uint8_t* encoded_message     = NULL;
        uint32_t encoded_message_len = 0;

        xi_cbor_codec_ct_encode( control_message, &encoded_message,
                                 &encoded_message_len );

        xi_debug_control_message_dump( control_message, "OUTGOING" );

        xi_control_message_free( &control_message );

        XI_CHECK_CND_DBGMESSAGE( NULL == encoded_message || 0 == encoded_message_len,
                                 XI_SERIALIZATION_ERROR, local_state,
                                 "ERROR: control topic CBOR encoding failed" );

        encoded_message_data_desc =
            xi_make_desc_from_buffer_share( encoded_message, encoded_message_len );
        // todo_atigyi: make more elegant ownership passing
        encoded_message_data_desc->memory_type = XI_MEMORY_TYPE_MANAGED;

        XI_CHECK_CND_DBGMESSAGE( NULL == encoded_message_data_desc,
                                 XI_SERIALIZATION_ERROR, local_state,
                                 "ERROR: failed to create data_desc from CBOR binary " );
    }

    mqtt_logic_task = xi_mqtt_logic_make_publish_task(
        layer_data->publish_topic_name, encoded_message_data_desc,
        XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, xi_make_empty_handle() );

    XI_CHECK_MEMORY( mqtt_logic_task, local_state );

    return XI_PROCESS_PUSH_ON_THIS_LAYER( context, mqtt_logic_task, XI_STATE_OK );

err_handling:
    return local_state;
}
#endif

static int8_t xi_topic_name_is_control_and_release( union xi_vector_selector_u* data )
{
    assert( data != NULL );
    assert( data->ptr_value != NULL );

    xi_mqtt_task_specific_data_t* task_spec_data = data->ptr_value;

    assert( task_spec_data->subscribe.topic != NULL );

    if ( memcmp( task_spec_data->subscribe.topic, XI_TOPIC_DOMAIN,
                 strlen( XI_TOPIC_DOMAIN ) ) == 0 )
    {
        xi_mqtt_task_spec_data_free_subscribe_data(
            ( xi_mqtt_task_specific_data_t** )&data->ptr_value );

        return 1;
    }

    return 0;
}

xi_state_t xi_on_control_message( xi_context_handle_t in_context_handle,
                                  xi_sub_call_type_t call_type,
                                  const xi_sub_call_params_t* const params,
                                  xi_state_t state,
                                  void* user_data )
{
    XI_UNUSED( in_context_handle );

    xi_context_t* context = ( xi_context_t* )user_data;

    assert( NULL != context );

    xi_control_topic_layer_data_t* layer_data =
        ( xi_control_topic_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    assert( layer_data != NULL );
    XI_UNUSED( layer_data );

    switch ( call_type )
    {
        case XI_SUB_CALL_SUBACK:
        {
            if ( params->suback.suback_status == XI_MQTT_SUBACK_FAILED )
            {
                xi_debug_logger( "Subscription to control topic failed." );

#ifdef XI_SECURE_FILE_TRANSFER_ENABLED
                xi_sft_on_connection_failed( layer_data->sft_context );
#endif
            }
            else
            {
                xi_debug_format( "Subscription to control topic successfull with QoS %d",
                                 params->suback.suback_status );

#ifdef XI_SECURE_FILE_TRANSFER_ENABLED
                xi_sft_on_connected( layer_data->sft_context );
#endif
            }
        }
        case XI_SUB_CALL_MESSAGE:
        {
            xi_debug_format( "received data on control topic length: %zu ",
                             params->message.temporary_payload_data_length );

#ifdef XI_SECURE_FILE_TRANSFER_ENABLED
            /* CBOR decoding */
            xi_control_message_t* control_message =
                xi_cbor_codec_ct_decode( params->message.temporary_payload_data,
                                         params->message.temporary_payload_data_length );

            xi_debug_control_message_dump( control_message, "INCOMING" );

            xi_sft_on_message( layer_data->sft_context, control_message );
#endif
        }

        default:;
    }

    return state;
}

xi_state_t xi_control_topic_subscribe( void* context, char* subscribe_control_topic_name )
{
    assert( context != NULL );

    /* local state */
    xi_state_t local_state = XI_STATE_OK;

    /* form a subscribe task */
    xi_mqtt_logic_task_t* task = NULL;

    xi_event_handle_t handler = xi_make_threaded_handle(
        XI_THREADID_MAINTHREAD, &xi_user_sub_call_wrapper, NULL, NULL, XI_STATE_OK,
        ( void* )&xi_on_control_message, ( void* )context, ( void* )NULL );

    /* create the proper task */
    task = xi_mqtt_logic_make_subscribe_task( subscribe_control_topic_name,
                                              XI_MQTT_QOS_AT_LEAST_ONCE, handler );

    XI_CHECK_MEMORY( task, local_state );

    task->session_state = XI_MQTT_LOGIC_TASK_SESSION_DO_NOT_STORE;

    /* update the task callback arguments */
    task->data.data_u->subscribe.handler.handlers.h6.a6 = task->data.data_u;

    return XI_PROCESS_PUSH_ON_PREV_LAYER( context, task, XI_STATE_OK );

err_handling:
    return local_state;
}
#endif

xi_state_t xi_control_topic_connection_state_changed( void* context, xi_state_t state )
{
    xi_debug_printf( "%s\n", __FUNCTION__ );

    XI_CONTEXT_DATA( context )->connection_callback.handlers.h3.a2 =
        XI_CONTEXT_DATA( context )->connection_data;

    XI_CONTEXT_DATA( context )->connection_callback.handlers.h3.a3 = state;

    xi_evttd_execute( XI_CONTEXT_DATA( context )->evtd_instance,
                      XI_CONTEXT_DATA( context )->connection_callback );

    if ( state == XI_STATE_OK &&
         XI_CONTEXT_DATA( context )->connection_data->connection_state ==
             XI_CONNECTION_STATE_OPENED )
    {
        XI_PROCESS_POST_CONNECT_ON_THIS_LAYER( context, NULL, state );
    }

    return state;
}

xi_state_t
xi_control_topic_layer_push( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( data );

    return XI_PROCESS_PUSH_ON_PREV_LAYER( context, data, in_out_state );
}

xi_state_t
xi_control_topic_layer_pull( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( data );
    XI_UNUSED( context );

    return in_out_state;
}

xi_state_t
xi_control_topic_layer_init( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    xi_debug_logger( "control topic layer initializing.. " );

#ifdef XI_CONTROL_TOPIC_ENABLED
    xi_control_topic_layer_data_t* layer_data =
        ( xi_control_topic_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    if ( NULL == layer_data )
    {
        XI_ALLOC_AT( xi_control_topic_layer_data_t, XI_THIS_LAYER( context )->user_data,
                     in_out_state );

        layer_data =
            ( xi_control_topic_layer_data_t* )XI_THIS_LAYER( context )->user_data;

#ifdef XI_SECURE_FILE_TRANSFER_ENABLED
        xi_sft_make_context( &layer_data->sft_context,
                             ( const char** )XI_CONTEXT_DATA( context )->updateable_files,
                             XI_CONTEXT_DATA( context )->updateable_files_count,
                             &xi_control_topic_publish_on_topic,
                             XI_CONTEXT_DATA( context )->sft_url_handler_callback,
                             context );
#endif
    }

    assert( NULL != layer_data );


#endif

    return XI_PROCESS_INIT_ON_PREV_LAYER( context, data, in_out_state );

#ifdef XI_CONTROL_TOPIC_ENABLED
err_handling:
    XI_SAFE_FREE( XI_THIS_LAYER( context )->user_data );
    return in_out_state;
#endif
}

xi_state_t
xi_control_topic_layer_connect( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( data );

#ifdef XI_CONTROL_TOPIC_ENABLED
    xi_control_topic_layer_data_t* layer_data =
        ( xi_control_topic_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    assert( layer_data != NULL );

    char* subscribe_control_topic_name = NULL;

    /* if the other layers has been connected succesfully let's try to subscribe to a
     * control topic */
    if ( XI_STATE_OK == in_out_state )
    {
        /* let's create the topic name */
        in_out_state = xi_control_topic_create_topic_name(
            XI_CONTEXT_DATA( context )->connection_data->username,
            &subscribe_control_topic_name, &layer_data->publish_topic_name );

        XI_CHECK_STATE( in_out_state );

        in_out_state = xi_control_topic_connection_state_changed( context, in_out_state );
        XI_CHECK_STATE( in_out_state );

        xi_debug_format( "subscribing on topic: %s", subscribe_control_topic_name );

        in_out_state =
            xi_control_topic_subscribe( context, subscribe_control_topic_name );
        XI_CHECK_STATE( in_out_state );

        return in_out_state;
    }

err_handling:

    XI_SAFE_FREE( subscribe_control_topic_name );

#else

    if ( XI_STATE_OK == in_out_state )
    {
        return xi_control_topic_connection_state_changed( context, in_out_state );
    }

#endif

    return XI_PROCESS_CLOSE_ON_PREV_LAYER( context, 0, in_out_state );
}

xi_state_t
xi_control_topic_layer_close( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( data );

    /* perform complete shutdown */
    XI_CONTEXT_DATA( context )->connection_data->connection_state =
        XI_CONNECTION_STATE_CLOSING;

    return XI_PROCESS_CLOSE_ON_PREV_LAYER( context, data, in_out_state );
}

xi_state_t xi_control_topic_layer_close_externally( void* context,
                                                    void* data,
                                                    xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( data );

    if ( XI_CONTEXT_DATA( context )->connection_data->connection_state ==
         XI_CONNECTION_STATE_OPENING )
    {
        XI_CONTEXT_DATA( context )->connection_data->connection_state =
            XI_CONNECTION_STATE_OPEN_FAILED;
    }
    else
    {
        XI_CONTEXT_DATA( context )->connection_data->connection_state =
            XI_CONNECTION_STATE_CLOSED;
    }

#ifdef XI_CONTROL_TOPIC_ENABLED
    xi_control_topic_layer_data_t* layer_data =
        ( xi_control_topic_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    if ( layer_data != NULL )
    {
        /* release memory required for topic names */
        XI_SAFE_FREE( layer_data->publish_topic_name );

#ifdef XI_SECURE_FILE_TRANSFER_ENABLED
        xi_sft_free_context( &layer_data->sft_context );
#endif

        /* release layer memory */
        XI_SAFE_FREE( XI_THIS_LAYER( context )->user_data );
    }

    /* if there is clean session and control topics remain
       as a copy we have to remove them from the list */
    if ( XI_THIS_LAYER( context )->context_data->copy_of_handlers_for_topics != NULL )
    {
        xi_vector_remove_if(
            XI_THIS_LAYER( context )->context_data->copy_of_handlers_for_topics,
            &xi_topic_name_is_control_and_release );
    }
#endif

    /* call the connection callback to notify the user */
    return xi_control_topic_connection_state_changed( context, in_out_state );
}

#ifdef __cplusplus
}
#endif
