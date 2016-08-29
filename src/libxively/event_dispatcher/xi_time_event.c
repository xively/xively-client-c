/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#include "xi_time_event.h"

typedef struct xi_time_event_s
{
    xi_event_handle_t event_handle;
    xi_time_t time_of_execution;
    xi_vector_index_type_t position;
} xi_time_event_t;

/**
 * @brief first_not_greater_than
 *
 * Function used in order to find the element in the vector that is first not greater than
 * the element we compare with.
 *
 * @param e0
 * @param e1
 * @return
 */
static int8_t first_not_greater_than( const union xi_vector_selector_u* e0,
                                      const union xi_vector_selector_u* e1 )
{
    xi_time_event_t* from_vector = e0->ptr_value;
    xi_time_event_t* arg = e1->ptr_value;

    if( arg->time_of_execution < from_vector->time_of_execution )
    {
        return 0;
    }

    return -1;
}

xi_state_t
xi_time_event_execute_handle_in( xi_vector_t* vector,
                                 xi_event_handle_t event_handle,
                                 xi_time_t time,
                                 xi_time_event_handle_t* ret_time_event_handle )
{
    xi_state_t out_state = XI_STATE_OK;

    XI_ALLOC( xi_time_event_t, time_event_to_cmp, out_state );

    time_event_to_cmp->event_handle      = event_handle;
    time_event_to_cmp->time_of_execution = time;

    int8_t result = xi_vector_find( vector, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR(
                                                ( void* )time_event_to_cmp ) ),
                                    &first_not_greater_than );

    /* there is no such element */
    if ( result != 0 )
    {
        xi_vector_push( vector, XI_VEC_CONST_VALUE_PARAM(
                                    XI_VEC_VALUE_PTR( ( void* )time_event_to_cmp ) ) );
        /* because the element has been added at the end */
        time_event_to_cmp->position = vector->elem_no - 1;
    }

    // xi_vector_insert_at( );

    return out_state;

err_handling:
    XI_SAFE_FREE( time_event_to_cmp );
    return out_state;
}
