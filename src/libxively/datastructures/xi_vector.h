/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_VECTOR_H__
#define __XI_VECTOR_H__

#include <stdint.h>

#include "xi_allocator.h"
#include "xi_debug.h"
#include "xi_macros.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ! This type has to be SIGNED ! */
typedef int8_t xi_vector_index_type_t;

union xi_vector_selector_u {
    void* ptr_value;
    intptr_t iptr_value;
    int32_t i32_value;
    uint32_t ui32_value;
};

typedef struct
{
    union xi_vector_selector_u selector_t;
} xi_vector_elem_t;

typedef struct
{
    xi_vector_elem_t* array;
    xi_vector_index_type_t elem_no;
    xi_vector_index_type_t capacity;
    xi_memory_type_t memory_type;
} xi_vector_t;

typedef void( xi_vector_for_t )( union xi_vector_selector_u*, void* arg );

/* helpers for vector data initialization */
#define XI_VEC_VALUE_UI32( v )                                                           \
    {                                                                                    \
        .ui32_value = v                                                                  \
    }
#define XI_VEC_VALUE_I32( v )                                                            \
    {                                                                                    \
        .i32_value = v                                                                   \
    }
#define XI_VEC_VALUE_PTR( v )                                                            \
    {                                                                                    \
        .ptr_value = v                                                                   \
    }
#define XI_VEC_VALUE_IPTR( v )                                                           \
    {                                                                                    \
        .iptr_value = v                                                                  \
    }

#define XI_VEC_ELEM( v )                                                                 \
    {                                                                                    \
        v                                                                                \
    }
#define XI_VEC_CONST_VALUE_PARAM( v ) ( const union xi_vector_selector_u ) v
#define XI_VEC_VALUE_PARAM( v ) ( union xi_vector_selector_u ) v

/* declaration of the comparition function type
 * suppose to return -1 if e0 < e1, 0 if e0 == e1 and 1 if e0 > e1 */
typedef int8_t( xi_vector_cmp_t )( const union xi_vector_selector_u* e0,
                                   const union xi_vector_selector_u* e1 );

typedef int8_t( xi_vector_pred_t )( union xi_vector_selector_u* e0 );

extern xi_vector_t* xi_vector_create();

/**
 * @brief xi_vector_create_from
 *
 * In the vector implementation it is possible to create vector from the chunk of already
 * allocated memory.
 *
 * returns new vector created on given memory or NULL if there is not enough memory to
 * create the vector structure
 *
 * @param array
 * @param len
 * @param memory_type
 * @return xi_vector_t*
 */
extern xi_vector_t* xi_vector_create_from( xi_vector_elem_t* array,
                                           size_t len,
                                           xi_memory_type_t memory_type );

/**
 * @brief xi_vector_reserve
 *
 * Changes the capacity of the vector. If the capacity is lower than the previous one
 * elements at the end will be lost. Function returns 0 on error and 1 on success.
 *
 * @param vector
 * @param n
 * @return int8_t
 */
extern int8_t xi_vector_reserve( xi_vector_t* vector, xi_vector_index_type_t n );

extern xi_vector_t* xi_vector_destroy( xi_vector_t* vector );

extern const xi_vector_elem_t*
xi_vector_push( xi_vector_t* vector, const union xi_vector_selector_u value );

extern void xi_vector_swap_elems( xi_vector_t* vector,
                                  xi_vector_index_type_t i0,
                                  xi_vector_index_type_t i1 );

extern void xi_vector_del( xi_vector_t* vector, xi_vector_index_type_t index );

extern void xi_vector_remove_if( xi_vector_t* vector, xi_vector_pred_t* fun_pred );

extern xi_vector_index_type_t xi_vector_find( xi_vector_t* vector,
                                              const union xi_vector_selector_u value,
                                              xi_vector_cmp_t* fun_cmp );

extern void xi_vector_for_each( xi_vector_t* vector,
                                xi_vector_for_t* fun_for,
                                void* arg,
                                xi_vector_index_type_t offset );

extern void* xi_vector_get( xi_vector_t* vector, xi_vector_index_type_t index );

#ifdef __cplusplus
}
#endif

#endif /* __XI_VECTOR_H__ */
