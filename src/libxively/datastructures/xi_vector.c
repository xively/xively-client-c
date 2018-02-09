/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_vector.h"

#define XI_VECTOR_DEFAULT_CAPACITY 2

/**
 * \brief reserve new block of memory, copy data and release the one previously used
 * \return 1 in case of success 0 otherwise
 */
static int8_t xi_vector_realloc( xi_vector_t* vector, xi_vector_index_type_t new_size )
{
    assert( new_size != 0 && new_size != vector->capacity );

    /* do not allow to reallocate unmanaged memory blocks */
    if ( XI_MEMORY_TYPE_MANAGED != vector->memory_type )
    {
        return 0;
    }

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

    ret->memory_type = XI_MEMORY_TYPE_MANAGED;

    XI_CHECK_MEMORY( xi_vector_realloc( ret, XI_VECTOR_DEFAULT_CAPACITY ), state );

    return ret;

err_handling:
    if ( ret )
    {
        XI_SAFE_FREE( ret->array );
    }
    XI_SAFE_FREE( ret );
    return NULL;
}

xi_vector_t*
xi_vector_create_from( xi_vector_elem_t* array, size_t len, xi_memory_type_t memory_type )
{
    assert( array != 0 );
    assert( len > 0 );

    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_vector_t, ret, state );

    ret->array       = array;
    ret->memory_type = memory_type;
    ret->capacity    = len;
    ret->elem_no     = len;

    return ret;

err_handling:
    if ( ret )
    {
        xi_vector_destroy( ret );
    }
    return NULL;
}

int8_t xi_vector_reserve( xi_vector_t* vector, xi_vector_index_type_t n )
{
    assert( NULL != vector );
    assert( n > 0 );

    /* don't reserve if capacity is already set */
    if ( n == vector->capacity )
    {
        return 1;
    }

    /* trim the number of elements to the new size */
    if ( vector->elem_no > n )
    {
        vector->elem_no = n;
    }

    int8_t result = xi_vector_realloc( vector, n );

    if ( result == 0 )
    {
        return 0;
    }

    return 1;
}

xi_vector_t* xi_vector_destroy( xi_vector_t* vector )
{
    /* PRECONDITION */
    assert( NULL != vector );
    assert( NULL != vector->array );

    /* only managed type memory can be released by the vector who owns it */
    if ( XI_MEMORY_TYPE_MANAGED == vector->memory_type )
    {
        XI_SAFE_FREE( vector->array );
    }

    XI_SAFE_FREE( vector );

    return NULL;
}

const xi_vector_elem_t*
xi_vector_push( xi_vector_t* vector, const union xi_vector_selector_u value )
{
    /* PRECONDITION */
    assert( NULL != vector );

    xi_state_t state = XI_STATE_OK;

    if ( vector->elem_no + 1 > vector->capacity )
    {
        XI_CHECK_MEMORY( xi_vector_realloc( vector, vector->capacity * 2 ), state );
    }

    vector->array[vector->elem_no].selector_t = value;
    vector->elem_no += 1;

    return &vector->array[vector->elem_no - 1];

err_handling:
    return NULL;
}

void xi_vector_swap_elems( xi_vector_t* vector,
                           xi_vector_index_type_t i0,
                           xi_vector_index_type_t i1 )
{
    /* PRECONDITIONS */
    assert( NULL != vector );
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
    assert( NULL != vector );
    assert( index >= 0 );
    assert( vector->elem_no > 0 && index < vector->elem_no );

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
}

xi_vector_index_type_t xi_vector_find( xi_vector_t* vector,
                                       const union xi_vector_selector_u value,
                                       xi_vector_cmp_t* fun_cmp )
{
    /* PRECONDITIONS */
    assert( NULL != vector );
    assert( NULL != fun_cmp );

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

void xi_vector_for_each( xi_vector_t* vector,
                         xi_vector_for_t* fun_for,
                         void* arg,
                         xi_vector_index_type_t offset )
{
    assert( NULL != vector );
    assert( offset >= 0 );
    assert( offset <= vector->elem_no );

    xi_vector_index_type_t i = offset;

    for ( ; i < vector->elem_no; ++i )
    {
        ( *fun_for )( &vector->array[i].selector_t, arg );
    }
}

void* xi_vector_get( xi_vector_t* vector, xi_vector_index_type_t index )
{
    if ( NULL == vector || vector->elem_no <= index || index < 0 )
    {
        return NULL;
    }

    return vector->array[index].selector_t.ptr_value;
}

void xi_vector_remove_if( xi_vector_t* vector, xi_vector_pred_t* fun_pred )
{
    /* PRECONDITIONS */
    assert( NULL != vector );

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
