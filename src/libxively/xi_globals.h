/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_GLOBALS_H__
#define __XI_GLOBALS_H__

#include <stdint.h>

#include "xi_types.h"
#include "xi_backoff_status.h"
#include "xi_timed_task.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "xi_event_dispatcher_api.h"

/* This struct is used for run-time config */
typedef struct
{
    uint32_t network_timeout;
    uint8_t globals_ref_count;
    xi_evtd_instance_t* evtd_instance;
    xi_context_handle_t default_context_handle;
    xi_vector_t* context_handles_vector;
    xi_timed_task_container_t* timed_tasks_container;
    struct xi_threadpool_s* main_threadpool;
    char* str_account_id;
    xi_backoff_status_t backoff_status;

    xi_vector_t* context_handles_vector_edge_devices;
} xi_globals_t;

extern xi_globals_t xi_globals;

#ifdef __cplusplus
}
#endif

#endif /* __XI_GLOBALS_H__ */
