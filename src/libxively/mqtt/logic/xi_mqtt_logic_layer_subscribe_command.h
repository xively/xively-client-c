/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_MQTT_LOGIC_LAYER_SUBSCRIBE_COMMAND_H__
#define __XI_MQTT_LOGIC_LAYER_SUBSCRIBE_COMMAND_H__

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
do_mqtt_subscribe( void* ctx, void* data, xi_state_t state, void* msg )
{
    xi_layer_connectivity_t* context = ( xi_layer_connectivity_t* )ctx;
    xi_mqtt_logic_task_t* task       = ( xi_mqtt_logic_task_t* )data;

    assert( NULL != context );
    assert( NULL != task );

    xi_mqtt_message_t* msg_memory        = ( xi_mqtt_message_t* )msg;
    xi_evtd_instance_t* event_dispatcher = XI_CONTEXT_DATA( ctx )->evtd_instance;
    xi_state_t local_state               = XI_STATE_OK;

    xi_mqtt_logic_layer_data_t* layer_data =
        ( xi_mqtt_logic_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    XI_UNUSED( state );

    if ( NULL == layer_data )
    {
        cancel_task_timeout( task, context );
        xi_mqtt_message_free( &msg_memory );
        return XI_STATE_OK;
    }

    XI_CR_START( task->cs );

    do
    {
        xi_debug_format( "[m.id[%d]]subscribe preparing message", task->msg_id );

        XI_ALLOC_AT( xi_mqtt_message_t, msg_memory, state );

        XI_CHECK_STATE(
            state = fill_with_subscribe_data(
                msg_memory, task->data.data_u->subscribe.topic, task->msg_id,
                task->data.data_u->subscribe.qos,
                XI_STATE_RESEND == state ? XI_MQTT_DUP_TRUE : XI_MQTT_DUP_FALSE ) );

        xi_debug_format( "[m.id[%d]]subscribe sending message", task->msg_id );

        XI_CR_YIELD( task->cs,
                     XI_PROCESS_PUSH_ON_PREV_LAYER( context, msg_memory, XI_STATE_OK ) );

        if ( XI_STATE_WRITTEN == state )
        {
            xi_debug_format( "[m.id[%d]]subscribe has been sent", task->msg_id );
            assert( NULL == task->timeout.ptr_to_position );
            task->session_state = task->session_state == XI_MQTT_LOGIC_TASK_SESSION_UNSET
                                      ? XI_MQTT_LOGIC_TASK_SESSION_STORE
                                      : task->session_state;
        }
        else
        {
            xi_debug_format( "[m.id[%d]]subscribe has not been sent", task->msg_id );

            assert( NULL == task->timeout.ptr_to_position );

            local_state = xi_evtd_execute_in(
                event_dispatcher, xi_make_handle( &do_mqtt_subscribe, context, task,
                                                  XI_STATE_RESEND, NULL ),
                1, &task->timeout );

            XI_CHECK_STATE( local_state );

            XI_CR_YIELD( task->cs, XI_STATE_OK );

            /* sanity checks */
            assert( NULL == task->timeout.ptr_to_position );
            assert( XI_STATE_RESEND == state );

            continue;
        }

        /* add a timeout for waiting for the response */
        assert( NULL == task->timeout.ptr_to_position );

        /* @TODO change it to use the defined timeout */
        if ( XI_CONTEXT_DATA( context )->connection_data->keepalive_timeout > 0 )
        {
            xi_state_t local_state = xi_evtd_execute_in(
                event_dispatcher, xi_make_handle( &do_mqtt_subscribe, context, task,
                                                  XI_STATE_TIMEOUT, NULL ),
                XI_CONTEXT_DATA( context )->connection_data->keepalive_timeout,
                &task->timeout );
            XI_CHECK_STATE( local_state );
        }

        /* wait for the suback */
        XI_CR_YIELD( task->cs, XI_STATE_OK );

        /* clear timeout if it was timeout */
        if ( XI_STATE_TIMEOUT == state )
        {
            xi_debug_format( "[m.id[%d]]subscribe timeout occured", task->msg_id );
            assert( NULL == task->timeout.ptr_to_position );
            state = XI_STATE_RESEND;
        }
        else
        {
            cancel_task_timeout( task, context );
        }

        if ( XI_STATE_RESEND == state )
        {
            xi_debug_format( "[m.id[%d]]subscribe resend", task->msg_id );
        }

        assert( NULL == task->timeout.ptr_to_position );

    } while ( XI_STATE_RESEND == state );

    assert( NULL == task->timeout.ptr_to_position );

    if ( state == XI_STATE_OK )
    {
        if ( msg_memory->common.common_u.common_bits.type != XI_MQTT_TYPE_SUBACK )
        {
            xi_debug_format( "[m.id[%d]]subscribe error was expecting suback got %d!",
                             task->msg_id, msg_memory->common.common_u.common_bits.type );

            state = XI_MQTT_LOGIC_WRONG_MESSAGE_RECEIVED;
            goto err_handling;
        }

        xi_debug_format( "[m.id[%d]]subscribe suback received", task->msg_id );

        xi_mqtt_suback_status_t suback_status =
            msg_memory->suback.topics->xi_mqtt_topic_pair_payload_u.status;

        xi_mqtt_message_free( &msg_memory );

        task->data.data_u->subscribe.handler.handlers.h6.a2 =
            ( void* )( intptr_t )suback_status;

        task->data.data_u->subscribe.handler.handlers.h6.a3 =
            suback_status == XI_MQTT_SUBACK_FAILED ? XI_MQTT_SUBSCRIPTION_FAILED
                                                   : XI_MQTT_SUBSCRIPTION_SUCCESSFULL;

        /* check if the suback registration was successfull */
        if ( XI_MQTT_SUBACK_FAILED != suback_status )
        {
            /* now it can be registered - we are passing the ownership of the data.data_u
             * to the vector */
            XI_CHECK_MEMORY( xi_vector_push( layer_data->handlers_for_topics,
                                             XI_VEC_VALUE_PARAM( XI_VEC_VALUE_PTR(
                                                 task->data.data_u ) ) ),
                             state );
        }

        XI_CHECK_MEMORY(
            xi_evtd_execute( event_dispatcher, task->data.data_u->subscribe.handler ),
            state );

        /* now it's safe to nullify this pointer because the ownership of this memory
         * block is now passed either to a subscription callback or the
         * handlers_for_topics vector if the subscription was succesfull */
        task->data.data_u = NULL;

        XI_CR_EXIT( task->cs, xi_mqtt_logic_layer_finalize_task( context, task ) );
    }

    XI_CR_END();

err_handling:
    xi_mqtt_message_free( &msg_memory );
    xi_mqtt_logic_layer_finalize_task( context, task );

    XI_CR_RESET( task->cs );

    return state;
}

#ifdef __cplusplus
}
#endif

#endif /* __XI_MQTT_LOGIC_LAYER_SUBSCRIBE_COMMAND_H__ */
