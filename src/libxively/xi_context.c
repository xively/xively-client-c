/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_context.h>
#include <xi_handle.h>
#include <xi_globals.h>

#include <xi_backoff_lut_config.h>
#include <xi_mqtt_logic_layer_data.h>
#include <xi_backoff_status_api.h>
#include <xi_thread_threadpool.h>
#include <xi_layer_macros.h>
#include <xi_layer_stack.h>

#include <xively.h>
#include <xi_macros.h>

#ifndef XI_MAX_NUM_CONTEXTS
#define XI_MAX_NUM_CONTEXTS 10
#endif

void xi_globals_remove_reference();

xi_state_t xi_globals_add_reference()
{
    xi_state_t state = XI_STATE_OK;

    xi_globals.globals_ref_count += 1;

    if ( 1 == xi_globals.globals_ref_count )
    {
        xi_globals.evtd_instance = xi_evtd_create_instance();

        XI_CHECK_STATE( state = xi_backoff_configure_using_data(
                            ( xi_vector_elem_t* )XI_BACKOFF_LUT,
                            ( xi_vector_elem_t* )XI_DECAY_LUT,
                            XI_ARRAYSIZE( XI_BACKOFF_LUT ), XI_MEMORY_TYPE_UNMANAGED ) );

        XI_CHECK_MEMORY( xi_globals.evtd_instance, state );

        /* note: this is NULL if thread module is disabled */
        xi_globals.main_threadpool = xi_threadpool_create_instance( 1 );

        xi_globals.context_handles_vector = xi_vector_create();
        xi_globals.timed_tasks_container  = xi_make_timed_task_container();

        xi_globals.context_handles_vector_edge_devices = xi_vector_create();
    }

    return XI_STATE_OK;

err_handling:

    xi_globals_remove_reference();

    return state;
}

void xi_globals_remove_reference()
{
    xi_globals.globals_ref_count =
        XI_MAX( ( int8_t )0, ( int8_t )( xi_globals.globals_ref_count ) - 1 );

    if ( 0 == xi_globals.globals_ref_count )
    {
        xi_cancel_backoff_event();
        xi_backoff_release();
        xi_evtd_destroy_instance( xi_globals.evtd_instance );
        xi_globals.evtd_instance = NULL;
        xi_threadpool_destroy_instance( &xi_globals.main_threadpool );

        xi_vector_destroy( xi_globals.context_handles_vector );
        xi_globals.context_handles_vector = NULL;

        xi_destroy_timed_task_container( xi_globals.timed_tasks_container );
        xi_globals.timed_tasks_container = NULL;

        xi_vector_destroy( xi_globals.context_handles_vector_edge_devices );
    }
}

xi_state_t xi_create_context_with_custom_layers_and_evtd(
    xi_context_t** context,
    xi_layer_type_t layer_config[],
    xi_layer_type_id_t layer_chain[],
    size_t layer_chain_size,
    xi_evtd_instance_t* event_dispatcher,
    uint8_t handle_support ) /* PLEASE NOTE: Event dispatcher's ownership is not taken
                                here! */
{
    xi_state_t state = XI_STATE_OK;

    if ( NULL == context )
    {
        return XI_INVALID_PARAMETER;
    }

    if ( NULL == xi_globals.str_account_id )
    {
        return XI_NOT_INITIALIZED;
    }

    *context = NULL;

    /* hold a reference on the globals variable */
    state = xi_globals_add_reference();

    XI_CHECK_STATE( state );

    /* allocate the structure to store new context */
    XI_ALLOC_AT( xi_context_t, *context, state );

    /* create io timeout vector */
    ( *context )->context_data.io_timeouts         = xi_vector_create();
    ( *context )->context_data.main_context_handle = XI_INVALID_CONTEXT_HANDLE;

    XI_CHECK_MEMORY( ( *context )->context_data.io_timeouts, state );

    /* set the event dispatcher to the global one, if none is provided */
    ( *context )->context_data.evtd_instance =
        ( NULL == event_dispatcher ) ? xi_globals.evtd_instance : event_dispatcher;

    ( *context )->protocol = XI_MQTT;

    ( *context )->layer_chain = xi_layer_chain_create(
        layer_chain, layer_chain_size, &( *context )->context_data, layer_config );

    if ( 0 != handle_support )
    {
        XI_CHECK_STATE(
            state = xi_register_handle_for_object( xi_globals.context_handles_vector,
                                                   XI_MAX_NUM_CONTEXTS, *context ) );
    }

    return XI_STATE_OK;

err_handling:

    XI_SAFE_FREE( *context );

    return state;
}

xi_context_handle_t xi_create_context()
{
    xi_context_t* context = NULL;
    xi_state_t state      = XI_STATE_OK;

    XI_CHECK_STATE( state = xi_create_context_with_custom_layers_and_evtd(
                        &context, xi_layer_types_g, XI_LAYER_CHAIN_DEFAULT,
                        XI_LAYER_CHAIN_DEFAULTSIZE_SUFFIX, NULL, 1 ) );

    xi_context_handle_t context_handle = 0;
    XI_CHECK_STATE( state = xi_find_handle_for_object( xi_globals.context_handles_vector,
                                                       context, &context_handle ) );

    return context_handle;

err_handling:

    return -state;
}


/**
 * @brief helper function used to clean and free the protocol specific in-context data
 **/
static void xi_free_context_data( xi_context_data_t* context_data )
{
    if ( NULL == context_data )
    {
        return;
    }

    /* destroy timeout */
    xi_vector_destroy( context_data->io_timeouts );

    /* see comment in xi_types.h */
    if ( context_data->copy_of_handlers_for_topics )
    {
        xi_vector_for_each( context_data->copy_of_handlers_for_topics,
                            &xi_mqtt_task_spec_data_free_subscribe_data_vec, NULL, 0 );

        context_data->copy_of_handlers_for_topics =
            xi_vector_destroy( context_data->copy_of_handlers_for_topics );
    }

    if ( context_data->copy_of_q12_unacked_messages_queue )
    {
        /* this pointer must be present otherwise we are not going to be able to release
         * mqtt specific data */
        assert( NULL != context_data->copy_of_q12_unacked_messages_queue_dtor_ptr );
        context_data->copy_of_q12_unacked_messages_queue_dtor_ptr(
            &context_data->copy_of_q12_unacked_messages_queue );
    }

    {
        uint16_t id_file = 0;
        for ( ; id_file < context_data->updateable_files_count; ++id_file )
        {
            XI_SAFE_FREE( context_data->updateable_files[id_file] );
        }

        XI_SAFE_FREE( context_data->updateable_files );
    }

    xi_free_connection_data( &context_data->connection_data );

    /* remember, event dispatcher ownership is not taken, this is why we don't delete
     * it. */
    context_data->evtd_instance = NULL;
}

xi_state_t xi_delete_context_with_custom_layers( xi_context_t** context,
                                                 xi_layer_type_t layer_config[],
                                                 size_t layer_chain_size )
{
    if ( NULL == context || NULL == *context )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_delete_handle_for_object( xi_globals.context_handles_vector, *context );

    xi_layer_chain_delete( &( ( *context )->layer_chain ), layer_chain_size,
                           layer_config );

    xi_free_context_data( &( *context )->context_data );

    XI_SAFE_FREE( *context );

    xi_globals_remove_reference();

    return XI_STATE_OK;
}

xi_state_t xi_delete_context( xi_context_handle_t context_handle )
{
    xi_context_t* context =
        xi_object_for_handle( xi_globals.context_handles_vector, context_handle );
    assert( context != NULL );

    return xi_delete_context_with_custom_layers(
        &context, xi_layer_types_g,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_DEFAULT ) );
}
