/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "stack_allocator_debug.h"

void stack_debug_entry_visitor( const list_t* const list,
                                stack_entry_visitor* visitor,
                                void* stack_entry_visitor_arg )
{
    list_node_t* tmp = list->head;

    while ( NULL != tmp )
    {
        visitor( ( stack_allocator_entry_t* )tmp->value, stack_entry_visitor_arg );
        tmp = tmp->next;
    }
}

void stack_debug_print_entry( const stack_allocator_entry_t* const entry )
{
    const intptr_t external_aligned_addr =
        get_external_aligned_addr_from_internal_aligned_addr(
            extra_space_size, XI_ALIGN, align_address( XI_ALIGN, ( intptr_t )entry ) );
    const intptr_t space_wasted = external_aligned_addr - entry->raw_address;

    debug_log( "%s", "entry:" );
    debug_log( "entry's raw address:\t\t\t%p", ( void* )entry->raw_address );
    debug_log( "entry's size:\t\t\t\t%zu", entry->size );
    debug_log( "entry's external address:\t\t%p", ( void* )external_aligned_addr );
    debug_log( "space wasted:\t\t\t\t%d which is %.2f%%", ( int )space_wasted,
               ( space_wasted / ( float )entry->size ) * 100.0f );
    debug_log( "%s", "------------------------======------------------------" );
}

void stack_debug_statistic_gatherer( const stack_allocator_entry_t* const entry,
                                     void* arg )
{
    const intptr_t external_aligned_addr =
        get_external_aligned_addr_from_internal_aligned_addr(
            extra_space_size, XI_ALIGN, align_address( XI_ALIGN, ( intptr_t )entry ) );
    const intptr_t space_wasted = external_aligned_addr - entry->raw_address;

    stack_allocator_statistics_t* statistics = ( stack_allocator_statistics_t* )arg;

    statistics->no_of_blocks += 1;

    statistics->avg_block_size += entry->size;
    statistics->avg_space_wasted += space_wasted;
}

void stack_debug_print_statistics( const stack_allocator_statistics_t* const stats )
{
    const float avg_block_size   = stats->avg_block_size / stats->no_of_blocks;
    const float avg_space_wasted = stats->avg_space_wasted / stats->no_of_blocks;

    debug_log( "%s", "Statistics:" );
    debug_log( "No. blocks: %d", stats->no_of_blocks );
    debug_log( "Avg. block size[bytes]: %.2f", avg_block_size );
    debug_log( "Avg. space wasted[bytes]: %.2f", avg_space_wasted );
    debug_log( "Avg. space wasted/avg block size[%%]: %.2f",
               ( avg_space_wasted / avg_block_size ) * ( float )100 );
    debug_log( "%s", "------------------------======------------------------" );
}
