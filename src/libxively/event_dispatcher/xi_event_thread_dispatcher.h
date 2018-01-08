/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_EVENT_THREAD_DISPATCHER_H__
#define __XI_EVENT_THREAD_DISPATCHER_H__

#include "xi_event_dispatcher_api.h"

#ifdef XI_MODULE_THREAD_ENABLED

#include "xi_event_handle_queue.h"

/**
 * @brief dispatches event to proper evtd for execution on targeted thread
 *
 * Create a threaded event, enqueue event through this function to
 * ensure execution on proper thread.
 */
extern xi_event_handle_queue_t*
xi_evttd_execute( xi_evtd_instance_t* evtd, xi_event_handle_t handle );

#else

/**
 * @brief for non-threaded library version: enqueue event to global evtd
 *          for main thread execution
 */
#define xi_evttd_execute( evtd, handle ) xi_evtd_execute( evtd, handle )

#endif

#endif /* __XI_EVENT_THREAD_DISPATCHER_H__ */
