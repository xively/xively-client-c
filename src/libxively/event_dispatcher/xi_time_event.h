/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

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
#include "xively_time.h"
#include "xi_event_handle.h"

typedef struct xi_time_event_handle_s
{
    xi_vector_index_type_t* ptr_to_position;
} xi_time_event_handle_t;

typedef struct xi_time_event_s
{
    xi_event_handle_t event_handle;
    xi_time_t time_of_execution;
    xi_vector_index_type_t position;
    xi_time_event_handle_t* time_event_handle;
} xi_time_event_t;

#define XI_TIME_EVENT_POSITION_INVALID -1

#define xi_make_empty_time_event_handle()                                                \
    {                                                                                    \
        NULL                                                                             \
    }

#define xi_make_time_event_handle( event_handle )                                        \
    {                                                                                    \
        &event_handle->position                                                          \
    }

#define xi_make_empty_time_event()                                                       \
    {                                                                                    \
        xi_make_empty_event_handle(), 0, XI_TIME_EVENT_POSITION_INVALID, NULL            \
    }


/* API */
/**
 * @brief xi_time_event_add
 *
 * Adds new time_event to the given vector. If the operation succeded it have to return
 * the xi_time_event_handle_t using the ret_time_event_handle return parameter. The
 * xi_time_event_handle_t is associated with the time_event and it can be used in order to
 * cancel or restart the time_event via calling xi_time_event_cancel or
 * xi_time_event_restart.
 *
 * @see xi_time_event_restart xi_time_event_cancel
 *
 * @note The user of this API has the ownership of the memory created outside of this API,
 * so if a time_event has been allocated on the heap, it has to be deallocated after it is
 * no longer used.
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
 *
 * Returns the pointer to the first element in the container which is guaranteed by the
 * time event implementation to be the time event with minimum execution time of all time
 * events stored within this container.
 *
 * It removes the returned time event element from the vector. Use xi_time_event_pee_top
 * in order to minitor for the value of the minimum element without removing it from the
 * container.
 *
 * @param vector
 * @return pointer to the time event with minimum execution time, NULL if the time event
 * container is empty
 */
xi_time_event_t* xi_time_event_get_top( xi_vector_t* vector );

/**
 * @brief xi_time_event_peek_top
 *
 * Returns the pointer to the first element in the container which is guaranteed by the
 * time event implementation to be the time event with minimum execution time of all time
 * events stored within this container.
 *
 * @param vector
 * @return pointer to the time event with minimum execution time, NULL if the time event
 * container is empty
 */
xi_time_event_t* xi_time_event_peek_top( xi_vector_t* vector );

/**
 * @brief xi_time_event_restart
 *
 * Changes the execution time of a time event associated with the gven time_event_handle.
 *
 * @param vector
 * @param time_event_handle
 * @return XI_STATE_OK in case of the success, XI_ELEMENT_NOT_FOUND if the time event does
 * not exist in the container
 */
xi_state_t xi_time_event_restart( xi_vector_t* vector,
                                  xi_time_event_handle_t* time_event_handle,
                                  xi_time_t new_time );

/**
 * @brief xi_time_event_cancel
 *
 * Cancels execution of the time event associated by the time_event_handle. It removes the
 * time event from the time events container.
 *
 * @param vector
 * @param time_event_handle
 * @return XI_STATE_OK in case of the success, XI_ELEMENT_NOT_FOUND if the time event
 * couldn't be found
 */
xi_state_t xi_time_event_cancel( xi_vector_t* vector,
                                 xi_time_event_handle_t* time_event_handle,
                                 xi_time_event_t** cancelled_time_event );

/**
 * @brief xi_time_event_destroy
 *
 * Releases all the memory allocated by time events.
 *
 * @param vector
 */
void xi_time_event_destroy( xi_vector_t* vector );

#endif /* __XI_TIME_EVENT_H__ */
