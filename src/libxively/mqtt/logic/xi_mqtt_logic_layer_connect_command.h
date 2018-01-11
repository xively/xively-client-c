/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_MQTT_LOGIC_LAYER_CONNECT_COMMAND_H__
#define __XI_MQTT_LOGIC_LAYER_CONNECT_COMMAND_H__

#include "xi_layer_api.h"
#include "xi_mqtt_logic_layer_data.h"
#include "xi_coroutine.h"
#include "xi_mqtt_message.h"
#include "xi_mqtt_logic_layer_data_helpers.h"
#include "xi_mqtt_logic_layer_task_helpers.h"
#include "xi_globals.h"
#include "xi_backoff_status_api.h"
#include "xi_event_thread_dispatcher.h"
#include "xi_io_timeouts.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "xi_mqtt_logic_layer_keepalive_handler.h"

static inline xi_state_t get_error_from_connack( int return_code )
{
    switch ( return_code )
    {
        case 0x01:
            return XI_MQTT_UNACCEPTABLE_PROTOCOL_VERSION;
        case 0x02:
            return XI_MQTT_IDENTIFIER_REJECTED;
        case 0x03:
            return XI_MQTT_SERVER_UNAVAILIBLE;
        case 0x04:
            return XI_MQTT_BAD_USERNAME_OR_PASSWORD;
        case 0x05:
            return XI_MQTT_NOT_AUTHORIZED;
        default:
            return XI_MQTT_CONNECT_UNKNOWN_RETURN_CODE;
    }
}

/**
 * @brief do_mqtt_connect_timeout
 *
 * Connection timeout, remove event from timeout event vector, exit
 */

static inline xi_event_handle_return_t
do_mqtt_connect_timeout( xi_event_handle_arg1_t arg1, xi_event_handle_arg2_t arg2 )
{
    xi_layer_connectivity_t* context = ( xi_layer_connectivity_t* )arg1;
    xi_mqtt_logic_task_t* task       = ( xi_mqtt_logic_task_t* )arg2;

    /* remove finished timeout event from context's io timeout vector */
    xi_io_timeouts_remove( &task->timeout, context->self->context_data->io_timeouts );
    assert( NULL == task->timeout.ptr_to_position );

    XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, arg2, XI_STATE_TIMEOUT );

    XI_CR_EXIT( task->cs, xi_mqtt_logic_layer_finalize_task( context, task ) );
}

static inline xi_state_t
do_mqtt_connect( void* ctx /* should be the context of the logic layer */
                 ,
                 void* data,
                 xi_state_t state,
                 void* msg_data )
{
    xi_layer_connectivity_t* context = ( xi_layer_connectivity_t* )ctx;
    xi_mqtt_logic_task_t* task       = ( xi_mqtt_logic_task_t* )data;

    xi_mqtt_logic_layer_data_t* layer_data =
        ( xi_mqtt_logic_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    xi_mqtt_message_t* msg_memory        = ( xi_mqtt_message_t* )msg_data;
    xi_evtd_instance_t* event_dispatcher = XI_CONTEXT_DATA( context )->evtd_instance;

    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) || layer_data == 0 )
    {
        xi_mqtt_message_free( &msg_memory );

        cancel_task_timeout( task, context );
        XI_CR_EXIT( task->cs, XI_STATE_OK );
    }

    XI_CR_START( task->cs );

    XI_ALLOC_AT( xi_mqtt_message_t, msg_memory, state );

    XI_CHECK_STATE( state = fill_with_connect_data(
                        msg_memory, XI_CONTEXT_DATA( context )->connection_data->username,
                        XI_CONTEXT_DATA( context )->connection_data->password,
                        XI_CONTEXT_DATA( context )->connection_data->keepalive_timeout,
                        XI_CONTEXT_DATA( context )->connection_data->session_type,
                        XI_CONTEXT_DATA( context )->connection_data->will_topic,
                        XI_CONTEXT_DATA( context )->connection_data->will_message,
                        XI_CONTEXT_DATA( context )->connection_data->will_qos,
                        XI_CONTEXT_DATA( context )->connection_data->will_retain ) );

    if ( XI_CONTEXT_DATA( context )->connection_data->connection_timeout > 0 )
    {
        state = xi_io_timeouts_create(
            xi_globals.evtd_instance,
            xi_make_handle( &do_mqtt_connect_timeout, context, task ),
            XI_CONTEXT_DATA( context )->connection_data->connection_timeout,
            context->self->context_data->io_timeouts, &task->timeout );

        XI_CHECK_STATE( state );
    }
    else
    {
        assert( NULL == task->timeout.ptr_to_position );
    }

    /* book up sending and wait till it's sent */
    XI_CR_YIELD( task->cs,
                 XI_PROCESS_PUSH_ON_PREV_LAYER( context, msg_memory, XI_STATE_OK ) );

    if ( state == XI_STATE_WRITTEN )
    {
        xi_debug_logger( "connect message has been sent" );
    }
    else
    {
        /* inform the next layer about a state change */
        XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, data, state );

        goto err_handling;
    }

    /* wait till the answer */
    XI_CR_YIELD( task->cs, XI_STATE_OK );

    /* parse the response */
    if ( msg_memory->common.common_u.common_bits.type == XI_MQTT_TYPE_CONNACK )
    {
        /* cancel io timeout */
        if ( NULL != task->timeout.ptr_to_position )
        {
            xi_io_timeouts_cancel( xi_globals.evtd_instance, &task->timeout,
                                   XI_CONTEXT_DATA( context )->io_timeouts );
            assert( NULL == task->timeout.ptr_to_position );
        }

        if ( msg_memory->connack.return_code == 0 )
        {
            xi_mqtt_message_free( &msg_memory );

            if ( XI_CONTEXT_DATA( context )->connection_data->keepalive_timeout > 0 )
            {
                state = xi_evtd_execute_in(
                    event_dispatcher, xi_make_handle( &do_mqtt_keepalive_once, context ),
                    XI_CONTEXT_DATA( context )->connection_data->keepalive_timeout,
                    &layer_data->keepalive_event );

                XI_CHECK_STATE( state );
            }

            /* now the layer is fully connected to the server */
            XI_CONTEXT_DATA( context )->connection_data->connection_state =
                XI_CONNECTION_STATE_OPENED;

            /* inform the next layer about a state change */
            XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, data, state );

            XI_CR_EXIT( task->cs, xi_mqtt_logic_layer_finalize_task( context, task ) );
        }
        else
        {
            xi_debug_format( "connack.return_code == %d",
                             msg_memory->connack.return_code );

            state = get_error_from_connack( msg_memory->connack.return_code );

            /* inform the next layer about a state change */
            XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, data, state );

            xi_mqtt_message_free( &msg_memory );

            /* cancel io timeout */
            if ( NULL != task->timeout.ptr_to_position )
            {
                xi_io_timeouts_cancel( xi_globals.evtd_instance, &task->timeout,
                                       XI_CONTEXT_DATA( context )->io_timeouts );
                assert( NULL == task->timeout.ptr_to_position );
            }

            XI_CR_EXIT( task->cs, xi_mqtt_logic_layer_finalize_task( context, task ) );
        }
    }

    xi_mqtt_message_free( &msg_memory );

    /* cancel io timeout */
    if ( NULL != task->timeout.ptr_to_position )
    {
        xi_io_timeouts_cancel( xi_globals.evtd_instance, &task->timeout,
                               XI_CONTEXT_DATA( context )->io_timeouts );
        assert( NULL == task->timeout.ptr_to_position );
    }

    XI_CR_EXIT( task->cs, xi_mqtt_logic_layer_finalize_task( context, task ) );

err_handling:
    xi_mqtt_message_free( &msg_memory );

    /* cancel io timeout */
    if ( NULL != task->timeout.ptr_to_position )
    {
        xi_io_timeouts_cancel( xi_globals.evtd_instance, &task->timeout,
                               XI_CONTEXT_DATA( context )->io_timeouts );
        assert( NULL == task->timeout.ptr_to_position );
    }

    XI_CR_EXIT( task->cs, xi_mqtt_logic_layer_finalize_task( context, task ) );

    XI_CR_END();
}

#ifdef __cplusplus
}
#endif

#endif /* __XI_MQTT_LOGIC_LAYER_CONNECT_COMMAND_H__ */
