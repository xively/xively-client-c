/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_list.h"
#include "xi_layer_api.h"
#include "xi_mqtt_logic_layer_data.h"
#include "xi_mqtt_logic_layer_task_helpers.h"
#include "xi_event_thread_dispatcher.h"

#ifdef __cplusplus
extern "C" {
#endif

xi_state_t xi_mqtt_logic_layer_run_next_q0_task( void* data )
{
    xi_layer_connectivity_t* context = data;

    xi_mqtt_logic_layer_data_t* layer_data =
        ( xi_mqtt_logic_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    assert( layer_data != 0 );

    /* cancel the timeout of the tasks if it is still registered */
    if ( layer_data->current_q0_task != 0 )
    {
        if ( NULL != layer_data->current_q0_task->timeout.ptr_to_position )
        {
            cancel_task_timeout( layer_data->current_q0_task, context );
        }

        xi_mqtt_logic_free_task( &layer_data->current_q0_task );
    }

    xi_mqtt_logic_task_t* task = 0;

    if ( layer_data->q0_tasks_queue != 0 )
    {
        XI_LIST_POP( xi_mqtt_logic_task_t, layer_data->q0_tasks_queue, task );

        /* prevent execution of other tasks while connecting */
        if ( XI_CONTEXT_DATA( context )->connection_data->connection_state ==
             XI_CONNECTION_STATE_OPENING )
        {
            /* we only allow connect while client is disconnected */
            if ( task->data.mqtt_settings.scenario != XI_MQTT_CONNECT )
            {
                XI_LIST_PUSH_BACK( xi_mqtt_logic_task_t, layer_data->q0_tasks_queue,
                                   task );

                return XI_STATE_OK;
            }
        }

        assert( layer_data->current_q0_task == 0 ); /* sanity check */

        layer_data->current_q0_task = task;
        return xi_evtd_execute_handle( &task->logic );
    }

    return XI_STATE_OK;
}

void xi_mqtt_logic_task_defer_users_callback( void* context,
                                              xi_mqtt_logic_task_t* task,
                                              xi_state_t state )
{
    /* PRECONDITION */
    assert( context != NULL );
    assert( task != NULL );

    /* if callback is not disposed */
    if ( xi_handle_disposed( &task->callback ) == 0 )
    {
        /* prepare handle */
        xi_event_handle_t handle = task->callback;
        handle.handlers.h3.a3    = state;

        xi_evttd_execute( XI_CONTEXT_DATA( context )->evtd_instance, handle );
    }
}


xi_state_t xi_mqtt_logic_layer_finalize_task( xi_layer_connectivity_t* context,
                                              xi_mqtt_logic_task_t* task )
{
    /* PRECONDITION */
    assert( NULL != task );

    /* let's make sure that the task timeout has been finished / cancelled
     * for now just a assertion so that we can check if it has been correctly
     * implemented */
    assert( NULL == task->timeout.ptr_to_position );

    xi_mqtt_logic_layer_data_t* layer_data =
        ( xi_mqtt_logic_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    if ( task->data.mqtt_settings.qos == XI_MQTT_QOS_AT_MOST_ONCE )
    {
        return xi_mqtt_logic_layer_run_next_q0_task( context );
    }
    else /* I left it for better code readability */
    {
        /* detach the task from the qos 1 and 2 queue */
        XI_LIST_DROP( xi_mqtt_logic_task_t, layer_data->q12_tasks_queue, task );

        /* release task's memory */
        xi_mqtt_logic_free_task( &task );
    }

    return XI_STATE_OK;
}

#ifdef __cplusplus
}
#endif
