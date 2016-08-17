#include "xi_memory_checks.h"
#include <stdio.h>

#ifdef XI_MEMORY_LIMITER_ENABLED

#if XI_DEBUG_EXTRA_INFO
const char* xi_memory_checks_get_filename( const char* filename_and_path )
{
    const char* p = filename_and_path;
    for( ; *p != '\0'; ++p );
    for( ; *p != '/' && p > filename_and_path; --p );
    if( *p == '/' ) ++p;
    return p;
}

void xi_memory_checks_log_memory_leak( const xi_memory_limiter_entry_t* entry )
{
    fprintf( stderr, "\x1b[33m \t [MLD] --- %zu bytes lost, allocated in %s:%zu\x1b[0m\r\n",
             entry->size,
             xi_memory_checks_get_filename( entry->allocation_origin_file_name ),
             entry->allocation_origin_line_number );
    fflush( stderr );
}
#endif /* XI_DEBUG_EXTRA_INFO */

void _xi_memory_limiter_tearup()
{
    if ( !xi_is_whole_memory_deallocated() )
    {
        fprintf( stderr, "\x1b[31m [MLD] Warning: Memory level before the tear up %zu "
                         "please check previously executed tests!\x1b[0m\r\n",
                 xi_memory_limiter_get_allocated_space() );

#if XI_DEBUG_EXTRA_INFO
        /* print information about leaks */
        xi_memory_limiter_visit_memory_leaks( &xi_memory_checks_log_memory_leak );

        /* garbage collection */
        xi_memory_limiter_gc();
        fprintf( stderr, "\x1b[31m [MLD] Memory has been cleaned for you!\x1b[0m\r\n" );

#else /* XI_DEBUG_EXTRA_INFO */
        fprintf( stderr, "\x1b[31m [MLD] This version has been built with "
                         "XI_DEBUG_EXTRA_INFO=0 garbage collection and memory leaks "
                         "locator doesn't work\x1b[0m\r\n" );
#endif
        fflush( stderr );
    }
}

void _xi_memory_limiter_teardown()
{
    /* check for memory leaks */
    if ( !xi_is_whole_memory_deallocated() )
    {
        fprintf( stderr, "\x1b[31m [MLD] WARNING: Memory leak detected - total memory lost "
                         "- %ld bytes \x1b[0m\r\n",
                 xi_memory_limiter_get_allocated_space() );

#if XI_DEBUG_EXTRA_INFO
        /* print information about leaks */
        xi_memory_limiter_visit_memory_leaks( &xi_memory_checks_log_memory_leak );

        /* garbage collection */
        xi_memory_limiter_gc();
#else /* XI_DEBUG_EXTRA_INFO */
        fprintf( stderr, "\x1b[31m [MLD] This version has been built with "
                         "XI_DEBUG_EXTRA_INFO=0 garbage collection and memory leaks "
                         "locator doesn't work!!!  \x1b[0m\r\n" );
#endif
    }

    fflush( stderr );
}

#endif
