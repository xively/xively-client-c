/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <stdint.h>

#include "xi_critical_section.h"
#include "xi_critical_section_def.h"
#include "xi_debug.h"
#include "xi_helpers.h"
#include "xi_macros.h"
#include "xi_memory_limiter.h"

#ifdef XI_PLATFORM_BASE_POSIX
#include <execinfo.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define get_ptr_from_entry( e )                                                          \
    ( void* )( ( intptr_t )e + sizeof( xi_memory_limiter_entry_t ) )

#define get_entry_from_ptr( p )                                                          \
    ( xi_memory_limiter_entry_t* )( ( intptr_t )p - sizeof( xi_memory_limiter_entry_t ) )

#define get_ptr_from_entry_const( e ) ( intptr_t ) get_ptr_from_entry( e )

#define get_entry_from_ptr_const( p )                                                    \
    ( xi_memory_limiter_entry_t* )get_entry_from_ptr( p )

#if XI_DEBUG_EXTRA_INFO
static xi_memory_limiter_entry_t* xi_memory_limiter_entry_list_head;

typedef const xi_memory_limiter_entry_t*( xi_memory_limiter_entry_list_on_elem_t )(
    const xi_memory_limiter_entry_t* entry, const void* user_data );

static const xi_memory_limiter_entry_t*
xi_memory_limiter_entry_array_same_predicate( const xi_memory_limiter_entry_t* entry,
                                              const void* user_data )
{
    return entry == user_data ? entry : NULL;
}

static xi_memory_limiter_entry_t*
xi_memory_limiter_entry_list_visitor( xi_memory_limiter_entry_list_on_elem_t* predicate,
                                      const void* user_data )
{
    xi_memory_limiter_entry_t* tmp = xi_memory_limiter_entry_list_head;

    while ( NULL != tmp )
    {
        if ( ( *predicate )( tmp, user_data ) != NULL )
        {
            return tmp;
        }

        tmp = tmp->next;
    }

    return NULL;
}
#endif

/* static initialisation of the critical section */
static struct xi_critical_section_s xi_memory_limiter_cs = {0};

static volatile size_t xi_memory_application_limit =
    XI_MEMORY_LIMITER_APPLICATION_MEMORY_LIMIT;
static volatile size_t xi_memory_total_limit =
    XI_MEMORY_LIMITER_APPLICATION_MEMORY_LIMIT + XI_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT;
static volatile size_t xi_memory_allocated = 0;

static xi_state_t
xi_memory_limiter_will_allocation_fit( xi_memory_limiter_allocation_type_t memory_type,
                                       size_t size_to_alloc )
{
    if ( xi_memory_allocated + size_to_alloc >
         ( memory_type == XI_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION
               ? xi_memory_application_limit
               : xi_memory_total_limit ) )
    {
        return XI_OUT_OF_MEMORY;
    }

    return XI_STATE_OK;
}

xi_state_t xi_memory_limiter_set_limit( const size_t new_memory_limit )
{
    const size_t current_required_capacity =
        xi_memory_limiter_get_allocated_space() + XI_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT;

    if ( new_memory_limit < current_required_capacity )
    {
        return XI_OUT_OF_MEMORY;
    }

    xi_memory_total_limit = new_memory_limit;
    xi_memory_application_limit =
        xi_memory_total_limit - XI_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT;

    return XI_STATE_OK;
}

size_t xi_memory_limiter_get_capacity( xi_memory_limiter_allocation_type_t limit_type )
{
    assert( limit_type <= XI_MEMORY_LIMITER_ALLOCATION_TYPE_COUNT );

    return limit_type == XI_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION
               ? xi_memory_application_limit
               : xi_memory_total_limit;
}

size_t
xi_memory_limiter_get_current_limit( xi_memory_limiter_allocation_type_t limit_type )
{
    assert( limit_type <= XI_MEMORY_LIMITER_ALLOCATION_TYPE_COUNT );

    const size_t capacity        = xi_memory_limiter_get_capacity( limit_type );
    const size_t allocated_space = xi_memory_limiter_get_allocated_space();

    if ( allocated_space >= capacity )
    {
        return 0;
    }

    return capacity - allocated_space;
}

size_t xi_memory_limiter_get_allocated_space()
{
    return xi_memory_allocated;
}

void* xi_memory_limiter_alloc( xi_memory_limiter_allocation_type_t limit_type,
                               size_t size_to_alloc,
                               const char* file,
                               size_t line )
{
    assert( limit_type < XI_MEMORY_LIMITER_ALLOCATION_TYPE_COUNT );
    assert( NULL != file );

    /* just to satisfy the compiler */
    ( void )xi_memory_limiter_cs;

    void* ptr_to_ret = NULL;

    xi_lock_critical_section( &xi_memory_limiter_cs );

    xi_state_t local_state = XI_STATE_OK;

    size_t real_size_to_alloc = size_to_alloc + sizeof( xi_memory_limiter_entry_t );

    local_state = xi_memory_limiter_will_allocation_fit( limit_type, real_size_to_alloc );

    if ( XI_STATE_OK != local_state )
    {
        goto end;
    }

    /* this is where we are going to use the platform alloc */
    void* entry_ptr = __xi_alloc( real_size_to_alloc );

    if ( NULL == entry_ptr )
    {
        goto end;
    }

    ptr_to_ret = get_ptr_from_entry( entry_ptr );

    xi_memory_limiter_entry_t* entry = ( xi_memory_limiter_entry_t* )entry_ptr;
    memset( entry, 0, sizeof( xi_memory_limiter_entry_t ) );

#if XI_DEBUG_EXTRA_INFO
    entry->allocation_origin_file_name   = file;
    entry->allocation_origin_line_number = line;

#ifdef XI_PLATFORM_BASE_POSIX
    int no_of_backtraces =
        backtrace( entry->backtrace_symbols_buffer, MAX_BACKTRACE_SYMBOLS );
    entry->backtrace_symbols_buffer_size = no_of_backtraces;
#endif

    if ( NULL == xi_memory_limiter_entry_list_head )
    {
        xi_memory_limiter_entry_list_head = entry;
    }
    else
    {
        entry->next                             = xi_memory_limiter_entry_list_head;
        xi_memory_limiter_entry_list_head->prev = entry;
        xi_memory_limiter_entry_list_head       = entry;
    }
#else
    XI_UNUSED( file );
    XI_UNUSED( line );
#endif

    entry->size = real_size_to_alloc;
    xi_memory_allocated += real_size_to_alloc;

end:
    xi_unlock_critical_section( &xi_memory_limiter_cs );
    return ptr_to_ret;
}

void* xi_memory_limiter_calloc( xi_memory_limiter_allocation_type_t limit_type,
                                size_t num,
                                size_t size_to_alloc,
                                const char* file,
                                size_t line )
{
    const size_t allocation_size = num * size_to_alloc;
    void* ret = xi_memory_limiter_alloc( limit_type, allocation_size, file, line );

    /* it's unspecified if memset works with NULL pointer */
    if ( NULL != ret )
    {
        memset( ret, 0, allocation_size );
    }

    return ret;
}

/**
 * @brief simulate realloc on limited memory this will return valid pointer or
 * NULL if memory isn't availible
 */
void* xi_memory_limiter_realloc( xi_memory_limiter_allocation_type_t limit_type,
                                 void* ptr,
                                 size_t size_to_alloc,
                                 const char* file,
                                 size_t line )
{
    if ( NULL == ptr )
    {
        return NULL;
    }

    assert( limit_type < XI_MEMORY_LIMITER_ALLOCATION_TYPE_COUNT );
    assert( NULL != file );

    void* ptr_to_ret = NULL;

    xi_lock_critical_section( &xi_memory_limiter_cs );

    xi_state_t local_state = XI_STATE_OK;

    xi_memory_limiter_entry_t* entry = get_entry_from_ptr( ptr );

    const size_t real_size_to_alloc = size_to_alloc + sizeof( xi_memory_limiter_entry_t );

    const long long real_diff = real_size_to_alloc - ( long long )entry->size;

    local_state =
        xi_memory_limiter_will_allocation_fit( limit_type, XI_MAX( real_diff, 0 ) );

    if ( XI_STATE_OK != local_state )
    {
        goto end;
    }

#if XI_DEBUG_EXTRA_INFO
    if ( NULL != entry->prev )
    {
        entry->prev->next = entry->next;
    }
    else
    {
        xi_memory_limiter_entry_list_head = entry->next;
    }

    if ( NULL != entry->next )
    {
        entry->next->prev = entry->prev;
    }
#endif

    /* this is where we are going to use the platform alloc */
    void* r_ptr = __xi_realloc( entry, real_size_to_alloc );

    if ( NULL == r_ptr )
    {
        goto end;
    }

    entry = ( xi_memory_limiter_entry_t* )r_ptr;

#if XI_DEBUG_EXTRA_INFO
    entry->allocation_origin_file_name   = file;
    entry->allocation_origin_line_number = line;

    if ( NULL == xi_memory_limiter_entry_list_head )
    {
        xi_memory_limiter_entry_list_head = entry;
    }
    else
    {
        entry->next                             = xi_memory_limiter_entry_list_head;
        xi_memory_limiter_entry_list_head->prev = entry;
        xi_memory_limiter_entry_list_head       = entry;
    }
#else
    XI_UNUSED( file );
    XI_UNUSED( line );
#endif

    entry->size = real_size_to_alloc;
    xi_memory_allocated += real_diff;

    ptr_to_ret = get_ptr_from_entry( r_ptr );

end:
    xi_unlock_critical_section( &xi_memory_limiter_cs );
    return ptr_to_ret;
}

void xi_memory_limiter_free( void* ptr )
{
    if ( NULL == ptr )
    {
        return;
    }

    xi_lock_critical_section( &xi_memory_limiter_cs );

    xi_memory_limiter_entry_t* entry = get_entry_from_ptr( ptr );

    const size_t size_to_free = entry->size;

    /* this is the simplest check to verify the memory integrity */
    assert( xi_memory_limiter_get_allocated_space() >= size_to_free );

#if XI_DEBUG_EXTRA_INFO
    {
        xi_memory_limiter_entry_t* leg = xi_memory_limiter_entry_list_visitor(
            &xi_memory_limiter_entry_array_same_predicate, entry );

        XI_UNUSED( leg );

        assert( NULL != leg );
        assert( entry == leg );
    }

    if ( NULL != entry->prev )
    {
        entry->prev->next = entry->next;
    }
    else
    {
        xi_memory_limiter_entry_list_head = entry->next;
    }

    if ( NULL != entry->next )
    {
        entry->next->prev = entry->prev;
    }
#endif

    /* this is actual free */
    __xi_free( entry );

    xi_memory_allocated = XI_MAX( ( int )xi_memory_allocated - ( int )size_to_free, 0 );

    xi_unlock_critical_section( &xi_memory_limiter_cs );
}

void* xi_memory_limiter_alloc_application( size_t size_to_alloc,
                                           const char* file,
                                           size_t line )
{
    return xi_memory_limiter_alloc( XI_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION,
                                    size_to_alloc, file, line );
}

void* xi_memory_limiter_calloc_application( size_t num,
                                            size_t size_to_alloc,
                                            const char* file,
                                            size_t line )
{
    return xi_memory_limiter_calloc( XI_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION, num,
                                     size_to_alloc, file, line );
}

void* xi_memory_limiter_realloc_application( void* ptr,
                                             size_t size_to_alloc,
                                             const char* file,
                                             size_t line )
{
    return xi_memory_limiter_realloc( XI_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION, ptr,
                                      size_to_alloc, file, line );
}

void* xi_memory_limiter_alloc_system( size_t size_to_alloc,
                                      const char* file,
                                      size_t line )
{
    return xi_memory_limiter_alloc( XI_MEMORY_LIMITER_ALLOCATION_TYPE_SYSTEM,
                                    size_to_alloc, file, line );
}

void* xi_memory_limiter_calloc_system( size_t num,
                                       size_t size_to_alloc,
                                       const char* file,
                                       size_t line )
{
    return xi_memory_limiter_calloc( XI_MEMORY_LIMITER_ALLOCATION_TYPE_SYSTEM, num,
                                     size_to_alloc, file, line );
}

void* xi_memory_limiter_realloc_system( void* ptr,
                                        size_t size_to_alloc,
                                        const char* file,
                                        size_t line )
{
    return xi_memory_limiter_realloc( XI_MEMORY_LIMITER_ALLOCATION_TYPE_SYSTEM, ptr,
                                      size_to_alloc, file, line );
}

void* xi_memory_limiter_alloc_application_export( size_t size_to_alloc )
{
    return xi_memory_limiter_alloc_application( size_to_alloc, "exported alloc", 0 );
}

void* xi_memory_limiter_calloc_application_export( size_t num, size_t size_to_alloc )
{
    return xi_memory_limiter_calloc_application( num, size_to_alloc, "exported calloc",
                                                 0 );
}

void* xi_memory_limiter_realloc_application_export( void* ptr, size_t size_to_alloc )
{
    return xi_memory_limiter_realloc_application( ptr, size_to_alloc, "exported re-alloc",
                                                  0 );
}

#if XI_DEBUG_EXTRA_INFO
void xi_memory_limiter_gc()
{
    xi_lock_critical_section( &xi_memory_limiter_cs );

    xi_memory_limiter_entry_t* tmp = xi_memory_limiter_entry_list_head;

    /* traverse to the last element of the list */
    if ( NULL != tmp )
    {
        while ( NULL != tmp->next )
        {
            tmp = tmp->next;
        };
    }
    else /* the list is empty */
    {
        return;
    }

    /* tmp points to the last element of the list */
    /* idea is to peel the list from the bottom to the top ( reverse order of
     * deallocation
     * ) */
    while ( NULL != tmp )
    {
        xi_memory_limiter_entry_t* prev = tmp->prev;

        xi_unlock_critical_section( &xi_memory_limiter_cs );

        /* using memory limiter free let's deallocate the memory */
        xi_memory_limiter_free( get_ptr_from_entry( tmp ) );

        xi_lock_critical_section( &xi_memory_limiter_cs );

        tmp = prev;
    }

    xi_unlock_critical_section( &xi_memory_limiter_cs );
}

void xi_memory_limiter_visit_memory_leaks(
    void ( *visitor_fn )( const xi_memory_limiter_entry_t* ) )
{
    xi_lock_critical_section( &xi_memory_limiter_cs );

    xi_memory_limiter_entry_t* tmp = xi_memory_limiter_entry_list_head;

    /* traverse to the last element of the list calling the visitor function */
    if ( NULL != tmp )
    {
        do
        {
            visitor_fn( tmp );
            tmp = tmp->next;
        } while ( NULL != tmp );
    }

    xi_unlock_critical_section( &xi_memory_limiter_cs );
}

#endif /* XI_DEBUG_EXTRA_INFO */

#ifdef __cplusplus
}
#endif
