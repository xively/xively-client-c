/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_timed_task.h"
#include "xi_handle.h"

#define XI_MAX_TIMED_EVENT 64

typedef enum { XI_TTS_SCHEDULED, XI_TTS_RUNNING, XI_TTS_DELETABLE } xi_timed_task_state_e;

typedef struct xi_timed_task_data_s
{
    xi_user_task_callback_t* callback;
    xi_context_handle_t context_handle;
    void* data;
    xi_time_event_handle_t delayed_event;
    xi_evtd_instance_t* dispatcher;
    xi_time_t seconds_repeat;
    xi_timed_task_state_e state;
} xi_timed_task_data_t;

xi_state_t xi_timed_task_callback_wrapper( void* void_task, void* void_scheduler );

xi_timed_task_container_t* xi_make_timed_task_container()
{
    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_timed_task_container_t, container, state );

    container->timed_tasks_vector = xi_vector_create();
    XI_CHECK_STATE( state = xi_init_critical_section( &container->cs ) );

    return container;

err_handling:
    XI_SAFE_FREE( container );
    return NULL;
}

void xi_destroy_timed_task_container( xi_timed_task_container_t* container )
{
    assert( NULL != container );
    xi_vector_destroy( container->timed_tasks_vector );
    xi_destroy_critical_section( &container->cs );
    XI_SAFE_FREE( container );
}

xi_timed_task_handle_t xi_add_timed_task( xi_timed_task_container_t* container,
                                          xi_evtd_instance_t* dispatcher,
                                          xi_context_handle_t context_handle,
                                          xi_user_task_callback_t* callback,
                                          xi_time_t seconds_from_now,
                                          const uint8_t repeats_forever,
                                          void* data )
{
    assert( NULL != container );
    assert( NULL != dispatcher );
    assert( XI_INVALID_CONTEXT_HANDLE < context_handle );
    assert( NULL != callback );

    xi_state_t state                   = XI_STATE_OK;
    xi_timed_task_handle_t task_handle = XI_INVALID_CONTEXT_HANDLE;

    XI_ALLOC( xi_timed_task_data_t, task, state );

    task->context_handle = context_handle;
    task->callback       = callback;
    task->data           = data;
    task->dispatcher     = dispatcher;
    task->seconds_repeat = ( repeats_forever ) ? seconds_from_now : 0;
    task->state          = XI_TTS_SCHEDULED;

    xi_lock_critical_section( container->cs );
    XI_CHECK_STATE( state = xi_register_handle_for_object( container->timed_tasks_vector,
                                                           XI_MAX_TIMED_EVENT, task ) );

    XI_CHECK_STATE( state = xi_find_handle_for_object( container->timed_tasks_vector,
                                                       task, &task_handle ) );
    xi_unlock_critical_section( container->cs );

    state = xi_evtd_execute_in( dispatcher,
                                xi_make_handle( &xi_timed_task_callback_wrapper,
                                                ( void* )task, ( void* )container ),
                                seconds_from_now, &task->delayed_event );

    XI_CHECK_STATE( state );

    return task_handle;

err_handling:
    XI_SAFE_FREE( task );
    return -state;
}

void xi_remove_timed_task( xi_timed_task_container_t* container,
                           xi_timed_task_handle_t timed_task_handle )
{
    assert( NULL != container );

    xi_state_t state = XI_STATE_OK;

    xi_lock_critical_section( container->cs );

    xi_timed_task_data_t* task = ( xi_timed_task_data_t* )xi_object_for_handle(
        container->timed_tasks_vector, timed_task_handle );

    if ( NULL != task )
    {
        if ( XI_TTS_SCHEDULED == task->state )
        {
            state = xi_evtd_cancel( task->dispatcher, &task->delayed_event );
            assert( XI_STATE_OK == state );

            state = xi_delete_handle_for_object( container->timed_tasks_vector, task );
            XI_UNUSED( state );

            /* POST-CONDITION */
            assert( XI_STATE_OK == state );

            XI_SAFE_FREE( task );
        }
        else if ( XI_TTS_RUNNING == task->state )
        {
            task->state = XI_TTS_DELETABLE;
        }
    }

    xi_unlock_critical_section( container->cs );
}

xi_state_t xi_timed_task_callback_wrapper( void* void_task, void* void_scheduler )
{
    xi_timed_task_data_t* task = ( xi_timed_task_data_t* )void_task;
    assert( NULL != task );
    xi_timed_task_container_t* container = ( xi_timed_task_container_t* )void_scheduler;
    assert( NULL != container );

    xi_state_t state                   = XI_STATE_OK;
    xi_timed_task_handle_t task_handle = -1;

    xi_lock_critical_section( container->cs );
    state =
        xi_find_handle_for_object( container->timed_tasks_vector, task, &task_handle );
    if ( XI_STATE_OK == state )
    {
        task->state = XI_TTS_RUNNING;
    }
    xi_unlock_critical_section( container->cs );

    if ( XI_STATE_OK == state )
    {
        assert( NULL != task->callback );
        assert( task->context_handle > XI_INVALID_CONTEXT_HANDLE );

        ( task->callback )( task->context_handle, task_handle, task->data );

        xi_lock_critical_section( container->cs );

        if ( 0 == task->seconds_repeat || XI_TTS_DELETABLE == task->state )
        {
            xi_state_t del_state =
                xi_delete_handle_for_object( container->timed_tasks_vector, task );

            XI_UNUSED( del_state );

            /* POST-CONDITION */
            assert( XI_STATE_OK == del_state );

            XI_SAFE_FREE( task );
        }
        else
        {
            state = xi_evtd_execute_in(
                task->dispatcher, xi_make_handle( &xi_timed_task_callback_wrapper,
                                                  ( void* )task, ( void* )container ),
                task->seconds_repeat, &task->delayed_event );
            assert( XI_STATE_OK == state );
            task->state = XI_TTS_SCHEDULED;
        }

        xi_unlock_critical_section( container->cs );
    }

    return state;
}
