/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_context.h>
#include <xi_handle.h>
#include <xi_globals.h>

#include <xi_mqtt_logic_layer_data.h>
#include <xi_backoff_status_api.h>
#include <xi_thread_threadpool.h>
#include <xi_layer_macros.h>
#include <xi_layer_stack.h>

/**
 * @brief helper function used to clean and free the protocol specific in-context data
 **/
static void xi_free_context_data( xi_context_t* context )
{
    assert( NULL != context );

    xi_context_data_t* context_data = ( xi_context_data_t* )&context->context_data;

    assert( NULL != context_data );

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
    if ( NULL == *context )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t state = XI_STATE_OK;

    state = xi_delete_handle_for_object( xi_globals.context_handles_vector, *context );

    if ( XI_STATE_OK != state )
    {
        return XI_ELEMENT_NOT_FOUND;
    }

    xi_layer_chain_delete( &( ( *context )->layer_chain ), layer_chain_size,
                           layer_config );

    xi_free_context_data( *context );

    XI_SAFE_FREE( *context );

    xi_globals.globals_ref_count -= 1;

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
    }

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
