// Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
//
// This is part of the Xively C Client library,
// it is licensed under the BSD 3-Clause license.

#include <stdio.h>

#include "xi_globals.h"
#include "xi_handle.h"
#include "xi_libxively_driver_impl.h"
#include "xi_macros.h"
#include "xi_memory_checks.h"
#include "xively.h"
#include <getopt.h>
#include <xi_event_loop.h>
#include <xi_bsp_time.h>

#ifdef XI_PLATFORM_BASE_POSIX
#include <execinfo.h>
#endif

#ifdef XI_MEMORY_LIMITER_ENABLED

#if XI_DEBUG_EXTRA_INFO
const char* xi_memory_checks_get_filename( const char* filename_and_path )
{
    const char* p = filename_and_path;
    for ( ; *p != '\0'; ++p )
        ;
    for ( ; *p != '/' && p > filename_and_path; --p )
        ;
    if ( *p == '/' )
        ++p;
    return p;
}

void xi_memory_checks_log_memory_leak( const xi_memory_limiter_entry_t* entry )
{
    fprintf( stderr, "\x1b[33m \t [MLD] --- %zu bytes lost, allocated in %s:%zu\x1b[0m\n",
             entry->size,
             xi_memory_checks_get_filename( entry->allocation_origin_file_name ),
             entry->allocation_origin_line_number );

#ifdef XI_PLATFORM_BASE_POSIX
    fprintf( stderr, "\x1b[33m\t\tbacktrace:\x1b[0m\n" );
    char** human_readable_symbols = backtrace_symbols(
        entry->backtrace_symbols_buffer, entry->backtrace_symbols_buffer_size );

    int i = 0;
    for ( ; i < entry->backtrace_symbols_buffer_size; ++i )
    {
        fprintf( stderr, "\t\t\t\x1b[33m%s\x1b[0m\n", human_readable_symbols[i] );
    }
#endif

    fflush( stderr );
}
#endif /* XI_DEBUG_EXTRA_INFO */

uint8_t _xi_memory_limiter_teardown()
{
    /* check for memory leaks */
    if ( !xi_is_whole_memory_deallocated() )
    {
        fprintf( stderr,
                 "\x1b[31m [MLD] WARNING: Memory leak detected - total memory lost "
                 "- %ld bytes \x1b[0m\n",
                 xi_memory_limiter_get_allocated_space() );

#if XI_DEBUG_EXTRA_INFO
        /* print information about leaks */
        xi_memory_limiter_visit_memory_leaks( &xi_memory_checks_log_memory_leak );

        /* garbage collection */
        xi_memory_limiter_gc();
#else /* XI_DEBUG_EXTRA_INFO */
        fprintf( stderr, "\x1b[31m [MLD] This version has been built with "
                         "XI_DEBUG_EXTRA_INFO=0 garbage collection and memory leaks "
                         "locator doesn't work!!!  \x1b[0m\n" );
#endif
    }

    fflush( stderr );

    return 0;
}

#endif

#ifdef XI_MEMORY_LIMITER_ENABLED
#define xi_is_whole_memory_deallocated() ( xi_memory_limiter_get_allocated_space() == 0 )

uint8_t _xi_memory_limiter_teardown();
#define xi_memory_limiter_teardown _xi_memory_limiter_teardown

#else
#define xi_is_whole_memory_deallocated() 1
#define xi_memory_limiter_teardown() 1
#endif


void driver_on_connected()
{
    xi_debug_printf( "[ driver     ] %s\n", __func__ );
}

extern char* optarg;
extern int optind, optopt, opterr;

void xi_driver_init( int argc,
                     char const* argv[],
                     char** control_channel_host_out,
                     int* control_channel_port_out )
{
    int result_getopt = 0;

    /* this is platform specific code */
    opterr = 0;
    while ( ( result_getopt = getopt( argc, ( char* const* )argv, "h:p:" ) ) != -1 )
    {
        switch ( result_getopt )
        {
            case 'h':
                *control_channel_host_out = optarg;
                break;
            case 'p':
                *control_channel_port_out = atoi( optarg );
                break;
        }
    }
}

int main( int argc, char const* argv[] )
{
    /*************************************
     * libxively initialization **********
     *************************************/
    xi_initialize( "unique account id" );

    xi_context_handle_t xi_app_context_handle = xi_create_context();
    if ( XI_INVALID_CONTEXT_HANDLE >= xi_app_context_handle )
    {
        xi_debug_printf( "[ driver     ] "
                         "Could not create xively client application context!\n" );
        return -1;
    }

    xi_globals.default_context_handle = xi_app_context_handle;

    /*************************************
     * driver initialization *************
     *************************************/
    char* control_channel_host = "localhost";
    int control_channel_port   = 12345;

    xi_driver_init( argc, argv, &control_channel_host, &control_channel_port );

    xi_debug_printf( "[ driver     ] Connecting to control channel: [ %s : %d ]\n",
                     control_channel_host, control_channel_port );

    libxively_driver = xi_libxively_driver_create_instance();

    xi_libxively_driver_connect_with_callback( libxively_driver, control_channel_host,
                                               control_channel_port,
                                               driver_on_connected );

    // do a single step on driver's evtd to start control
    // channel connect before doing first empty select 1sec blocking
    xi_evtd_step( libxively_driver->context->context_data.evtd_instance,
                  xi_bsp_time_getcurrenttime_seconds() );

    xi_evtd_instance_t* evtd_all[2] = {
        xi_globals.evtd_instance, libxively_driver->context->context_data.evtd_instance};

    /***********************************************************
     * time-sliced actuation of both: libxively and driver *****
     ***********************************************************/
    while ( xi_evtd_dispatcher_continue( libxively_driver->evtd_instance ) == 1 )
    {
        xi_event_loop_with_evtds( 1, evtd_all, 2 );

        // artificial processing of events in libxively's and driver's dispatcher
        // this is for speeding up
        // driver->libxively and
        // driver->libxively->driver requests (avoiding select timeout between)
        xi_evtd_step( xi_globals.evtd_instance, xi_bsp_time_getcurrenttime_seconds() );
        xi_evtd_step( libxively_driver->context->context_data.evtd_instance,
                      xi_bsp_time_getcurrenttime_seconds() );
    }

    xi_libxively_driver_destroy_instance( &libxively_driver );

    xi_delete_context( xi_app_context_handle );
    xi_shutdown();

    return xi_memory_limiter_teardown() ? 0 : 1;
}
