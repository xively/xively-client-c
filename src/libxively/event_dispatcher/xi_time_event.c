/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#include "xi_time_event.h"

/**
 * @brief This part of the file implements the simplified heap order functionality. This
 * implementation assumes that the element type is always the xi_time_event_t. Which is
 * the only reason to use heap order within the vector.
 *
 * What is a heap order ?
 *
 * The heap order is a relation of key's in each node. If we take any node let's call it
 * x, these conditions must be true:
 * a) PARENT(x).key > x.key;
 * b) LEFT_CHILD(x).key < x.key;
 * c) RIGHT_CHILD(x).key < x.key;
 *
 * This gives us a nice quality of having the element with the lowest key at the top. So
 * extraction of the element with the lowest key has O(1) + fixing the order after
 * removing it O(logn) complexity.
 *
 * Peeking the element with the lowest key without removing it has O(1) complexity.
 *
 * All of the elements are being kept in the regular vector. This implementation uses the
 * vector as a container.
 *
 * The heap order based on vector uses index based addressing. So the index of childs and
 * parent of any node are calculated using this formulas:
 *
 * LEFT_CHILD(x)= ( x.i + 1 ) / 2 ) - 1
 * RIGHT_CHILD(x) = ( x.i + 1 ) / 2
 * PARENT(x) = ( ( x.i + 1 ) * 2 ) - 1
 *
 * Using that indexing technique we can traverse through the vector as if it was a binary
 * tree. Thanks to that all of the fix heap order functions works in O(logn) complexity.
 * Since we don't need to revisit every element in the vector.
 *
 * xi_time_event_handler_t special quality improves the complexity of functions such as
 * cancel or restart. It holds a pointer to the position element of the xi_time_event_t
 * structure. This way while the handler is valid it holds correct position of the time
 * event it is handling.
 */

/**
 * note: the indexes are increased and decreased
 * in order to maintain the 0 - based indexing of elements
 */
#undef LEFT
#undef RIGHT
#undef PARENT
#define LEFT( i ) ( ( ( ( i ) + 1 ) << 1 ) - 1 )
#define RIGHT( i ) ( ( ( i ) + 1 ) << 1 )
#define PARENT( i ) ( ( ( ( i ) + 1 ) >> 1 ) - 1 )

/**
 * @brief xi_vector_heap_swap_time_events
 *
 * Swaps two vector xi_time_event_t elements pointed by fi and li indexes with each other.
 * It updates element's positions value to the new ones.
 *
 * @param vector
 * @param fi
 * @param li
 */
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

/**
 * @brief xi_vector_heap_fix_order_up
 *
 * Function restores the heap order in the vector in the towards the root direction.
 *
 * @param vector
 * @param index
 * @return
 */
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

/**
 * @brief xi_vector_heap_fix_order_down
 *
 * Restores the order in the heap in the down direction, towards the leafs.
 *
 * @param vector
 * @param index
 */
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
        xi_time_event_t* left_child_time_event =
            ( xi_time_event_t* )vector->array[left_sibling_index].selector_t.ptr_value;
        xi_time_event_t* right_child_time_event =
            ( xi_time_event_t* )vector->array[right_sibling_index].selector_t.ptr_value;
        xi_time_event_t* current_time_event =
            ( xi_time_event_t* )vector->array[index].selector_t.ptr_value;

        xi_vector_index_type_t index_to_cmp = index;
        xi_time_event_t* time_event_to_cmp  = NULL;

        /* pick up the sibling with lower key to cmp with */
        if ( left_child_time_event->time_of_execution <
             right_child_time_event->time_of_execution )
        {
            index_to_cmp      = left_sibling_index;
            time_event_to_cmp = left_child_time_event;
        }
        else
        {
            index_to_cmp      = right_sibling_index;
            time_event_to_cmp = right_child_time_event;
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

    } while ( index != left_sibling_index || index != right_sibling_index );
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

static void xi_time_event_clean_time_event( xi_time_event_t* time_event )
{
    if ( NULL != time_event->time_event_handle )
    {
        time_event->time_event_handle->position = NULL;
    }
}

xi_state_t xi_time_event_add( xi_vector_t* vector,
                              xi_time_event_t* time_event,
                              xi_time_event_handle_t* ret_time_event_handle )
{
    /* PRE-CONDITIONS */
    assert( NULL != vector );
    assert( NULL != time_event );
    assert(
        ( NULL != ret_time_event_handle && NULL == ret_time_event_handle->position ) ||
        ( NULL == ret_time_event_handle ) );

    xi_state_t out_state = XI_STATE_OK;

    /* call the insert at function it will place the new element at the correct place */
    const xi_vector_elem_t* elem = xi_vector_heap_add_time_event( vector, time_event );

    /* if there is a problem with the memory go to err_handling */
    XI_CHECK_MEMORY( elem, out_state );

    /* extract the element */
    xi_time_event_t* added_time_event = ( xi_time_event_t* )elem->selector_t.ptr_value;

    /* sanity checks */
    assert( added_time_event == time_event );

    if ( NULL != ret_time_event_handle )
    {
        /* update the return value with the pointer to the position in the vector */
        ret_time_event_handle->position = &added_time_event->position;

        /* set the time event handle pointer for further sanity checks and cleaning */
        added_time_event->time_event_handle = ret_time_event_handle;
    }

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

    xi_time_event_clean_time_event( top_one );

    return top_one;
}

xi_time_event_t* xi_time_event_peek_top( xi_vector_t* vector )
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
     * element's position during every operation that could've broken it */

    xi_vector_index_type_t index = *time_event_handle->position;

    /* check for the correctness of the time_event_handle */
    if ( index >= vector->elem_no || index < 0 )
    {
        return XI_ELEMENT_NOT_FOUND;
    }

    /* let's update the key of this element */
    xi_time_event_t* time_event =
        ( xi_time_event_t* )vector->array[index].selector_t.ptr_value;

    /* sanity check on the time handle */
    assert( time_event->time_event_handle == time_event_handle );

    time_event->time_of_execution = new_time;

    /* now we have to restore the order in the vector */
    xi_vector_heap_fix_order_up( vector, index );
    xi_vector_heap_fix_order_down( vector, index );

    return XI_STATE_OK;
}

xi_state_t xi_time_event_cancel( xi_vector_t* vector,
                                 xi_time_event_handle_t* time_event_handle,
                                 xi_time_event_t** cancelled_time_event )
{
    /* PRE-CONDITIONS */
    assert( NULL != vector );
    assert( NULL != time_event_handle );
    assert( NULL != time_event_handle->position );
    assert( NULL != cancelled_time_event );

    /* the element we would like to remove should be at position described by the
     * time_event_handle */

    xi_vector_index_type_t index = *time_event_handle->position;

    if ( index >= vector->elem_no || index < 0 )
    {
        return XI_ELEMENT_NOT_FOUND;
    }

    /* if it's somwhere else than the end, let's swap it with the last element */
    if ( index < vector->elem_no - 1 )
    {
        xi_vector_heap_swap_time_events( vector, index, vector->elem_no - 1 );
    }

    /* let's update the return parameter */
    *cancelled_time_event =
        ( xi_time_event_t* )vector->array[vector->elem_no - 1].selector_t.ptr_value;

    /* now we can remove that element from the vector */
    xi_vector_del( vector, vector->elem_no - 1 );

    xi_time_event_clean_time_event( *cancelled_time_event );

    /* whenever we swap we have to fix the order that might have been broken */
    if ( index != vector->elem_no )
    {
        /* restore the heap order in the vector for that element */
        xi_vector_heap_fix_order_down( vector, index );
        xi_vector_heap_fix_order_up( vector, index );
    }

    return XI_STATE_OK;
}

/* local helper function used to release the memory required for time event */
static void xi_time_event_destructor( union xi_vector_selector_u* selector, void* arg )
{
    XI_UNUSED( arg );

    xi_time_event_t* time_event = ( xi_time_event_t* )selector->ptr_value;
    time_event->position        = XI_TIME_EVENT_POSITION_INVALID;
    XI_SAFE_FREE( time_event );
}

void xi_time_event_destroy( xi_vector_t* vector )
{
    xi_vector_for_each( vector, &xi_time_event_destructor, NULL, 0 );
}
