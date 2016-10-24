/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#include "xi_vector.h"

/**
 * \brief reserve new block of memory, copy data and release the one previously used
 */
static int8_t xi_vector_realloc( xi_vector_t* vector, xi_vector_index_type_t new_size )
{
    assert( new_size != 0 && new_size != vector->capacity );

    xi_state_t state  = XI_STATE_OK;
    size_t elems_size = new_size * sizeof( xi_vector_elem_t );

    XI_ALLOC_BUFFER( xi_vector_elem_t, new_array, elems_size, state );

    if ( vector->elem_no > 0 )
    {
        memcpy( new_array, vector->array, sizeof( xi_vector_elem_t ) * vector->elem_no );
    }

    XI_SAFE_FREE( vector->array );

    vector->array    = new_array;
    vector->capacity = new_size;

    return 1;

err_handling:
    return 0;
}

xi_vector_t* xi_vector_create()
{
    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_vector_t, ret, state );

    XI_CHECK_MEMORY( xi_vector_realloc( ret, 2 ), state );

    return ret;

err_handling:
    if ( ret )
    {
        XI_SAFE_FREE( ret->array );
    }
    XI_SAFE_FREE( ret );
    return 0;
}

xi_vector_t* xi_vector_destroy( xi_vector_t* vector )
{
    /* PRECONDITION */
    assert( vector != 0 );
    assert( vector->array != 0 );

    XI_SAFE_FREE( vector->array );
    XI_SAFE_FREE( vector );

    return 0;
}

const xi_vector_elem_t*
xi_vector_push( xi_vector_t* vector, const union xi_vector_selector_u value )
{
    /* PRECONDITION */
    assert( vector != 0 );

    xi_state_t state = XI_STATE_OK;

    if ( vector->elem_no + 1 > vector->capacity )
    {
        XI_CHECK_MEMORY( xi_vector_realloc( vector, vector->capacity * 2 ), state );
    }

    vector->array[vector->elem_no].selector_t = value;
    vector->elem_no += 1;

    return &vector->array[vector->elem_no - 1];

err_handling:
    return 0;
}

void xi_vector_swap_elems( xi_vector_t* vector,
                           xi_vector_index_type_t i0,
                           xi_vector_index_type_t i1 )
{
    /* PRECONDITIONS */
    assert( vector != 0 );
    assert( i0 >= 0 );
    assert( i1 >= 0 );
    assert( i0 <= vector->elem_no - 1 );
    assert( i1 <= vector->elem_no - 1 );

    union xi_vector_selector_u tmp_value = vector->array[i0].selector_t;
    vector->array[i0].selector_t         = vector->array[i1].selector_t;
    vector->array[i1].selector_t         = tmp_value;
}

void xi_vector_del( xi_vector_t* vector, xi_vector_index_type_t index )
{
    /* PRECONDITIONS */
    assert( vector != 0 );
    assert( index >= 0 );
    assert( vector->elem_no > 0 && index < vector->elem_no );

    xi_state_t state = XI_STATE_OK;

    if ( vector->elem_no > 0 && index < vector->elem_no )
    {
        if ( index != vector->elem_no - 1 )
        {
            xi_vector_swap_elems( vector, index, vector->elem_no - 1 );
        }

        vector->elem_no -= 1;
        memset( &vector->array[vector->elem_no].selector_t, 0,
                sizeof( vector->array[vector->elem_no].selector_t ) );
    }

    /* if the element number has been decreased below
     * half of the capacity let's resize it down */
    xi_vector_index_type_t half_cap = vector->capacity / 2;
    if ( vector->elem_no < half_cap )
    {
        XI_CHECK_MEMORY( xi_vector_realloc( vector, half_cap ), state );
    }

err_handling:
    return;
}

xi_vector_index_type_t xi_vector_find( xi_vector_t* vector,
                                       const union xi_vector_selector_u value,
                                       xi_vector_cmp_t* fun_cmp )
{
    /* PRECONDITIONS */
    assert( vector != 0 );

    xi_vector_index_type_t i = 0;

    for ( i = 0; i < vector->elem_no; ++i )
    {
        if ( ( *fun_cmp )( &vector->array[i].selector_t, &value ) == 0 )
        {
            return i;
        }
    }

    return -1;
}

void xi_vector_for_each( xi_vector_t* vector, xi_vector_for_t* fun_for )
{
    assert( vector != 0 );

    xi_vector_index_type_t i = 0;

    for ( i = 0; i < vector->elem_no; ++i )
    {
        ( *fun_for )( &vector->array[i].selector_t );
    }
}

void* xi_vector_get( xi_vector_t* vector, xi_vector_index_type_t index )
{
    if ( vector->elem_no <= index || index < 0 )
    {
        return NULL;
    }
    return vector->array[index].selector_t.ptr_value;
}

void xi_vector_remove_if( xi_vector_t* vector, xi_vector_pred_t* fun_pred )
{
    /* PRECONDITIONS */
    assert( vector != 0 );

    xi_vector_index_type_t i     = 0;
    xi_vector_index_type_t i_end = vector->elem_no;

    for ( i = i_end - 1; i >= 0; --i )
    {
        if ( ( *fun_pred )( &vector->array[i].selector_t ) == 1 )
        {
            xi_vector_del( vector, i );
        }
    }
}
