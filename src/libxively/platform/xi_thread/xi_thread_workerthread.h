/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_THREAD_WORKERTHREAD_H__
#define __XI_THREAD_WORKERTHREAD_H__

#include <xi_event_dispatcher_api.h>

struct xi_workerthread_s;

/**
 * @brief creates a workerthread instance
 *
 * @param evtd_secondary secondary event source. Workerthread does not own this evtd
 * neither ensures all event processing upon destruction. Workerthread simply
 * consumes a SINGLE event of this secondary evtd in each loop.
 */
struct xi_workerthread_s*
xi_workerthread_create_instance( xi_evtd_instance_t* evtd_secondary );

/**
 * @brief destroys workerthread instance
 *
 * At return all events are executed contained by the primary evtd.
 */
void xi_workerthread_destroy_instance( struct xi_workerthread_s** workerthread );

/**
 * @brief waits for syncpoint: workerthread started to run
 *
 * At return the workerthread is actually running. After calling this function one
 * can be certain that enqueuing events to evtd of the workerthread will be processed.
 *
 * @retval 1         sync point is reached, thread is running at return
 * @retval 0        timeout occurred, probably the thread has not started yet, see impl.
 * for timeout value (5sec)
 */
uint8_t xi_workerthread_wait_sync_point( struct xi_workerthread_s* workerthread );


#endif /* __XI_THREAD_WORKERTHREAD_H__ */
