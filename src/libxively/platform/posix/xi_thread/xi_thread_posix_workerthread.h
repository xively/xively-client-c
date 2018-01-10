/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_THREAD_POSIX_WORKERTHREAD_H__
#define __XI_THREAD_POSIX_WORKERTHREAD_H__

#include <pthread.h>
#include "xi_thread_workerthread.h"

typedef struct xi_workerthread_s
{
    xi_evtd_instance_t* thread_evtd;
    xi_evtd_instance_t* thread_evtd_secondary;

    pthread_t thread;
    uint8_t sync_start_flag;
} xi_workerthread_t;

#endif /* __XI_THREAD_POSIX_WORKERTHREAD_H__ */
