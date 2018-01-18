/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_handle.h"
#include "xively_types.h"

/* -----------------------------------------------------------------------
 *  INTERNAL FUNCTIONS
 * ----------------------------------------------------------------------- */

static inline int8_t xi_compare_context_pointers( const union xi_vector_selector_u* e0,
                                                  const union xi_vector_selector_u* e1 )
{
    return e0->ptr_value == e1->ptr_value ? 0 : 1;
}

/* -----------------------------------------------------------------------
 *  MAIN LIBRARY FUNCTIONS
 * ----------------------------------------------------------------------- */

void* xi_object_for_handle( xi_vector_t* vector, xi_handle_t handle )
{
    assert( vector != NULL );
    return xi_vector_get( vector, handle );
}

xi_state_t
xi_find_handle_for_object( xi_vector_t* vector, const void* object, xi_handle_t* handle )
{
    assert( vector != NULL );
    xi_vector_index_type_t handler_index = xi_vector_find(
        vector, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( ( void* )object ) ),
        xi_compare_context_pointers );
    if ( handler_index < 0 )
    {
        *handle = XI_INVALID_CONTEXT_HANDLE;
        return XI_ELEMENT_NOT_FOUND;
    }
    *handle = handler_index;
    return XI_STATE_OK;
}

xi_state_t xi_delete_handle_for_object( xi_vector_t* vector, const void* object )
{
    assert( vector != NULL );
    xi_vector_index_type_t handler_index = xi_vector_find(
        vector, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( ( void* )object ) ),
        xi_compare_context_pointers );

    if ( handler_index < 0 )
    {
        return XI_ELEMENT_NOT_FOUND;
    }
    vector->array[handler_index].selector_t.ptr_value = NULL;
    return XI_STATE_OK;
}

xi_state_t xi_register_handle_for_object( xi_vector_t* vector,
                                          const int32_t max_object_cnt,
                                          const void* object )
{
    assert( vector != NULL );
    xi_vector_index_type_t handler_index =
        xi_vector_find( vector, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( NULL ) ),
                        xi_compare_context_pointers );
    if ( handler_index < 0 )
    {
        if ( vector->elem_no >= max_object_cnt )
        {
            return XI_NO_MORE_RESOURCE_AVAILABLE;
        }
        xi_vector_push( vector,
                        XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( ( void* )object ) ) );
    }
    else
    {
        vector->array[handler_index].selector_t.ptr_value = ( void* )object;
    }
    return XI_STATE_OK;
}
