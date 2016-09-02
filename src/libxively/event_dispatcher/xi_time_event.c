/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#include "xi_time_event.h"

/**
 * note: the indexes are increased and decreased
 * in order to maintain the 0 - based indexing of elements
 */
#define LEFT( i ) ( ( ( i + 1 ) << 1 ) - 1 )
#define RIGHT( i ) ( ( ( ( i + 1 ) << 1 ) + 1 ) - 1 )
#define PARENT( i ) ( ( ( i + 1 ) >> 1 ) - 1 )


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
    xi_time_event_t* arg         = e1->ptr_value;

    if ( arg->time_of_execution < from_vector->time_of_execution )
    {
        return 0;
    }

    return -1;
}

/**
 * @brief xi_fun_update_position
 *
 * Local helper function that updates the position value of given time event.
 *
 * @param e0 - time event from the vector
 * @param arg - extra argument passed through the vector for each
 */
static void xi_fun_update_position( union xi_vector_selector_u* e0, void* arg )
{
    xi_vector_index_type_t* index = ( xi_vector_index_type_t* )arg;
    xi_time_event_t* time_event   = ( xi_time_event_t* )e0->ptr_value;
    time_event->position          = *index;
    *index += 1;
}


static void xi_vector_heap_fix_order_up( xi_vector_t* vector, xi_vector_index_type_t index )
{
    while ( index != 0 )
    {
        xi_vector_index_type_t parent_index = PARENT(index);

        xi_time_event_t* e = vector->array[index].selector_t.ptr_value;
        xi_time_event_t* p = vector->array[parent_index].selector_t.ptr_value;
        da
                sdf



        if( e->time_of_execution < p->time_of_execution )
        {
            xi_vector_swap_elems( vector, index, parent_index );

            /* update internal positions */
            e->position = parent_index;
            p->position = index;
        }

        index = PARENT( index );
    }
}

static const xi_vector_elem_t*
xi_vector_heap_element_add_event_handle( xi_vector_t* vector,
                                         xi_time_t time,
                                         xi_event_handle_t event_handle )
{
    xi_state_t out_state = XI_STATE_OK;

    XI_ALLOC( xi_time_event_t, time_event_to_cmp, out_state );

    time_event_to_cmp->event_handle      = event_handle;
    time_event_to_cmp->time_of_execution = time;

    /* stage first let's add it to the end of the heap */
    const xi_vector_elem_t* elem = xi_vector_push(
        vector,
        XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( ( void* )time_event_to_cmp ) ) );

    XI_CHECK_MEMORY( elem, out_state );

    /* 2nd stage now let's fix the heap from bottom to the top */
    xi_vector_heap_fix_order_up( vector, vector->elem_no - 1 );

    return vector[]

err_handling:
    XI_SAFE_FREE( time_event_to_cmp );
    return NULL;
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

    const xi_vector_index_type_t result = xi_vector_find(
        vector,
        XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( ( void* )time_event_to_cmp ) ),
        &first_not_greater_than );

    /* if it's -1 it means the correct place for new element is at the end */
    xi_vector_index_type_t index = result == -1 ? vector->elem_no : result;

    /* call the insert at function it will place the new element at the correct place */
    const xi_vector_elem_t* elem = xi_vector_insert_at(
        vector,
        XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( ( void* )time_event_to_cmp ) ),
        index );

    /* if there is a problem with the memory go to err_handling */
    XI_CHECK_MEMORY( elem, out_state );

    /* fix information about the position in all elements after index including index
     * element */
    xi_vector_for_each( vector, xi_fun_update_position, &index, result );

    /* update the return value with the pointer to the position in the vector */
    ret_time_event_handle->position = &time_event_to_cmp->position;

    /* exit with out_state value */
    return out_state;

err_handling:
    XI_SAFE_FREE( time_event_to_cmp );
    return out_state;
}
