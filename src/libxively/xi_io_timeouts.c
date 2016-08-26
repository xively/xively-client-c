/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#include <inttypes.h>

#include "xi_io_timeouts.h"
#include "xi_heap.h"

/**
 * @brief xi_io_timeouts_create
 *
 * Starts io event execution, and stores it in the given vector
 * Vector binds to an io thread, so no locking is needed ( ? )
 */

extern xi_heap_element_t* xi_io_timeouts_create( xi_evtd_instance_t* event_dispatcher,
                                                 xi_event_handle_t handle,
                                                 xi_time_t time_diff,
                                                 xi_vector_t* io_timeouts )
{
    assert( event_dispatcher != NULL );
    assert( io_timeouts != NULL );

    xi_heap_element_t* ret = xi_evtd_execute_in( event_dispatcher, handle, time_diff );

    xi_vector_push( io_timeouts, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( ret ) ) );

    return ret;
}

/**
 * @brief xi_io_timeouts_cancel
 *
 * Cancels io event and removes event from the io event vector.
 */

extern xi_heap_element_t* xi_io_timeouts_cancel( xi_evtd_instance_t* event_dispatcher,
                                                 xi_heap_element_t* heap_element,
                                                 xi_vector_t* io_timeouts )
{
    assert( event_dispatcher != NULL );
    assert( io_timeouts != NULL );

    xi_io_timeouts_remove( heap_element, io_timeouts );
    xi_evtd_cancel( event_dispatcher, heap_element );

    return 0;
}

/**
 * @brief xi_io_timeouts_restart
 *
 * Restarts io events in the timeouts vector
 */

extern void xi_io_timeouts_restart( xi_evtd_instance_t* event_dispatcher,
                                    xi_time_t new_time,
                                    xi_vector_t* io_timeouts )
{
    assert( event_dispatcher != NULL );
    assert( io_timeouts != NULL );

    xi_vector_index_type_t index = 0;

    for ( index = 0; index < io_timeouts->elem_no; ++index )
    {
        xi_vector_elem_t timeout_element = io_timeouts->array[index];
        xi_heap_element_t* timeout_event = timeout_element.selector_t.ptr_value;
        xi_evtd_restart( event_dispatcher, timeout_event, new_time );
    }
}

/**
 * @brief xi_io_timeouts_compare_heap_elements
 *
 * Compare function for finding events in the io event vector
 */

static inline int8_t
xi_io_timeouts_compare_heap_elements( const union xi_vector_selector_u* e0,
                                      const union xi_vector_selector_u* e1 )
{
    return e0->ptr_value == e1->ptr_value ? 0 : 1;
}

/**
 * @brief xi_io_timeouts_remove
 *
 * Remove event from timeout vector
 */

extern xi_heap_element_t*
xi_io_timeouts_remove( xi_heap_element_t* heap_element, xi_vector_t* io_timeouts )
{
    assert( heap_element != NULL );
    assert( io_timeouts != NULL );

    xi_vector_index_type_t event_index = xi_vector_find(
        io_timeouts, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( heap_element ) ),
        xi_io_timeouts_compare_heap_elements );

    if ( event_index > -1 )
    {
        xi_vector_del( io_timeouts, event_index );
    }

    return NULL;
}
