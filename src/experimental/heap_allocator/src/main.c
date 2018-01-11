/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "stack_allocator.h"
#include "stack_allocator_debug.h"

#define UNUSED( arg ) ( void )arg

void stack_reporter( const stack_allocator_entry_t* const entry, void* arg )
{
    ( void )arg;
    stack_debug_print_entry( entry );
}

void print_stats( stack_allocator_t* allocator )
{
    stack_allocator_statistics_t stack_allocator_statistics_busy =
        empty_stack_allocator_statistic;
    stack_allocator_statistics_t stack_allocator_statistics_free =
        empty_stack_allocator_statistic;

    stack_debug_entry_visitor( &allocator->allocated_blocks,
                               stack_debug_statistic_gatherer,
                               &stack_allocator_statistics_busy );
    debug_log( "%s", "Statistics for allocated blocks of memory:" );
    stack_debug_print_statistics( &stack_allocator_statistics_busy );

    stack_debug_entry_visitor( &allocator->free_blocks, stack_debug_statistic_gatherer,
                               &stack_allocator_statistics_free );
    debug_log( "%s", "Statistics for free blocks of memory:" );
    stack_debug_print_statistics( &stack_allocator_statistics_free );
}

void deallocate_sequentially_elements( int no_of_deallocations,
                                       int elements_left,
                                       void** data,
                                       stack_allocator_t* allocator )
{
    int number_items_left = elements_left;

    for ( int i = 0; i < no_of_deallocations; ++i )
    {
        number_items_left -= 1;
        stack_allocator_free( allocator, data[number_items_left] );
    }

    print_stats( allocator );
}

void deallocate_randomly_elements( int no_of_deallocations,
                                   int elements_left,
                                   void** data,
                                   stack_allocator_t* allocator )
{
    int number_items_left = elements_left;

    for ( int i = 0; i < no_of_deallocations; ++i )
    {
        const int random_index = rand() % number_items_left;

        stack_allocator_free( allocator, data[random_index] );
        number_items_left -= 1;

        if ( random_index != number_items_left )
        {
            data[random_index] = data[number_items_left];
        }
    }

    print_stats( allocator );
}

typedef void( deallocation_fn_t )( int, int, void**, stack_allocator_t* );

void memory_test( int min_size,
                  int max_size,
                  int no_of_allocations,
                  int stack_memory_size,
                  deallocation_fn_t* deallocation_techinque )
{
    uint8_t* stack_memory = malloc( stack_memory_size );
    stack_allocator_t stack_allocator;

    const int half_no_of_allocations   = ( int )( no_of_allocations * 0.5f );
    const int quater_no_of_allocations = ( int )( half_no_of_allocations * 0.5f );
    const int eight_no_of_allocations  = ( int )( quater_no_of_allocations * 0.5f );
    const int max_min_diff             = max_size - min_size;
    int ret = stack_allocator_init( &stack_allocator, stack_memory, stack_memory_size );

    if ( ret < 0 )
    {
        debug_log( "%s", "failed to initialise stack_allocator" );
        return;
    }

    void** blocks = malloc( sizeof( void* ) * no_of_allocations );

    for ( int i = 0; i < no_of_allocations; ++i )
    {
        const int size_to_alloc = min_size + ( rand() % max_min_diff );
        blocks[i] = stack_allocator_alloc( &stack_allocator, size_to_alloc );
    }

    {
        stack_allocator_statistics_t stack_allocator_statistics_busy =
            empty_stack_allocator_statistic;
        stack_allocator_statistics_t stack_allocator_statistics_free =
            empty_stack_allocator_statistic;

        stack_debug_entry_visitor( &stack_allocator.allocated_blocks,
                                   stack_debug_statistic_gatherer,
                                   &stack_allocator_statistics_busy );
        debug_log( "%s", "Statistics for allocated blocks of memory:" );
        stack_debug_print_statistics( &stack_allocator_statistics_busy );

        stack_debug_entry_visitor( &stack_allocator.free_blocks,
                                   stack_debug_statistic_gatherer,
                                   &stack_allocator_statistics_free );
        debug_log( "%s", "Statistics for free blocks of memory:" );
        stack_debug_print_statistics( &stack_allocator_statistics_free );
    }

    int number_items_left = no_of_allocations;

    debug_log(
        "%s", "Now let's perform some random deallocation... let's say 50%% of allocated "
              "entries" );


    deallocation_techinque( half_no_of_allocations, number_items_left, blocks,
                            &stack_allocator );
    number_items_left -= half_no_of_allocations;


    debug_log(
        "%s",
        "Now let's perform some more random deallocation... let's say 50%% of what's "
        "left" );

    deallocation_techinque( quater_no_of_allocations, number_items_left, blocks,
                            &stack_allocator );
    number_items_left -= quater_no_of_allocations;

    debug_log(
        "%s",
        "Now let's perform some more random deallocation... let's say 50%% of what's "
        "left" );

    deallocation_techinque( eight_no_of_allocations, number_items_left, blocks,
                            &stack_allocator );
    number_items_left -= eight_no_of_allocations;

    debug_log( "%s", "Now let's remove of what's left" );

    deallocation_techinque( number_items_left, number_items_left, blocks,
                            &stack_allocator );
    number_items_left -= number_items_left;

    free( stack_memory );
    free( blocks );
}

int main( int argc, char* argv[] )
{
    UNUSED( argc );
    UNUSED( argv );

    time_t t;
    srand( ( unsigned )time( &t ) );

    debug_log( "%s", "-------------================================-------------" );
    debug_log( "%s", "-------------================================-------------" );
    debug_log( "%s", "~~~~~~~~~~~~~~Testing random deallocation....~~~~~~~~~~~~~" );
    debug_log( "%s", "-------------================================-------------" );
    debug_log( "%s", "-------------================================-------------" );
    memory_test( 10, 50, 10000, 1024 * 768, deallocate_randomly_elements );
    debug_log( "%s", "-------------================================-------------" );
    debug_log( "%s", "-------------================================-------------" );
    debug_log( "%s", "~~~~~~~~~~~~~Testing sequence deallocation.... ~~~~~~~~~~~" );
    debug_log( "%s", "-------------================================-------------" );
    debug_log( "%s", "-------------================================-------------" );
    memory_test( 10, 50, 10000, 1024 * 768, deallocate_sequentially_elements );
}
