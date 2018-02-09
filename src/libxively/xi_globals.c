/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_globals.h"

xi_globals_t xi_globals = {
    .network_timeout        = 1500,
    .globals_ref_count      = 0,
    .evtd_instance          = NULL,
    .default_context_handle = XI_INVALID_CONTEXT_HANDLE,
    .context_handles_vector = NULL,
    .timed_tasks_container  = NULL,
    .main_threadpool        = NULL,
    .str_account_id         = NULL,
    .str_device_unique_id   = NULL,
    .backoff_status = {xi_make_empty_time_event_handle(), 0, 0, XI_BACKOFF_CLASS_NONE, 0},
    .context_handles_vector_edge_devices = NULL};
