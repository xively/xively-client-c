/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#ifndef __XI_STATIC_VECTOR_H__
#define __XI_STATIC_VECTOR_H__

#include <stdint.h>

#include "xi_debug.h"
#include "xi_allocator.h"
#include "xi_macros.h"
#include "xi_memory_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int8_t xi_static_vector_index_type_t;

union xi_static_vector_selector_u
{
    void* ptr_value;
    int32_t i32_value;
    uint32_t ui32_value;
};

typedef struct
{
    union xi_static_vector_selector_u selector_t;
} xi_static_vector_elem_t;

typedef void( xi_static_vector_for_t )( union xi_static_vector_selector_u* );

/* helpers for static vector data initialization */
#define XI_SVEC_VALUE_UI32( v )                                                          \
    {                                                                                    \
        .ui32_value = v                                                                  \
    }
#define XI_SVEC_VALUE_I32( v )                                                           \
    {                                                                                    \
        .i32_value = v                                                                   \
    }
#define XI_SVEC_VALUE_PTR( v )                                                           \
    {                                                                                    \
        .ptr_value = v                                                                   \
    }

#define XI_SVEC_ELEM( v )                                                                \
    {                                                                                    \
        v                                                                                \
    }
#define XI_SVEC_CONST_VALUE_PARAM( v ) ( const union xi_static_vector_selector_u ) v
#define XI_SVEC_VALUE_PARAM( v ) ( union xi_static_vector_selector_u ) v

typedef struct
{
    xi_static_vector_elem_t* array;
    xi_static_vector_index_type_t elem_no;
    xi_static_vector_index_type_t capacity;
    xi_memory_type_t memory_type;
} xi_static_vector_t;

/* declaration of the comparition function type
 * suppose to return -1 if e0 < e1, 0 if e0 == e1 and 1 if e0 > e1 */
typedef int8_t( xi_static_vector_cmp_t )( const union xi_static_vector_selector_u* e0,
                                          const union xi_static_vector_selector_u* e1 );

typedef int8_t( xi_static_vector_pred_t )( union xi_static_vector_selector_u* e0 );

extern xi_static_vector_t*
xi_static_vector_create( xi_static_vector_index_type_t capacity );

extern xi_static_vector_t* xi_static_vector_create_from( xi_static_vector_elem_t* array,
                                                         size_t len,
                                                         xi_memory_type_t data_source );

extern xi_static_vector_t* xi_static_vector_destroy( xi_static_vector_t* vector );

extern const xi_static_vector_elem_t*
xi_static_vector_push( xi_static_vector_t* vector,
                       const union xi_static_vector_selector_u value );

extern void xi_static_vector_swap_elems( xi_static_vector_t* vector,
                                         xi_static_vector_index_type_t i0,
                                         xi_static_vector_index_type_t i1 );

extern void
xi_static_vector_del( xi_static_vector_t* vector, xi_static_vector_index_type_t index );

extern void
xi_static_vector_for_each( xi_static_vector_t* vector, xi_static_vector_for_t* fun_for );

extern void xi_static_vector_remove_if( xi_static_vector_t* vector,
                                        xi_static_vector_pred_t* fun_pred );

extern xi_static_vector_index_type_t
xi_static_vector_find( xi_static_vector_t* vector,
                       const union xi_static_vector_selector_u value,
                       xi_static_vector_cmp_t* fun_cmp );

#ifdef __cplusplus
}
#endif

#endif /* __XI_STATIC_VECTOR_H__ */
