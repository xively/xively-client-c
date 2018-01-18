/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_MQTT_LOGIC_LAYER_PUBLISH_HANDLER_H__
#define __XI_MQTT_LOGIC_LAYER_PUBLISH_HANDLER_H__

#include "xi_layer_api.h"
#include "xi_mqtt_logic_layer_data.h"
#include "xi_coroutine.h"
#include "xi_event_thread_dispatcher.h"
#include "xi_mqtt_message.h"
#include "xi_mqtt_logic_layer_data_helpers.h"
#include "xi_mqtt_logic_layer_task_helpers.h"
#include "xi_globals.h"
#include "xi_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline void
call_topic_handler( void* context, /* should be the context of the logic layer */
                    void* msg_data )
{
    xi_mqtt_logic_layer_data_t* layer_data =
        ( xi_mqtt_logic_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    xi_mqtt_message_t* msg_memory = ( xi_mqtt_message_t* )msg_data;

    // pre-conditions
    assert( NULL != msg_memory );

    xi_vector_index_type_t index = 0;

    xi_debug_format( "[m.id[%d]] looking for publish message handler",
                     xi_mqtt_get_message_id( msg_memory ) );

    index = xi_vector_find(
        layer_data->handlers_for_topics,
        XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( msg_memory->publish.topic_name ) ),
        cmp_topics );

    if ( index != -1 )
    {
        xi_mqtt_task_specific_data_t* subscribe_data =
            ( xi_mqtt_task_specific_data_t* )layer_data->handlers_for_topics->array[index]
                .selector_t.ptr_value;

        subscribe_data->subscribe.handler.handlers.h3.a2 = msg_memory;
        subscribe_data->subscribe.handler.handlers.h3.a3 = XI_STATE_OK;

        xi_evttd_execute( XI_CONTEXT_DATA( context )->evtd_instance,
                          subscribe_data->subscribe.handler );
    }
    else
    {
        xi_debug_format( "[m.id[%d]] received publish message for topic which "
                         "is not registered",
                         xi_mqtt_get_message_id( msg_memory ) );
        xi_debug_mqtt_message_dump( msg_memory );
        xi_mqtt_message_free( &msg_memory );
    }
}

static inline xi_state_t on_publish_q0_recieved(
    xi_layer_connectivity_t* context /* should be the context of the logic layer */
    ,
    void* data,
    xi_state_t state,
    void* msg_data )
{
    XI_UNUSED( data );
    XI_UNUSED( state );

    xi_mqtt_logic_layer_data_t* layer_data =
        ( xi_mqtt_logic_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    xi_mqtt_message_t* msg = ( xi_mqtt_message_t* )msg_data;

    if ( NULL == layer_data )
    {
        /* means that the layer has been shutted down */
        xi_mqtt_message_free( &msg );

        return XI_STATE_OK;
    }

    call_topic_handler( context, msg );

    return XI_STATE_OK;
}

static inline xi_state_t
on_publish_q1_recieved( void* context /* should be the context of the logic layer */
                        ,
                        void* data,
                        xi_state_t state,
                        void* msg_data )
{
    XI_UNUSED( data );
    XI_UNUSED( state );

    xi_mqtt_logic_layer_data_t* layer_data =
        ( xi_mqtt_logic_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    xi_mqtt_message_t* msg     = ( xi_mqtt_message_t* )msg_data;
    xi_mqtt_logic_task_t* task = ( xi_mqtt_logic_task_t* )data;

    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) || NULL == layer_data )
    {
        goto err_handling;
    }

    /* if task doesn't exist create and register one */
    if ( NULL == task )
    {
        uint16_t msg_id = msg->publish.message_id;

        XI_ALLOC_AT( xi_mqtt_logic_task_t, task, state );

        task->data.mqtt_settings.scenario = XI_MQTT_PUBACK;
        task->data.mqtt_settings.qos      = XI_MQTT_QOS_AT_LEAST_ONCE;

        task->logic =
            xi_make_handle( &on_publish_q1_recieved, context, task, XI_STATE_OK, msg );

        task->msg_id = msg_id;

/* @TODO change this into debug only compilation */
#if 1
        {
            xi_mqtt_logic_task_t* test_task = NULL;

            XI_LIST_FIND( xi_mqtt_logic_task_t, layer_data->q12_recv_tasks_queue,
                          CMP_TASK_MSG_ID, task->msg_id, test_task );

            // there must not be similar tasks
            assert( NULL == test_task );
        }
#endif

        XI_LIST_PUSH_BACK( xi_mqtt_logic_task_t, layer_data->q12_recv_tasks_queue, task );
    }

    XI_CR_START( task->cs );

    /* PRECONDITIONS */
    assert( NULL != msg );

    do
    {
        /* send puback to the server */
        XI_ALLOC_AT( xi_mqtt_message_t, msg, state );

        XI_CHECK_STATE( state = fill_with_puback_data( msg, task->msg_id ) );

        xi_debug_format( "[m.id[%d]preparing puback data", task->msg_id );

        XI_CR_YIELD_UNTIL( task->cs, ( state != XI_STATE_WRITTEN ),
                           XI_PROCESS_PUSH_ON_PREV_LAYER( context, msg, XI_STATE_OK ) );

    } while ( state != XI_STATE_WRITTEN );

    xi_debug_format( "[m.id[%d]]PUBACK sent", task->msg_id );

    /* msg sent now proceed with cleaning */

    /* clean the created task data */
    XI_LIST_DROP( xi_mqtt_logic_task_t, layer_data->q12_recv_tasks_queue, task );

    xi_mqtt_logic_free_task( &task );

    /* use the recv publish function */
    call_topic_handler( context, msg );

    XI_CR_END();

    return XI_STATE_OK;

err_handling:
    xi_mqtt_message_free( &msg );

    if ( task )
    {
        XI_CR_RESET( task->cs );
    }

    return state;
}

static inline xi_state_t on_publish_recieved(
    xi_layer_connectivity_t* context, /* should be the context of the logic layer */
    xi_mqtt_message_t* msg_memory,
    xi_state_t state )
{
    xi_mqtt_logic_layer_data_t* layer_data =
        ( xi_mqtt_logic_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    if ( layer_data == 0 )
    {
        xi_mqtt_message_free( &msg_memory );
        return XI_STATE_OK;
    }

    switch ( msg_memory->common.common_u.common_bits.qos )
    {
        case XI_MQTT_QOS_AT_MOST_ONCE:
            return on_publish_q0_recieved( context, 0, state, msg_memory );
        case XI_MQTT_QOS_AT_LEAST_ONCE:
            return on_publish_q1_recieved( context, 0, state, msg_memory );
        case XI_MQTT_QOS_EXACTLY_ONCE:
            xi_debug_logger( "recv_publish for XI_MQTT_QOS_EXACTLY_ONCE not "
                             "yet implemented!" );
            break;
    }

    return XI_STATE_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* __XI_MQTT_LOGIC_LAYER_PUBLISH_HANDLER_H__ */
