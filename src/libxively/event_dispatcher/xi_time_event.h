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
} xi_time_event_handle_t;

typedef struct xi_time_event_s
{
    xi_event_handle_t event_handle;
    xi_time_t time_of_execution;
    xi_vector_index_type_t position;
} xi_time_event_t;

#define xi_make_empty_time_event_handle()                                                \
    {                                                                                    \
        NULL                                                                             \
    }

#define xi_make_empty_time_event()                                                       \
    {                                                                                    \
        xi_make_empty_event_handle(), 0, 0                                               \
    }

/* API */
/**
 * @brief xi_time_event_add
 *
 * Adds new time_event to the given vector. If the operation succeded it have to return
 * the xi_time_event_handle_t using the ret_time_event_handle return parameter. The
 * xi_time_event_handle_t is associated with the time_event and it can be used in order to
 * cancel or restart the time_event.
 *
 * @note The user of this API has the ownership of the memory created outside of this API,
 * so if a time_event has been allocated on heap, it has to be deallocated after it is no
 * longer used.
 *
 * @param vector - the storage for time_events
 * @param time_event - new time event to get registered
 * @param ret_time_event_handle - return parameter, a handle associated with the
 * time_event
 * @return XI_STATE_OK in case of success other values in case of failure
 */
xi_state_t xi_time_event_add( xi_vector_t* vector,
                              xi_time_event_t* time_event,
                              xi_time_event_handle_t* ret_time_event_handle );

/**
 * @brief xi_time_event_get_top
 * @param vector
 * @return
 */
xi_time_event_t* xi_time_event_get_top( xi_vector_t* vector );

/**
 * @brief xi_time_event_peek_top
 * @param vector
 * @return
 */
const xi_time_event_t* xi_time_event_peek_top( xi_vector_t* vector );

/**
 * @brief xi_time_event_restart
 * @param vector
 * @param time_event_handle
 * @return
 */
xi_state_t xi_time_event_restart( xi_vector_t* vector,
                                  xi_time_event_handle_t* time_event_handle,
                                  xi_time_t new_time );

/**
 * @brief xi_time_event_cancel
 * @param vector
 * @param time_event_handle
 * @return
 */
xi_state_t xi_time_event_cancel( xi_vector_t* vector,
                                 xi_time_event_handle_t* time_event_handle,
                                 xi_time_event_t** cancelled_time_event );

#endif /* __XI_TIME_EVENT_H__ */
