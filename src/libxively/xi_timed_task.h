/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_TIMED_TASK_H__
#define __XI_TIMED_TASK_H__

#include "xi_types.h"
#include "xi_vector.h"
#include "xi_macros.h"
#include "xi_critical_section.h"

typedef struct xi_timed_task_container_s
{
    struct xi_critical_section_s* cs;
    xi_vector_t* timed_tasks_vector;
} xi_timed_task_container_t;

xi_timed_task_container_t* xi_make_timed_task_container();

void xi_destroy_timed_task_container( xi_timed_task_container_t* container );

xi_timed_task_handle_t xi_add_timed_task( xi_timed_task_container_t* container,
                                          xi_evtd_instance_t* dispatcher,
                                          xi_context_handle_t context_handle,
                                          xi_user_task_callback_t* callback,
                                          xi_time_t seconds_from_now,
                                          const uint8_t repeats_forever,
                                          void* data );

void xi_remove_timed_task( xi_timed_task_container_t* container,
                           xi_timed_task_handle_t timed_task_handle );

#endif /* __XI_TIMED_TASK_H__ */
