/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_THREAD_THREADPOOL_H__
#define __XI_THREAD_THREADPOOL_H__

#ifdef XI_MODULE_THREAD_ENABLED

#include <stddef.h>
#include <xi_event_dispatcher_api.h>
#include <xi_vector.h>

#define XI_THREADPOOL_MAXNUMOFTHREADS 10

/**
 * @brief threadpool, owns workerthreads, coordinates event executions
 */
typedef struct xi_threadpool_s
{
    xi_vector_t* workerthreads;
    xi_evtd_instance_t* threadpool_evtd;
} xi_threadpool_t;

/**
 * @brief creates a threadpool instance
 *
 * @param num_of_threads the desired number of threads, note: there is a maximum
 * number of
 * threads
 */
xi_threadpool_t* xi_threadpool_create_instance( uint8_t num_of_threads );

/**
 * @brief destroys a threadpool instance
 *
 * @param threadpool double pointer on the treadpool
 *
 * Destroys all worker threads in the pool, ensures all any-thread events are
 * executed.
 * These
 * latters most probably on the caller thread of this destroy function (current
 * thread).
 */
void xi_threadpool_destroy_instance( xi_threadpool_t** threadpool );

/**
 * @brief enqueues event for any-thread execution
 *
 * @param threadpool enqueue the event into this threadpool
 * @param handle this event handle will be enqueued
 */
xi_event_handle_queue_t*
xi_threadpool_execute( xi_threadpool_t* threadpool, xi_event_handle_t handle );

/**
 * @brief enqueues event for specific thread execution
 *
 * @param threadpool enqueue the event into one workerthread of this threadpool
 * @param handle this event handle will be enqueued
 */
xi_event_handle_queue_t* xi_threadpool_execute_on_thread( xi_threadpool_t* threadpool,
                                                          xi_event_handle_t handle,
                                                          uint8_t tid );

#else

struct xi_threadpool_t;

/**
 * @brief NULL threadpool generator for non-threaded library versions
 */
#define xi_threadpool_create_instance( ... ) NULL

/**
 * @brief NOOPERATION destructor for non-threaded library versions
 */
#define xi_threadpool_destroy_instance( ... )

#endif

#endif /* __XI_THREAD_THREADPOOL_H__ */
