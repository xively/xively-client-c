/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <inttypes.h>
#include "xi_io_timeouts.h"

/**
 * @brief xi_io_timeouts_create
 *
 * Starts io event execution, and stores it in the given vector
 * Vector binds to an io thread, so no locking is needed ( ? )
 */
xi_state_t xi_io_timeouts_create( xi_evtd_instance_t* event_dispatcher,
                                  xi_event_handle_t handle,
                                  xi_time_t time_diff,
                                  xi_vector_t* io_timeouts,
                                  xi_time_event_handle_t* ret_time_event_handle )
{
    assert( NULL != event_dispatcher );
    assert( NULL != io_timeouts );
    assert( NULL != ret_time_event_handle &&
            NULL == ret_time_event_handle->ptr_to_position );

    const xi_vector_elem_t* elem = NULL;

    xi_state_t ret_state =
        xi_evtd_execute_in( event_dispatcher, handle, time_diff, ret_time_event_handle );

    XI_CHECK_STATE( ret_state );

    elem = xi_vector_push( io_timeouts, XI_VEC_CONST_VALUE_PARAM(
                                            XI_VEC_VALUE_PTR( ret_time_event_handle ) ) );

    XI_CHECK_MEMORY( elem, ret_state );

err_handling:
    return ret_state;
}

/**
 * @brief xi_io_timeouts_cancel
 *
 * Cancels io event and removes event from the io event vector.
 */

void xi_io_timeouts_cancel( xi_evtd_instance_t* event_dispatcher,
                            xi_time_event_handle_t* time_event_handle,
                            xi_vector_t* io_timeouts )
{
    assert( event_dispatcher != NULL );
    assert( io_timeouts != NULL );
    assert( NULL != time_event_handle && NULL != time_event_handle->ptr_to_position );

    xi_io_timeouts_remove( time_event_handle, io_timeouts );
    const xi_state_t local_state = xi_evtd_cancel( event_dispatcher, time_event_handle );
    XI_UNUSED( local_state );
    assert( XI_STATE_OK == local_state );
}

/**
 * @brief xi_io_timeouts_restart
 *
 * Restarts io events in the timeouts vector
 */
void xi_io_timeouts_restart( xi_evtd_instance_t* event_dispatcher,
                             xi_time_t new_time,
                             xi_vector_t* io_timeouts )
{
    assert( event_dispatcher != NULL );
    assert( io_timeouts != NULL );

    xi_vector_index_type_t index = 0;

    for ( index = 0; index < io_timeouts->elem_no; ++index )
    {
        xi_vector_elem_t timeout_element          = io_timeouts->array[index];
        xi_time_event_handle_t* time_event_handle = timeout_element.selector_t.ptr_value;

        assert( NULL != time_event_handle && NULL != time_event_handle->ptr_to_position );

        const xi_state_t local_state =
            xi_evtd_restart( event_dispatcher, time_event_handle, new_time );
        XI_UNUSED( local_state );
        assert( XI_STATE_OK == local_state );
    }
}

/**
 * @brief xi_io_timeouts_compare_heap_elements
 *
 * Compare function for finding events in the io event vector
 */
static inline int8_t
xi_io_timeouts_compare_pointers( const union xi_vector_selector_u* e0,
                                 const union xi_vector_selector_u* e1 )
{
    return e0->ptr_value == e1->ptr_value ? 0 : 1;
}

/**
 * @brief xi_io_timeouts_remove
 *
 * Remove event from timeout vector
 */
void xi_io_timeouts_remove( xi_time_event_handle_t* time_event_handle,
                            xi_vector_t* io_timeouts )
{
    assert( time_event_handle != NULL );
    assert( io_timeouts != NULL );

    xi_vector_index_type_t event_index = xi_vector_find(
        io_timeouts, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( time_event_handle ) ),
        xi_io_timeouts_compare_pointers );

    if ( event_index > -1 )
    {
        xi_vector_del( io_timeouts, event_index );
    }
}
