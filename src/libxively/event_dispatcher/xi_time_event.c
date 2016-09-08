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

static void xi_vector_heap_swap_time_events( xi_vector_t* vector,
                                             xi_vector_index_type_t fi,
                                             xi_vector_index_type_t li )
{
    /* PRE-CONDITIONS */
    assert( NULL != vector );
    assert( fi < vector->elem_no );
    assert( li < vector->elem_no );

    xi_time_event_t* fte = ( xi_time_event_t* )vector->array[fi].selector_t.ptr_value;
    xi_time_event_t* lte = ( xi_time_event_t* )vector->array[li].selector_t.ptr_value;

    xi_vector_swap_elems( vector, fi, li );

    fte->position = li;
    lte->position = fi;
}

static xi_vector_index_type_t
xi_vector_heap_fix_order_up( xi_vector_t* vector, xi_vector_index_type_t index )
{
    /* PRE-CONDITIONS */
    assert( NULL != vector );
    assert( index < vector->elem_no );

    xi_vector_index_type_t ret_index = index;

    while ( index != 0 )
    {
        xi_vector_index_type_t parent_index = PARENT( index );

        xi_time_event_t* e = vector->array[index].selector_t.ptr_value;
        xi_time_event_t* p = vector->array[parent_index].selector_t.ptr_value;

        if ( e->time_of_execution < p->time_of_execution )
        {
            xi_vector_heap_swap_time_events( vector, index, parent_index );
            ret_index = parent_index;
        }
        else /* we don't need to go to the very top */
        {
            break;
        }

        index = parent_index;
    }

    return ret_index;
}

#define GET_LEFT_SIBLING_INDEX( index, last_elem_index )                                 \
    ( LEFT( ( index ) ) > ( last_elem_index ) ) ? ( ( index ) ) : LEFT( ( index ) )

#define GET_RIGHT_SIBLING_INDEX( index, last_elem_index )                                \
    ( RIGHT( ( index ) ) > ( last_elem_index ) ) ? ( ( index ) ) : RIGHT( ( index ) )

static void
xi_vector_heap_fix_order_down( xi_vector_t* vector, xi_vector_index_type_t index )
{
    /* PRE-CONDITIONS */
    assert( NULL != vector );
    assert( index < vector->elem_no );

    xi_vector_index_type_t left_sibling_index =
        GET_LEFT_SIBLING_INDEX( index, ( vector->elem_no - 1 ) );
    xi_vector_index_type_t right_sibling_index =
        GET_RIGHT_SIBLING_INDEX( index, ( vector->elem_no - 1 ) );

    do
    {
        xi_time_event_t* left_time_event =
            ( xi_time_event_t* )vector->array[left_sibling_index].selector_t.ptr_value;
        xi_time_event_t* right_time_event =
            ( xi_time_event_t* )vector->array[right_sibling_index].selector_t.ptr_value;
        xi_time_event_t* current_time_event =
            ( xi_time_event_t* )vector->array[index].selector_t.ptr_value;

        xi_vector_index_type_t index_to_cmp = index;
        xi_time_event_t* time_event_to_cmp  = NULL;

        /* pick up the sibling with lower key to cmp with */
        if ( left_time_event->time_of_execution < right_time_event->time_of_execution )
        {
            index_to_cmp      = left_sibling_index;
            time_event_to_cmp = left_time_event;
        }
        else
        {
            index_to_cmp      = right_sibling_index;
            time_event_to_cmp = right_time_event;
        }

        /* if the order is not correct fix it */
        if ( time_event_to_cmp->time_of_execution <
             current_time_event->time_of_execution )
        {
            xi_vector_heap_swap_time_events( vector, index, index_to_cmp );
            index = index_to_cmp;
        }
        else
        {
            return;
        }

        left_sibling_index  = GET_LEFT_SIBLING_INDEX( index, vector->elem_no - 1 );
        right_sibling_index = GET_RIGHT_SIBLING_INDEX( index, vector->elem_no - 1 );

    } while ( index != left_sibling_index && index != right_sibling_index );
}

static const xi_vector_elem_t*
xi_vector_heap_add_time_event( xi_vector_t* vector, xi_time_event_t* time_event )
{
    /* PRE-CONDITIONS */
    assert( NULL != vector );
    assert( NULL != time_event );

    xi_state_t out_state = XI_STATE_OK;

    time_event->position = vector->elem_no;

    /* stage first let's add it to the end of the heap */
    const xi_vector_elem_t* elem = xi_vector_push(
        vector, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( ( void* )time_event ) ) );

    XI_CHECK_MEMORY( elem, out_state );

    /* 2nd stage now let's fix the heap from bottom to the top */
    xi_vector_index_type_t element_position =
        xi_vector_heap_fix_order_up( vector, vector->elem_no - 1 );

    /* return the element of the vector that points to the new time event*/
    return &vector->array[element_position];

err_handling:
    return NULL;
}

xi_state_t xi_time_event_add( xi_vector_t* vector,
                              xi_time_event_t* time_event,
                              xi_time_event_handle_t* ret_time_event_handle )
{
    /* PRE-CONDITIONS */
    assert( NULL != vector );
    assert( NULL != time_event );
    assert( NULL != ret_time_event_handle );

    xi_state_t out_state = XI_STATE_OK;

    /* call the insert at function it will place the new element at the correct place */
    const xi_vector_elem_t* elem = xi_vector_heap_add_time_event( vector, time_event );

    /* if there is a problem with the memory go to err_handling */
    XI_CHECK_MEMORY( elem, out_state );

    /* extract the element */
    xi_time_event_t* added_time_event = ( xi_time_event_t* )elem->selector_t.ptr_value;

    /* sanity checks */
    assert( added_time_event == time_event );

    /* update the return value with the pointer to the position in the vector */
    ret_time_event_handle->position = &added_time_event->position;

    /* exit with out_state value */
    return out_state;

err_handling:
    return out_state;
}

xi_time_event_t* xi_time_event_get_top( xi_vector_t* vector )
{
    /* PRE-CONDITIONS */
    assert( NULL != vector );

    if ( 0 == vector->elem_no )
    {
        return NULL;
    }

    xi_time_event_t* top_one = ( xi_time_event_t* )vector->array[0].selector_t.ptr_value;

    /* the trick is to swap the first element with the last one and to proceed bottom
     * direction fix of heap datastructure */

    if ( vector->elem_no > 1 )
    {
        xi_vector_heap_swap_time_events( vector, 0, vector->elem_no - 1 );
        xi_vector_del( vector, vector->elem_no - 1 );
        xi_vector_heap_fix_order_down( vector, 0 );
    }
    else
    {
        xi_vector_del( vector, vector->elem_no - 1 );
    }

    return top_one;
}

const xi_time_event_t* xi_time_event_peek_top( xi_vector_t* vector )
{
    /* PRE-CONDITIONS */
    assert( NULL != vector );

    if ( 0 == vector->elem_no )
    {
        return NULL;
    }

    xi_time_event_t* top_one = ( xi_time_event_t* )vector->array[0].selector_t.ptr_value;

    return top_one;
}

xi_state_t xi_time_event_restart( xi_vector_t* vector,
                                  xi_time_event_handle_t* time_event_handle,
                                  xi_time_t new_time )
{
    /* PRE-CONDITIONS */
    assert( NULL != vector );
    assert( NULL != time_event_handle );

    /* the element can be found with O(1) complexity cause we've been updating each
     * element's position during every operation that could've break it */

    xi_vector_index_type_t index = *time_event_handle->position;

    /* check for the correctness of the time_event_handle */
    if ( index >= vector->elem_no || index < 0 )
    {
        return XI_ELEMENT_NOT_FOUND;
    }

    /* let's update the key of this element */
    xi_time_event_t* time_event =
        ( xi_time_event_t* )vector->array[index].selector_t.ptr_value;

    time_event->time_of_execution = new_time;

    /* now we have to restore the order in the vector */
    xi_vector_heap_fix_order_up( vector, index );
    xi_vector_heap_fix_order_down( vector, index );

    return XI_STATE_OK;
}

xi_state_t
xi_time_event_cancel( xi_vector_t* vector, xi_time_event_handle_t* time_event_handle )
{
    /* PRE-CONDITIONS */
    assert( NULL != vector );
    assert( NULL != time_event_handle );

    /* the element we would like to remove should be at position described by the
     * time_event_handle */

    xi_vector_index_type_t index = *time_event_handle->position;

    if ( index >= vector->elem_no || index < 0 )
    {
        return XI_ELEMENT_NOT_FOUND;
    }

    /* if it's somwhere else than the end, let's put it there */
    if ( index < vector->elem_no - 1 )
    {
    }

    return XI_STATE_OK;
}
