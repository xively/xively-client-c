/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_coroutine.h"
#include "xi_layer_api.h"
#include "xi_globals.h"
#include "xi_mqtt_logic_layer.h"
#include "xi_mqtt_logic_layer_data.h"
#include "xi_mqtt_logic_layer_keepalive_handler.h"
#include "xi_mqtt_logic_layer_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

xi_state_t do_mqtt_keepalive_once( void* data )
{
    xi_layer_connectivity_t* context = data;
    xi_state_t state                 = XI_STATE_OK;

    xi_mqtt_logic_layer_data_t* layer_data =
        ( xi_mqtt_logic_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    if ( layer_data == 0 )
    {
        return XI_STATE_OK;
    }

    layer_data->keepalive_event.ptr_to_position = NULL;

    XI_ALLOC( xi_mqtt_logic_task_t, task, state );

    task->data.mqtt_settings.scenario = XI_MQTT_KEEPALIVE;
    task->data.mqtt_settings.qos      = XI_MQTT_QOS_AT_MOST_ONCE;

    xi_debug_logger( "do_mqtt_keepalive_once" );

    return xi_mqtt_logic_layer_push( context, task, XI_STATE_OK );

err_handling:
    XI_SAFE_FREE( task );
    return state;
}

xi_state_t
do_mqtt_keepalive_task( void* ctx, void* data, xi_state_t state, void* msg_data )
{
    xi_layer_connectivity_t* context     = ctx;
    xi_mqtt_logic_task_t* task           = data;
    xi_mqtt_message_t* msg_memory        = msg_data;
    xi_evtd_instance_t* event_dispatcher = XI_CONTEXT_DATA( context )->evtd_instance;

    xi_mqtt_logic_layer_data_t* layer_data =
        ( xi_mqtt_logic_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    if ( layer_data == 0 )
    {
        xi_mqtt_message_free( &msg_memory );
        return XI_STATE_OK;
    }

    assert( task != 0 );

    XI_CR_START( task->cs );

    XI_ALLOC_AT( xi_mqtt_message_t, msg_memory, state );

    XI_CHECK_STATE( state = fill_with_pingreq_data( msg_memory ) );

    xi_debug_logger( "message memory filled with pingreq data" );

    XI_CR_YIELD( task->cs,
                 XI_PROCESS_PUSH_ON_PREV_LAYER( context, msg_memory, XI_STATE_OK ) );

    if ( state == XI_STATE_WRITTEN )
    {
        xi_debug_logger( "pingreq message sent... waiting for response" );
    }
    else
    {
        xi_debug_format( "pingreq message has not been sent... %d", ( int )state );
    }

    // wait for an interval of keepalive
    {
        assert( NULL == task->timeout.ptr_to_position );

        state = xi_evtd_execute_in(
            event_dispatcher, xi_make_handle( &on_keepalive_timeout_expiry, context, task,
                                              state, msg_memory ),
            XI_CONTEXT_DATA( context )->connection_data->keepalive_timeout,
            &task->timeout );

        XI_CHECK_STATE( state );

        /* for a message */
        XI_CR_YIELD( task->cs, XI_STATE_OK );
    }

    if ( state == XI_STATE_TIMEOUT )
    {
        xi_debug_logger( "keepalive timeout passed!" );
        assert( NULL == task->timeout.ptr_to_position );
        XI_CR_EXIT( task->cs, do_reconnect( context, 0, XI_STATE_TIMEOUT ) );
    }
    else if ( state != XI_STATE_OK )
    {
        xi_debug_logger( "error while waiting for pingresp!" );
        cancel_task_timeout( task, context );
        XI_CR_EXIT( task->cs, xi_mqtt_logic_layer_finalize_task( context, task ) );
    }

    cancel_task_timeout( task, context );

    if ( msg_memory->common.common_u.common_bits.type == XI_MQTT_TYPE_PINGRESP )
    {
        xi_debug_logger( "PINGRESP received..." );
    }
    else
    {
        xi_debug_format( "PINGRESP expected got: %d",
                         msg_memory->common.common_u.common_bits.type );
        state = XI_MQTT_LOGIC_WRONG_MESSAGE_RECEIVED;
        goto err_handling;
    }

    /* only if it is connected */
    if ( XI_CONTEXT_DATA( context )->connection_data->connection_state ==
         XI_CONNECTION_STATE_OPENED )
    {
        state = xi_evtd_execute_in(
            event_dispatcher, xi_make_handle( &do_mqtt_keepalive_once, context ),
            XI_CONTEXT_DATA( context )->connection_data->keepalive_timeout,
            &layer_data->keepalive_event );
        XI_CHECK_STATE( state );
    }

    xi_mqtt_message_free( &msg_memory );

    XI_CR_EXIT( task->cs, xi_mqtt_logic_layer_finalize_task( context, task ) );

    XI_CR_END();

err_handling:
    xi_mqtt_message_free( &msg_memory );
    XI_CR_RESET( task->cs );
    xi_mqtt_logic_layer_finalize_task( context, task );

    return state;
}

#ifdef __cplusplus
}
#endif
