/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <unistd.h>
#include <errno.h>

#include <xi_thread_posix_workerthread.h>

#define XI_THREAD_WORKERTHREAD_RESTTIME_IN_NANOSECONDS 10000000;         // 1/100 sec
#define XI_THREAD_WORKERTHREAD_WAITFORSYNCTIME_IN_NANOSECONDS 100000000; // 1/10 sec

void* xi_workerthread_start_routine( void* ctx )
{
    xi_state_t state = XI_STATE_OK;

    XI_CHECK_CND_DBGMESSAGE( ctx == NULL, XI_INVALID_PARAMETER, state,
                             "no context is provided for workerthrad function" );

    xi_workerthread_t* corresponding_workerthread = ( xi_workerthread_t* )ctx;

    xi_debug_format( "[%p] sync point reached", pthread_self() );
    corresponding_workerthread->sync_start_flag = 1;
    xi_debug_format( "[%p] sync point passed", pthread_self() );

    struct timespec deltatime;
    deltatime.tv_sec  = 0;
    deltatime.tv_nsec = XI_THREAD_WORKERTHREAD_RESTTIME_IN_NANOSECONDS;

    /* simple event loop impl, just executes all handlers in event dispatcher
     * in each 1/100 sec */
    while ( xi_evtd_dispatcher_continue( corresponding_workerthread->thread_evtd ) )
    {
        /* consume all handles of evtd */
        xi_evtd_step( corresponding_workerthread->thread_evtd, time( 0 ) );
        /* consume a single handle of secondary evtd */
        if ( xi_evtd_dispatcher_continue(
                 corresponding_workerthread->thread_evtd_secondary ) )
        {
            xi_evtd_single_step( corresponding_workerthread->thread_evtd_secondary,
                                 time( 0 ) );
        }
        /* let thread rest a little */
        nanosleep( &deltatime, NULL );
    }

    /* ensuring execution of handlers added right before turning of event dispatcher */
    xi_evtd_step( corresponding_workerthread->thread_evtd, time( 0 ) );

err_handling:
    return NULL;
}

xi_workerthread_t* xi_workerthread_create_instance( xi_evtd_instance_t* evtd_secondary )
{
    xi_state_t state = XI_STATE_OK;
    XI_ALLOC( xi_workerthread_t, new_workerthread_instance, state );

    new_workerthread_instance->thread_evtd = xi_evtd_create_instance();

    XI_CHECK_CND_DBGMESSAGE(
        new_workerthread_instance->thread_evtd == NULL, XI_OUT_OF_MEMORY, state,
        "could not create event dispatcher for new xi_workerthread instance" );

    new_workerthread_instance->thread_evtd_secondary = evtd_secondary;

    const int ret_pthread_create =
        pthread_create( &new_workerthread_instance->thread, NULL,
                        xi_workerthread_start_routine, new_workerthread_instance );

    if ( ret_pthread_create != 0 )
    {
        xi_debug_format( "creation of pthread instance failed with error: %d",
                         ret_pthread_create );
        goto err_handling;
    }

    return new_workerthread_instance;

err_handling:
    XI_SAFE_FREE( new_workerthread_instance );

    return NULL;
}

void xi_workerthread_destroy_instance( xi_workerthread_t** workerthread )
{
    if ( workerthread == NULL || *workerthread == NULL )
        return;

    if ( ( *workerthread )->thread_evtd != NULL )
    {
        xi_evtd_stop( ( *workerthread )->thread_evtd );
    }

    const int ret_pthread_join = pthread_join( ( *workerthread )->thread, NULL );
    XI_UNUSED( ret_pthread_join );

    if ( ( *workerthread )->thread_evtd != NULL )
    {
        xi_evtd_destroy_instance( ( *workerthread )->thread_evtd );
    }

    XI_SAFE_FREE( *workerthread );
}

uint8_t xi_workerthread_wait_sync_point( xi_workerthread_t* workerthread )
{
    xi_debug_format( "[%p] waiting for sync start of thread [%p]...", pthread_self(),
                     workerthread->thread );

    struct timespec deltatime;
    deltatime.tv_sec  = 0;
    deltatime.tv_nsec = XI_THREAD_WORKERTHREAD_WAITFORSYNCTIME_IN_NANOSECONDS;

    /* note: pthread_mutex_timedlock_np is not available on osx
     * note: pthread_barrier_t is optional for POSIX pthreads, not available on osx */
    size_t i = 0;
    for ( ; i < 50; ++i )
    {
        if ( workerthread->sync_start_flag != 0 )
            break;
        nanosleep( &deltatime, NULL );
    }

    xi_debug_format( "[%p] sync start %s for thread [%p]", pthread_self(),
                     workerthread->sync_start_flag ? "OK" : "FAILED",
                     workerthread->thread );

    return ( workerthread->sync_start_flag != 0 ) ? 1 : 0;
}
