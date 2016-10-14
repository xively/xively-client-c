/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license. 
 */

#ifndef __XI_HEAP_H__
#define __XI_HEAP_H__

#include <stdint.h>

#include "xi_debug.h"
#include "xi_allocator.h"
#include "xi_macros.h"
#include "xi_time.h"

#include "xi_event_handle.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef xi_time_t xi_heap_key_type_t;
typedef uint8_t xi_heap_index_type_t;

typedef xi_event_handle_t xi_heap_value_type_t;

typedef struct xi_heap_element_s
{
    union {
        xi_heap_value_type_t type_value;
        void* void_value;
    } heap_value;
    xi_heap_key_type_t key;
    xi_heap_index_type_t index;
} xi_heap_element_t;

typedef struct xi_heap_s
{
    xi_heap_element_t** elements;
    xi_heap_index_type_t first_free;
    xi_heap_index_type_t capacity;
} xi_heap_t;

/* @TODO why are these same defines in xi_heap.c, too?
 * index calculus
 * note: the indexes are increased and decreased
 * in order to maintain the 0 - based indexing of elements */
#define LEFT( i ) ( ( ( i + 1 ) << 1 ) - 1 )
#define RIGHT( i ) ( ( ( ( i + 1 ) << 1 ) + 1 ) - 1 )
#define PARENT( i ) ( ( ( i + 1 ) >> 1 ) - 1 )

/* api */
extern xi_heap_t* xi_heap_create();

extern void xi_heap_destroy( xi_heap_t* xi_heap );

extern void xi_heap_elements_swap( xi_heap_element_t** a, xi_heap_element_t** b );

extern void xi_heap_fix_order_up( xi_heap_t* xi_heap, xi_heap_index_type_t index );

extern void xi_heap_fix_order_down( xi_heap_t* xi_heap, xi_heap_index_type_t index );

extern xi_heap_element_t* xi_heap_element_add( xi_heap_t* xi_heap,
                                               xi_heap_key_type_t key,
                                               xi_heap_value_type_t value );

extern xi_heap_element_t*
xi_heap_element_add_void( xi_heap_t* xi_heap, xi_heap_key_type_t key, void* value );

extern uint8_t xi_heap_element_remove( xi_heap_t* xi_heap, xi_heap_element_t* elem );

extern void xi_heap_element_update_key( xi_heap_t* xi_heap,
                                        xi_heap_element_t* elem,
                                        xi_heap_key_type_t key );

extern xi_heap_element_t* xi_heap_get_top( xi_heap_t* xi_heap );

extern xi_heap_element_t* xi_heap_peek_top( xi_heap_t* xi_heap );

extern uint8_t xi_heap_is_empty( xi_heap_t* xi_heap );

#ifdef __cplusplus
}
#endif

#endif /* __XI_HEAP_H__ */
