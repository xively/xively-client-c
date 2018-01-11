/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_MEMORY_LIMITER_H__
#define __XI_MEMORY_LIMITER_H__

#include <stddef.h>

#include "xi_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#if XI_PLATFORM_BASE_POSIX
#define MAX_BACKTRACE_SYMBOLS 16
#endif

/**
 * internal structure for keeping the mapping between
 * the address of the allocated memory and the amount of the
 * memory that has been allocated
 */
typedef struct xi_memory_limiter_entry_s
{
#if XI_DEBUG_EXTRA_INFO
    struct xi_memory_limiter_entry_s* next;
    struct xi_memory_limiter_entry_s* prev;
    const char* allocation_origin_file_name;
#ifdef XI_PLATFORM_BASE_POSIX
    void* backtrace_symbols_buffer[MAX_BACKTRACE_SYMBOLS];
    int backtrace_symbols_buffer_size;
#endif
    size_t allocation_origin_line_number;
#endif
    size_t size;
} xi_memory_limiter_entry_t;

/**
 * types of memory alloations handled by the memory limiter
 *
 * application is the most common across the library. This is memory that is used
 * to faciliate applicaiton requests for (de)serializing and encoding data.
 *
 * system shares allocation space with application, but has a extra amount of reserved
 * space. Only xively mechanisms for scheduling and tracking tasks should use system
 * allocations, so that we can ensure that we have enough memory to schedule tasks
 * that unravel any currently running layer stacks when a application memory
 * exhaustion event occurs.
 */
typedef enum {
    XI_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION = 0,
    XI_MEMORY_LIMITER_ALLOCATION_TYPE_SYSTEM,
    XI_MEMORY_LIMITER_ALLOCATION_TYPE_COUNT
} xi_memory_limiter_allocation_type_t;


void* xi_memory_limiter_alloc( xi_memory_limiter_allocation_type_t limit_type,
                               size_t size_to_alloc,
                               const char* file,
                               size_t line );

void* xi_memory_limiter_calloc( xi_memory_limiter_allocation_type_t limit_type,
                                size_t num,
                                size_t size_to_alloc,
                                const char* file,
                                size_t line );

void* xi_memory_limiter_realloc( xi_memory_limiter_allocation_type_t limit_type,
                                 void* ptr,
                                 size_t size_to_alloc,
                                 const char* file,
                                 size_t line );

/**
 * @brief sets the limit of the memory limiter
 */
extern xi_state_t xi_memory_limiter_set_limit( const size_t new_memory_limit );

/**
 * @brief allows to get the limit
 */
extern size_t
xi_memory_limiter_get_capacity( xi_memory_limiter_allocation_type_t limit_type );

/**
 * @brief allows to get the main limit
 */
extern size_t
xi_memory_limiter_get_current_limit( xi_memory_limiter_allocation_type_t limit_type );

/**
 * @brief xi_memory_limiter_get_allocated_space
 */
extern size_t xi_memory_limiter_get_allocated_space();

/**
 * @brief simulates free operation on memory block it just re-add the memory to
 * the pool it will
 *          check if the pool hasn't been exceeded
 */
extern void xi_memory_limiter_free( void* ptr );

/**
 * @brief fasade for allocing in application memory space
 */
extern void* xi_memory_limiter_alloc_application( size_t size_to_alloc,
                                                  const char* file,
                                                  size_t line );

/*
 * @brief fasade for an (c)allocation in application memory space
 */
extern void* xi_memory_limiter_calloc_application( size_t num,
                                                   size_t size_to_alloc,
                                                   const char* file,
                                                   size_t line );


/**
 * @brief fasade for realloc in application memory space
 */
extern void* xi_memory_limiter_realloc_application( void* ptr,
                                                    size_t size_to_alloc,
                                                    const char* file,
                                                    size_t line );

/**
 * @brief fasade for alloc in system memory space
 */
extern void*
xi_memory_limiter_alloc_system( size_t size_to_alloc, const char* file, size_t line );

/**
 * @brief fasade for alloc in system memory space
 */
extern void* xi_memory_limiter_calloc_system( size_t num,
                                              size_t size_to_alloc,
                                              const char* file,
                                              size_t line );

/**
 * @brief fasade for realloc in system memory space
 */
extern void* xi_memory_limiter_realloc_system( void* ptr,
                                               size_t size_to_alloc,
                                               const char* file,
                                               size_t line );

/**
 * @brief xi_memory_limiter_alloc_application_export
 *
 * Fasade for allocating memory in application memory space in a form of wrapper
 * for outside usage.
 *
 * @see xi_memory_limiter_alloc_application
 *
 * @param size_to_alloc
 * @return
 */
extern void* xi_memory_limiter_alloc_application_export( size_t size_to_alloc );

/**
 * @brief xi_memory_limiter_calloc_application_export
 *
 * Fasade for (c)allocating memory in application memory space in a form of wrapper
 * for outside usage.
 *
 * @see xi_memory_limiter_calloc_application
 *
 * @param num
 * @param size_to_alloc
 * @return
 */
extern void*
xi_memory_limiter_calloc_application_export( size_t num, size_t size_to_alloc );

/**
 * @brief xi_memory_limiter_realloc_application_export
 *
 * Fasade for re-allocating memory in application memory space in a form of wrapper
 * for outside usage.
 *
 * @see xi_memory_limiter_realloc_application
 *
 * @param ptr
 * @param size_to_alloc
 * @return
 */
extern void*
xi_memory_limiter_realloc_application_export( void* ptr, size_t size_to_alloc );


#if XI_DEBUG_EXTRA_INFO

/**
 * @brief it does the garbage collection run ( only if the XI_DEBUG_EXTRA_INFO flag is
 * turned ON )
 */
extern void xi_memory_limiter_gc();

/**
 * @brief xi_debug_log_leaks
 */
extern void xi_memory_limiter_visit_memory_leaks(
    void ( *visitor_fn )( const xi_memory_limiter_entry_t* ) );

/**
 * @brief xi_debug_log_leaks
 */
extern void xi_memory_limiter_print_memory_leaks();

#endif /* XI_DEBUG_EXTRA_INFO */


#ifdef __cplusplus
}
#endif

#endif /* __XI_MEMORY_LIMITER_H__ */
