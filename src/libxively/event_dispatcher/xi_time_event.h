/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

/**
 * @file xi_time_event.h
 * @brief Implements time based events
 *
 * Time based events can be used to register function handler for later execution.
 * Entities such timeouts can be easily implemented using time based events.
 */

#ifndef __XI_TIME_EVENT_H__
#define __XI_TIME_EVENT_H__

#include <stdint.h>

#include "xi_vector.h"
#include "xi_debug.h"
#include "xi_allocator.h"
#include "xi_time.h"
#include "xi_handle.h"

typedef struct xi_time_event_handle_s
{
    xi_vector_index_type_t* position;
}xi_time_event_handle_t;

/* API */
xi_state_t
xi_time_event_execute_handle_in( xi_vector_t* vector,
                                 xi_event_handle_t event_handle,
                                 xi_time_t time,
                                 xi_time_event_handle_t* ret_time_event_handle );

xi_state_t
xi_time_event_restart( xi_vector_t* vector, xi_time_event_handle_t* time_event_handle );

xi_state_t
xi_time_event_cancel( xi_vector_t* vector, xi_time_event_handle_t* time_event_handle );

#endif /* __XI_TIME_EVENT_H__ */
