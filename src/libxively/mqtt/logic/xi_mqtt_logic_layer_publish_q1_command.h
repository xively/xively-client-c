/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_MQTT_LOGIC_LAYER_PUBLISH_Q1_COMMAND_H__
#define __XI_MQTT_LOGIC_LAYER_PUBLISH_Q1_COMMAND_H__

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

static xi_state_t
do_mqtt_publish_q1( void* ctx /* should be the context of the logic layer */
                    ,
                    void* data,
                    xi_state_t state,
                    void* msg_data )
{
    xi_layer_connectivity_t* context = ( xi_layer_connectivity_t* )ctx;
    xi_mqtt_logic_task_t* task       = ( xi_mqtt_logic_task_t* )data;

    /* pre-conditions */
    assert( NULL != context );
    assert( NULL != task );

    xi_mqtt_logic_layer_data_t* layer_data =
        ( xi_mqtt_logic_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    xi_evtd_instance_t* event_dispatcher = XI_CONTEXT_DATA( context )->evtd_instance;
    xi_mqtt_message_t* msg_memory        = ( xi_mqtt_message_t* )msg_data;

    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) || NULL == layer_data )
    {
        cancel_task_timeout( task, context );

        xi_mqtt_message_free( &msg_memory );

        XI_CR_RESET( task->cs );
        return XI_STATE_OK;
    }

    XI_CR_START( task->cs );

    assert( NULL == task->timeout.ptr_to_position );

    do
    {
        xi_debug_format( "[m.id[%d]]publish q1 preparing message", task->msg_id );

        XI_ALLOC_AT( xi_mqtt_message_t, msg_memory, state );

        /* note on memory - here the data ptr's are shared, so no data copy */
        XI_CHECK_STATE(
            state = fill_with_publish_data(
                msg_memory, task->data.data_u->publish.topic,
                task->data.data_u->publish.data, XI_MQTT_QOS_AT_LEAST_ONCE,
                task->data.data_u->publish.retain,
                state == XI_STATE_RESEND ? XI_MQTT_DUP_TRUE : XI_MQTT_DUP_FALSE,
                task->msg_id ) );

        xi_debug_format( "[m.id[%d]]publish q1 sending message", task->msg_id );

        XI_CR_YIELD( task->cs,
                     XI_PROCESS_PUSH_ON_PREV_LAYER( context, msg_memory, XI_STATE_OK ) );

        if ( state == XI_STATE_WRITTEN )
        {
            xi_debug_format( "[m.id[%d]]publish q1 has been sent", task->msg_id );
            task->session_state = task->session_state == XI_MQTT_LOGIC_TASK_SESSION_UNSET
                                      ? XI_MQTT_LOGIC_TASK_SESSION_STORE
                                      : task->session_state;
        }
        else
        {
            xi_debug_format( "[m.id[%d]]publish q1 has not been sent", task->msg_id );
            state = XI_STATE_RESEND;
            continue;
        }

        /* add a timeout for waiting for the response */
        assert( NULL == task->timeout.ptr_to_position );

        /* @TODO change it to use the defined timeout */
        if ( XI_CONTEXT_DATA( context )->connection_data->keepalive_timeout > 0 )
        {
            state = xi_evtd_execute_in(
                event_dispatcher, xi_make_handle( &do_mqtt_publish_q1, context, task,
                                                  XI_STATE_TIMEOUT, NULL ),
                XI_CONTEXT_DATA( context )->connection_data->keepalive_timeout,
                &task->timeout );
            XI_CHECK_STATE( state );
        }

        /* wait for the puback */
        XI_CR_YIELD( task->cs, XI_STATE_OK );

        if ( XI_STATE_TIMEOUT == state )
        {
            xi_debug_format( "[m.id[%d]]publish q1 timeout occured", task->msg_id );

            /* clear timeout if it was timeout */
            assert( NULL == task->timeout.ptr_to_position );

            /* let's change the actual state to resend as the coroutine has to resend the
             * message */
            state = XI_STATE_RESEND;
        }
        else
        {
            cancel_task_timeout( task, context );
        }

        if ( state == XI_STATE_RESEND )
        {
            xi_debug_format( "[m.id[%d]]publish q1 resend", task->msg_id );
        }

        /* post-loop condition */
        assert( NULL == task->timeout.ptr_to_position );

    } while ( XI_STATE_RESEND == state );
    /* try to send the message until timeout occurs */

    assert( NULL == task->timeout.ptr_to_position );

    if ( XI_STATE_OK != state )
    {
        xi_debug_format( "[m.id[%d]]publish q1 error while waiting for PUBACK",
                         task->msg_id );
        goto err_handling;
    }

    assert( NULL != msg_memory );

    if ( msg_memory->common.common_u.common_bits.type != XI_MQTT_TYPE_PUBACK )
    {
        xi_debug_format( "[m.id[%d]]publish q1 error was expecting puback got %d!",
                         task->msg_id, msg_memory->common.common_u.common_bits.type );

        state = XI_MQTT_LOGIC_WRONG_MESSAGE_RECEIVED;
        goto err_handling;
    }

    xi_debug_format( "[m.id[%d]]publish q1 publish puback received", task->msg_id );

    xi_mqtt_logic_task_defer_users_callback( context, task, state );

    xi_mqtt_message_free( &msg_memory );

    xi_mqtt_logic_free_task_data( task );

    XI_CR_EXIT( task->cs, xi_mqtt_logic_layer_finalize_task( context, task ) );

    XI_CR_END();

err_handling:
    xi_mqtt_logic_task_defer_users_callback( context, task, state );

    xi_mqtt_message_free( &msg_memory );

    if ( task->data.data_u )
    {
        xi_mqtt_logic_free_task_data( task );
    }

    XI_CR_RESET( task->cs );

    xi_mqtt_logic_layer_finalize_task( context, task );

    return state;
}

#ifdef __cplusplus
}
#endif

#endif /* __XI_MQTT_LOGIC_LAYER_PUBLISH_Q1_COMMAND_H__ */
