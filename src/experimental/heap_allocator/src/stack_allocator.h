/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XIVELY_EXPERIMENTAL_STACK_ALLOCATOR_H__
#define __XIVELY_EXPERIMENTAL_STACK_ALLOCATOR_H__

#include <assert.h>
#include <memory.h>
#include <stddef.h>
#include <stdint.h>

#include "debug.h"
#include "list.h"

/* HELPERS */

/* */
#define XI_ALIGN 4
typedef enum errors_e { STACK_ALLOCATOR_NOT_ENOUGH_MEMORY = -1 } errors_t;
extern const size_t extra_space_size;

/**
 * @brief stores information about the allocation entry
 */
typedef struct stack_allocator_entry_s
{
    /* original address of the memory block given ( unaligned ) we can easily calculate
     * aligned version of it and external aligned version of this address */
    intptr_t raw_address;
    /* place for a list node element required via list implementation */
    list_node_t list_node;
    /* memory buffer size - without the size of the stack_allocator_entry_t
     * itselt */
    size_t size;
} stack_allocator_entry_t;

typedef struct stack_allocator_s
{
    /* pointer to a beginnning of the memory buffer */
    uint8_t* memory_buffer;
    /* pointer to the first element of the free blocks list */
    list_t free_blocks;
    /* pointer to the first elemen of the allocated blocks list */
    list_t allocated_blocks;
    size_t memory_size;
} stack_allocator_t;

intptr_t align_address( intptr_t align, intptr_t offset );
intptr_t get_internal_aligned_addr_from_external_aligned_addr( size_t extra_size,
                                                               intptr_t align,
                                                               intptr_t aligned_addr );
intptr_t
get_external_aligned_addr_from_internal_aligned_addr( size_t extra_size,
                                                      intptr_t align,
                                                      intptr_t internal_aligned_addr );
int stack_allocator_init( stack_allocator_t* stack_allocator,
                          uint8_t* memory_buffer,
                          size_t memory_buffer_size );
void* stack_allocator_alloc( stack_allocator_t* stack_allocator, size_t size );
void stack_allocator_free( stack_allocator_t* stack_allocator, void* ptr );

#endif /* __XIVELY_EXPERIMENTAL_STACK_ALLOCATOR_H__ */
