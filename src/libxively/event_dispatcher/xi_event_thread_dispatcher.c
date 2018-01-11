/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifdef XI_MODULE_THREAD_ENABLED

#include "xi_event_thread_dispatcher.h"
#include "xi_event_dispatcher_api.h"
#include "xi_globals.h"
#include "xi_thread_ids.h"
#include "xi_thread_threadpool.h"

xi_event_handle_queue_t*
xi_evttd_execute( xi_evtd_instance_t* evtd, xi_event_handle_t handle )
{
    if ( XI_THREADID_MAINTHREAD == handle.target_tid ||
         xi_globals.main_threadpool == NULL )
    {
        return xi_evtd_execute( evtd, handle );
    }
    else if ( handle.target_tid == XI_THREADID_ANYTHREAD )
    {
        return xi_threadpool_execute( xi_globals.main_threadpool, handle );
    }
    else
    {
        return xi_threadpool_execute_on_thread( xi_globals.main_threadpool, handle,
                                                handle.target_tid );
    }

    return NULL;
}
#else
/*
 * Using strict compiler settings requires every translation unit to contain some code so
 * we'll add an empty function here.
 */
void xi_evttd_execute_dummy()
{
    return;
}
#endif
