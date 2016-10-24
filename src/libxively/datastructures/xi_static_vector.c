/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#include "xi_static_vector.h"

xi_static_vector_t* xi_static_vector_create( xi_static_vector_index_type_t capacity )
{
    /* PRECONDITION */
    assert( capacity > 0 );

    xi_state_t state  = XI_STATE_OK;
    size_t elems_size = capacity * sizeof( xi_static_vector_elem_t );

    XI_ALLOC( xi_static_vector_t, ret, state );

    XI_ALLOC_BUFFER_AT( xi_static_vector_elem_t, ret->array, elems_size, state );

    ret->capacity    = capacity;
    ret->memory_type = XI_MEMORY_TYPE_MANAGED;

    return ret;

err_handling:
    if ( ret )
    {
        xi_static_vector_destroy( ret );
    }
    return 0;
}

xi_static_vector_t* xi_static_vector_create_from( xi_static_vector_elem_t* array,
                                                  size_t len,
                                                  xi_memory_type_t memory_type )
{
    assert( array != 0 );
    assert( len > 0 );

    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_static_vector_t, ret, state );

    ret->array       = array;
    ret->memory_type = memory_type;
    ret->capacity    = len;
    ret->elem_no     = len;

    return ret;

err_handling:
    if ( ret )
    {
        xi_static_vector_destroy( ret );
    }
    return 0;
}

xi_static_vector_t* xi_static_vector_destroy( xi_static_vector_t* vector )
{
    /* PRECONDITION */
    assert( vector != 0 );
    assert( vector->array != 0 );

    if ( XI_MEMORY_TYPE_MANAGED == vector->memory_type )
    {
        XI_SAFE_FREE( vector->array );
    }

    XI_SAFE_FREE( vector );

    return 0;
}

const xi_static_vector_elem_t*
xi_static_vector_push( xi_static_vector_t* vector,
                       const union xi_static_vector_selector_u value )
{
    /* PRECONDITION */
    assert( vector != 0 );

    if ( vector->elem_no + 1 <= vector->capacity )
    {
        vector->array[vector->elem_no].selector_t = value;
        vector->elem_no += 1;

        return &vector->array[vector->elem_no - 1];
    }

    return 0;
}

void xi_static_vector_swap_elems( xi_static_vector_t* vector,
                                  xi_static_vector_index_type_t i0,
                                  xi_static_vector_index_type_t i1 )
{
    /* PRECONDITIONS */
    assert( vector != 0 );
    assert( i0 >= 0 );
    assert( i1 >= 0 );
    assert( i0 <= vector->elem_no - 1 );
    assert( i1 <= vector->elem_no - 1 );

    union xi_static_vector_selector_u tmp_value = vector->array[i0].selector_t;
    vector->array[i0].selector_t                = vector->array[i1].selector_t;
    vector->array[i1].selector_t                = tmp_value;
}

void xi_static_vector_del( xi_static_vector_t* vector,
                           xi_static_vector_index_type_t index )
{
    /* PRECONDITIONS */
    assert( vector != 0 );
    assert( index >= 0 );
    assert( vector->elem_no > 0 && index < vector->elem_no );

    if ( vector->elem_no > 0 && index < vector->elem_no )
    {
        if ( index != vector->elem_no - 1 )
        {
            xi_static_vector_swap_elems( vector, index, vector->elem_no - 1 );
        }

        vector->elem_no -= 1;
        memset( &vector->array[vector->elem_no].selector_t, 0,
                sizeof( vector->array[vector->elem_no].selector_t ) );
    }
}

void xi_static_vector_for_each( xi_static_vector_t* vector,
                                xi_static_vector_for_t* fun_for )
{
    assert( vector != 0 );

    xi_static_vector_index_type_t i = 0;

    for ( i = 0; i < vector->elem_no; ++i )
    {
        ( *fun_for )( &vector->array[i].selector_t );
    }
}

void xi_static_vector_remove_if( xi_static_vector_t* vector,
                                 xi_static_vector_pred_t* fun_pred )
{
    /* PRECONDITIONS */
    assert( vector != 0 );

    xi_static_vector_index_type_t i     = 0;
    xi_static_vector_index_type_t i_end = vector->elem_no;

    for ( i = i_end - 1; i >= 0; --i )
    {
        if ( ( *fun_pred )( &vector->array[i].selector_t ) == 1 )
        {
            xi_static_vector_del( vector, i );
        }
    }
}

xi_static_vector_index_type_t
xi_static_vector_find( xi_static_vector_t* vector,
                       const union xi_static_vector_selector_u value,
                       xi_static_vector_cmp_t* fun_cmp )
{
    /* PRECONDITIONS */
    assert( vector != 0 );

    xi_static_vector_index_type_t i = 0;

    for ( i = 0; i < vector->elem_no; ++i )
    {
        if ( ( *fun_cmp )( &vector->array[i].selector_t, &value ) == 0 )
        {
            return i;
        }
    }

    return -1;
}
