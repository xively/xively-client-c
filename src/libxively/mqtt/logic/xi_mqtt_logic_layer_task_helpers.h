/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_MQTT_LOGIC_LAYER_TASK_HELPERS_H__
#define __XI_MQTT_LOGIC_LAYER_TASK_HELPERS_H__

#include "xi_event_dispatcher_api.h"
#include "xi_mqtt_logic_layer_data.h"
#include "xi_layer_api.h"
#include "xi_list.h"
#include "xi_globals.h"
#include "xi_io_timeouts.h"

#ifdef __cplusplus
extern "C" {
#endif

xi_state_t xi_mqtt_logic_layer_finalize_task( xi_layer_connectivity_t* context,
                                              xi_mqtt_logic_task_t* task );

xi_state_t xi_mqtt_logic_layer_run_next_q0_task( void* data );

void xi_mqtt_logic_task_defer_users_callback( void* context,
                                              xi_mqtt_logic_task_t* task,
                                              xi_state_t state );

#define CMP_TASK_MSG_ID( task, id ) ( task->msg_id == id )

static inline void
cancel_task_timeout( xi_mqtt_logic_task_t* task, xi_layer_connectivity_t* context )
{
    /* PRE-CONDITIONS */
    assert( task != NULL );
    assert( context != NULL );

    xi_evtd_instance_t* event_dispatcher = XI_CONTEXT_DATA( context )->evtd_instance;
    xi_vector_t* io_timeouts             = XI_CONTEXT_DATA( context )->io_timeouts;

    assert( event_dispatcher != NULL );
    assert( io_timeouts != NULL );

    if ( NULL != task->timeout.ptr_to_position )
    {
        /* check if timeout is in timeouts vector, and remove it */
        xi_io_timeouts_remove( &task->timeout, io_timeouts );
        xi_state_t local_state = xi_evtd_cancel( event_dispatcher, &task->timeout );

        if ( XI_STATE_OK != local_state )
        {
            //@TODO add proper implementation and error handling
            xi_debug_logger( "error while canceling task timeout" );
        }
        assert( XI_STATE_OK == local_state );
    }

    /* POST-CONDITIONS */
    assert( NULL == task->timeout.ptr_to_position );
}

static inline void signal_task( xi_mqtt_logic_task_t* task, xi_state_t state )
{
    task->logic.handlers.h4.a3 = state;
    xi_evtd_execute_handle( &task->logic );
}

/* if sent after layer_data cleared
 * will stop each task and free tasks memory */
static inline void abort_task( xi_mqtt_logic_task_t* task )
{
    /* call the task handler with ok status */
    signal_task( task, XI_STATE_OK );
}

static inline void resend_task( xi_mqtt_logic_task_t* task )
{
    signal_task( task, XI_STATE_RESEND );
}

static inline void timeout_task( xi_mqtt_logic_task_t* task )
{
    signal_task( task, XI_STATE_TIMEOUT );
}

static inline void
set_new_context_and_call_resend( xi_mqtt_logic_task_t* task, void* context )
{
    if ( task->session_state == XI_MQTT_LOGIC_TASK_SESSION_STORE )
    {
        task->logic.handlers.h4.a1 = context;
        resend_task( task );
    }
}

static inline xi_state_t
run_task( xi_layer_connectivity_t* context, xi_mqtt_logic_task_t* task )
{
    /* PRECONDITION */
    assert( context != 0 );
    assert( task != 0 );

    xi_mqtt_logic_layer_data_t* layer_data =
        ( xi_mqtt_logic_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    if ( task->data.mqtt_settings.qos == XI_MQTT_QOS_AT_MOST_ONCE ) /* qos zero tasks
                                                                     * must be enqued */
    {
        /* task on qos0 can be prioritized */
        switch ( task->priority )
        {
            case XI_MQTT_LOGIC_TASK_NORMAL:
                XI_LIST_PUSH_BACK( xi_mqtt_logic_task_t, layer_data->q0_tasks_queue,
                                   task );
                break;
            case XI_MQTT_LOGIC_TASK_IMMEDIATE:
                XI_LIST_PUSH_FRONT( xi_mqtt_logic_task_t, layer_data->q0_tasks_queue,
                                    task );
                break;
        }

        if ( layer_data->current_q0_task == 0 )
        {
            return xi_mqtt_logic_layer_run_next_q0_task( context );
        }
    }
    else
    {
        /* generate the new id this id will be used to communicate with the server
         * and to demultiplex msgs */
        task->msg_id = ++layer_data->last_msg_id;

#ifdef XI_DEBUG_EXTRA_INFO
        xi_mqtt_logic_task_t* needle = NULL;
        XI_LIST_FIND(
            xi_mqtt_logic_task_t, layer_data->q12_tasks_queue, CMP_TASK_MSG_ID,
            task->msg_id, /* this is linear search so we have O(n) complexity it can be
                             optimized but for the small n it is acceptable complexity */
            needle );
        assert( NULL == needle && "task with the same id already exist" );
#endif

        /* add it to the queue which is really a multiplexer of message id's */
        XI_LIST_PUSH_BACK( xi_mqtt_logic_task_t, layer_data->q12_tasks_queue, task );

        /* execute it immediately
         * @TODO concider a different strategy of execution in order to minimize the
         * device overload we could execute only a certain amount per one loop
         *  but let's keep it simple for now */
        if ( XI_CONTEXT_DATA( context )->connection_data->connection_state ==
             XI_CONNECTION_STATE_OPENED )
        {
            return xi_evtd_execute_handle( &task->logic );
        }
    }

    return XI_STATE_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* __XI_MQTT_LOGIC_LAYER_TASK_HELPERS_H__ */
