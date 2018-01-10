/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __STACK_ALLOCATOR_DEBUG_H__
#define __STACK_ALLOCATOR_DEBUG_H__

#include "list.h"
#include "stack_allocator.h"

typedef struct stack_allocator_statistics_s
{
    int no_of_blocks;
    float avg_space_wasted;
    float avg_block_size;
} stack_allocator_statistics_t;

#define empty_stack_allocator_statistic                                                  \
    {                                                                                    \
        0, .0f, .0f                                                                      \
    }

typedef void( stack_entry_visitor )( const stack_allocator_entry_t* const entry,
                                     void* arg );
void stack_debug_entry_visitor( const list_t* const list,
                                stack_entry_visitor* visitor,
                                void* stack_entry_visitor_arg );
void stack_debug_print_entry( const stack_allocator_entry_t* const entry );
void stack_debug_statistic_gatherer( const stack_allocator_entry_t* const entry,
                                     void* arg );
void stack_debug_print_statistics( const stack_allocator_statistics_t* const stats );
#endif /* __STACK_ALLOCATOR_DEBUG_H__ */
