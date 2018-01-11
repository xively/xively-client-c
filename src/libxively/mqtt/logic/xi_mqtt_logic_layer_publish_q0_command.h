/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_MQTT_LOGIC_LAYER_PUBLISH_Q0_COMMAND_H__
#define __XI_MQTT_LOGIC_LAYER_PUBLISH_Q0_COMMAND_H__

#include "xi_layer_api.h"
#include "xi_mqtt_logic_layer_data.h"
#include "xi_coroutine.h"
#include "xi_mqtt_message.h"
#include "xi_mqtt_logic_layer_data_helpers.h"
#include "xi_mqtt_logic_layer_task_helpers.h"
#include "xi_globals.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline xi_state_t
do_mqtt_publish_q0( void* ctx, /* should be the context of the logic layer */
                    void* data,
                    xi_state_t state,
                    void* msg_data )
{
    XI_UNUSED( state );
    XI_UNUSED( msg_data );

    /* PRECONDITIONS */
    assert( NULL == msg_data );

    xi_layer_connectivity_t* context = ( xi_layer_connectivity_t* )ctx;
    xi_mqtt_logic_task_t* task       = ( xi_mqtt_logic_task_t* )data;

    assert( NULL != task );
    assert( NULL != context );

    xi_mqtt_logic_layer_data_t* layer_data =
        ( xi_mqtt_logic_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    xi_mqtt_message_t* msg_memory = NULL;
    xi_state_t callback_state     = XI_STATE_OK;

    /* means that we are shutting down */
    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) || NULL == layer_data )
    {
        return XI_STATE_OK;
    }

    XI_CR_START( task->cs );

    xi_debug_logger( "publish preparing message..." );

    XI_ALLOC_AT( xi_mqtt_message_t, msg_memory, state );

    XI_CHECK_STATE( state = fill_with_publish_data(
                        msg_memory, task->data.data_u->publish.topic,
                        task->data.data_u->publish.data, XI_MQTT_QOS_AT_MOST_ONCE,
                        task->data.data_u->publish.retain, XI_MQTT_DUP_FALSE, 0 ) );

    xi_debug_logger( "publish sending message..." );

    /* wait till it is sent */
    XI_CR_YIELD( task->cs,
                 XI_PROCESS_PUSH_ON_PREV_LAYER( context, msg_memory, XI_STATE_OK ) );

    callback_state = state;

    if ( XI_STATE_WRITTEN == state )
    {
        callback_state = XI_STATE_OK;
        xi_debug_logger( "publish message has been sent..." );
    }
    else
    {
        xi_debug_logger( "publish message has not been sent..." );
    }

    xi_mqtt_logic_free_task_data( task );

    xi_mqtt_logic_task_defer_users_callback( context, task, callback_state );

    XI_CR_EXIT( task->cs, xi_mqtt_logic_layer_finalize_task( context, task ) );

err_handling:
    xi_mqtt_logic_task_defer_users_callback( context, task, state );

    xi_mqtt_logic_free_task_data( task );

    XI_CR_EXIT( task->cs, xi_mqtt_logic_layer_finalize_task( context, task ) );

    XI_CR_END();
}

#ifdef __cplusplus
}
#endif

#endif /* __XI_MQTT_LOGIC_LAYER_PUBLISH_Q0_COMMAND_H__ */
