/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_MQTT_LOGIC_LAYER_CONNECTION_HELPERS_H__
#define __XI_MQTT_LOGIC_LAYER_CONNECTION_HELPERS_H__

#include "xi_layer_api.h"
#include "xi_mqtt_logic_layer_data.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline xi_state_t
do_reconnect( void* context, void* data, xi_state_t in_out_state )
{
    return XI_PROCESS_CLOSE_ON_PREV_LAYER( context, data, in_out_state );
}

static inline xi_state_t
do_shutdown( void* ctx /* should be the context of the logic layer */
             ,
             void* data,
             xi_state_t state,
             void* msg_data )
{
    xi_layer_connectivity_t* context = ( xi_layer_connectivity_t* )ctx;
    xi_mqtt_logic_task_t* task       = ( xi_mqtt_logic_task_t* )data;
    xi_mqtt_message_t* msg_memory    = ( xi_mqtt_message_t* )msg_data;

    xi_mqtt_logic_layer_data_t* layer_data =
        ( xi_mqtt_logic_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    if ( layer_data == 0 )
    {
        /* means that the layer has been shutted down */
        xi_mqtt_message_free( &msg_memory );
        return XI_STATE_OK;
    }

    XI_UNUSED( state );

    assert( task != 0 );

    XI_CR_START( task->cs );

    XI_ALLOC_AT( xi_mqtt_message_t, msg_memory, state );

    XI_CHECK_STATE( state = fill_with_disconnect_data( msg_memory ) );

    xi_debug_logger( "message memory filled with disconnect data" );

    XI_CR_YIELD( task->cs,
                 XI_PROCESS_PUSH_ON_PREV_LAYER( context, msg_memory, XI_STATE_OK ) );

    if ( state == XI_STATE_WRITTEN )
    {
        xi_debug_logger( "disconnect message has been sent "
                         "continue with shutting down" );
    }
    else
    {
        xi_debug_logger( "disconnect message has not been "
                         "sent continue with shutting down" );
    }

    XI_SAFE_FREE( task->data.data_u );

    /* let's close */
    xi_mqtt_logic_layer_close( ctx, 0, XI_STATE_OK );

    XI_CR_EXIT( task->cs, xi_mqtt_logic_layer_finalize_task( context, task ) );

    XI_CR_END();

err_handling:
    return state;
}

#ifdef __cplusplus
}
#endif

#endif /* __XI_MQTT_LOGIC_LAYER_CONNECTION_HELPERS_H__ */
