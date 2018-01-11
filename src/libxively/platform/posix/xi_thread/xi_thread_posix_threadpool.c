/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_thread_threadpool.h"
#include <xi_thread_posix_workerthread.h>

xi_threadpool_t* xi_threadpool_create_instance( uint8_t num_of_threads )
{
    num_of_threads = XI_MIN( XI_MAX( num_of_threads, 1 ), XI_THREADPOOL_MAXNUMOFTHREADS );

    xi_state_t state = XI_STATE_OK;
    XI_ALLOC( xi_threadpool_t, threadpool, state );

    threadpool->threadpool_evtd = xi_evtd_create_instance();

    XI_CHECK_CND_DBGMESSAGE( threadpool->threadpool_evtd == NULL, XI_OUT_OF_MEMORY, state,
                             "could not create event dispatcher for threadpool" );

    threadpool->workerthreads = xi_vector_create();

    XI_CHECK_CND_DBGMESSAGE( threadpool->workerthreads == NULL, XI_OUT_OF_MEMORY, state,
                             "could not instantiate vector for workerthreads" );

    int8_t result = xi_vector_reserve( threadpool->workerthreads, num_of_threads );

    XI_CHECK_CND_DBGMESSAGE(
        result == 0, XI_OUT_OF_MEMORY, state,
        "could not reserve enough space in vector for workerthreads" );

    uint8_t counter_workerthread = 0;
    for ( ; counter_workerthread < num_of_threads; ++counter_workerthread )
    {
        xi_workerthread_t* new_workerthread =
            xi_workerthread_create_instance( threadpool->threadpool_evtd );

        XI_CHECK_CND_DBGMESSAGE( new_workerthread == NULL, XI_OUT_OF_MEMORY, state,
                                 "could not allocate a workerthread" );

        xi_vector_push(
            threadpool->workerthreads,
            XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( new_workerthread ) ) );
    }

    return threadpool;

err_handling:
    return NULL;
}

void xi_threadpool_destroy_instance( xi_threadpool_t** threadpool )
{
    if ( threadpool == NULL || *threadpool == NULL )
        return;

    xi_threadpool_t* threadpool_ptr = *threadpool;

    if ( threadpool_ptr->workerthreads != NULL )
    {
        /* stop all workerthreads in advance their destroy to avoid summing up join
         * times at destruction with that all thread exits are done parallelly */
        uint8_t counter_workerthread = 0;
        for ( ; counter_workerthread < threadpool_ptr->workerthreads->elem_no;
              ++counter_workerthread )
        {
            xi_evtd_stop( ( ( xi_workerthread_t* )threadpool_ptr->workerthreads
                                ->array[counter_workerthread]
                                .selector_t.ptr_value )
                              ->thread_evtd );
        }

        counter_workerthread = 0;
        for ( ; counter_workerthread < threadpool_ptr->workerthreads->elem_no;
              ++counter_workerthread )
        {
            xi_workerthread_destroy_instance(
                ( xi_workerthread_t** )&threadpool_ptr->workerthreads
                    ->array[counter_workerthread]
                    .selector_t.ptr_value );
        }

        if ( threadpool_ptr->threadpool_evtd != NULL )
        {
            /* ensure all any-thread handlers are executed before destroy */
            xi_evtd_step( threadpool_ptr->threadpool_evtd, time( 0 ) );
        }

        xi_vector_destroy( threadpool_ptr->workerthreads );
    }

    xi_evtd_destroy_instance( threadpool_ptr->threadpool_evtd );

    XI_SAFE_FREE( *threadpool );
}

xi_event_handle_queue_t*
xi_threadpool_execute( xi_threadpool_t* threadpool, xi_event_handle_t handle )
{
    if ( threadpool == NULL || threadpool->threadpool_evtd == NULL )
        return NULL;

    return xi_evtd_execute( threadpool->threadpool_evtd, handle );
}

xi_event_handle_queue_t* xi_threadpool_execute_on_thread( xi_threadpool_t* threadpool,
                                                          xi_event_handle_t handle,
                                                          uint8_t tid )
{
    if ( threadpool == NULL || threadpool->workerthreads == NULL ||
         threadpool->workerthreads->array == NULL )
        return NULL;

    if ( tid < threadpool->workerthreads->elem_no &&
         threadpool->workerthreads->array[tid].selector_t.ptr_value != NULL )
    { /* valid tid */
        xi_workerthread_t* workerthread_target =
            ( ( xi_workerthread_t* )threadpool->workerthreads->array[tid]
                  .selector_t.ptr_value );

        if ( workerthread_target->thread_evtd != NULL )
        {
            return xi_evtd_execute( workerthread_target->thread_evtd, handle );
        }
    }

    /* invalid tid, fallback: execute handler on any thread */
    return xi_threadpool_execute( threadpool, handle );
}
