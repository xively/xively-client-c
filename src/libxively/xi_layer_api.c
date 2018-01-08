/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_config.h"
#include "xi_layer_api.h"
#include "xi_globals.h"
#include "xi_event_thread_dispatcher.h"

/**
 * @brief get_next_layer_state function that checks what should be the next
 * layer state according to some very simple rules related to which function on which
 * layer is being called
 *
 * @param func next function
 * @param from_context layer's context from which we are moving
 * @param context target context of function invocation
 * @param state state in which the transition is happening
 * @return next layer state
 */
static xi_layer_state_t get_next_layer_state( xi_layer_func_t* func,
                                              xi_layer_connectivity_t* from_context,
                                              xi_layer_connectivity_t* context,
                                              xi_state_t state )
{
    if ( func == XI_THIS_LAYER( context )->layer_funcs->close )
    {
        if ( XI_THIS_LAYER_STATE( from_context ) == XI_LAYER_STATE_CONNECTED )
        {
            return XI_LAYER_STATE_CLOSING;
        }
        else
        {
            return XI_THIS_LAYER_STATE( from_context );
        }
    }
    else if ( func == XI_THIS_LAYER( context )->layer_funcs->close_externally )
    {
        return XI_LAYER_STATE_CLOSED;
    }
    else if ( func == XI_THIS_LAYER( context )->layer_funcs->connect )
    {
        return state == XI_STATE_OK ? XI_LAYER_STATE_CONNECTED : XI_LAYER_STATE_CLOSED;
    }
    else if ( func == XI_THIS_LAYER( context )->layer_funcs->init )
    {
        return XI_LAYER_STATE_CONNECTING;
    }
    else if ( func == XI_THIS_LAYER( context )->layer_funcs->pull )
    {
        return XI_THIS_LAYER_STATE( from_context );
    }
    else if ( func == XI_THIS_LAYER( context )->layer_funcs->push )
    {
        return XI_THIS_LAYER_STATE( from_context );
    }
    else if ( func == XI_THIS_LAYER( context )->layer_funcs->post_connect )
    {
        return XI_THIS_LAYER_STATE( from_context );
    }

    return XI_LAYER_STATE_NONE;
}

#if XI_DEBUG_EXTRA_INFO

xi_state_t xi_layer_continue_with_impl( xi_layer_func_t* func,
                                        xi_layer_connectivity_t* from_context,
                                        xi_layer_connectivity_t* context,
                                        void* data,
                                        xi_state_t state,
                                        const char* file_name,
                                        const int line_no )
{
    xi_state_t local_state = XI_STATE_OK;

    if ( NULL != context )
    {
        context->self->debug_info.debug_file_last_call = file_name;
        context->self->debug_info.debug_line_last_call = line_no;
    }

    if ( func != NULL )
    {
        xi_event_handle_queue_t* e_ptr =
            xi_evttd_execute( XI_CONTEXT_DATA( context )->evtd_instance,
                              xi_make_handle( func, context, data, state ) );
        XI_CHECK_MEMORY( e_ptr, local_state );

        xi_layer_state_t next_state =
            get_next_layer_state( func, from_context, context, state );

        XI_THIS_LAYER_STATE_UPDATE( from_context, next_state );
    }

err_handling:
    return local_state;
}

#else /* XI_DEBUG_EXTRA_INFO */

xi_state_t xi_layer_continue_with_impl( xi_layer_func_t* func,
                                        xi_layer_connectivity_t* from_context,
                                        xi_layer_connectivity_t* context,
                                        void* data,
                                        xi_state_t state )
{
    xi_state_t local_state = XI_STATE_OK;

    if ( func != NULL )
    {
        xi_event_handle_queue_t* e_ptr =
            xi_evttd_execute( XI_CONTEXT_DATA( context )->evtd_instance,
                              xi_make_handle( func, context, data, state ) );
        XI_CHECK_MEMORY( e_ptr, local_state );

        xi_layer_state_t next_state =
            get_next_layer_state( func, from_context, context, state );

        XI_THIS_LAYER_STATE_UPDATE( from_context, next_state );
    }

err_handling:
    return local_state;
}

#endif
