/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#include "xi_event_dispatcher_api.h"
#include "xi_io_posix_layer_compat.h"

#ifdef XI_MEMORY_LIMITER_ENABLED
#include "xi_memory_limiter.h"
#endif

void xi_event_loop_with_evtds( uint32_t num_iterations,
                               xi_evtd_instance_t** event_dispatchers,
                               uint8_t num_evtds )
{
    fd_set rfds;
    fd_set wfds;
    fd_set efds;

    FD_ZERO( &rfds );
    FD_ZERO( &wfds );
    FD_ZERO( &efds );

    struct timeval tv;
    uint32_t loops_processed = 0;

    while ( xi_evtd_all_continue( event_dispatchers, num_evtds ) &&
            ( 0 == num_iterations || loops_processed < num_iterations ) )
    {
        ++loops_processed;

        xi_static_vector_index_type_t i = 0;
        int max_fd_read                 = 0;
        int max_fd_write                = 0;
        int max_fd_error                = 0;
        uint8_t was_file_updated        = 0;

        tv.tv_sec  = 1;
        tv.tv_usec = 0;

        FD_ZERO( &rfds );
        FD_ZERO( &wfds );
        FD_ZERO( &efds );

        uint8_t it_evtd = 0;
        for ( ; it_evtd < num_evtds; ++it_evtd )
        {
            xi_evtd_instance_t* event_dispatcher = event_dispatchers[it_evtd];

            for ( i = 0; i < event_dispatcher->handles_and_socket_fd->elem_no; ++i )
            {
                xi_evtd_fd_tuple_t* tuple =
                    ( xi_evtd_fd_tuple_t* )event_dispatcher->handles_and_socket_fd
                        ->array[i]
                        .selector_t.ptr_value;

                if ( ( tuple->event_type & XI_EVENT_WANT_READ ) > 0 )
                {
                    FD_SET( tuple->fd, &rfds );
                    max_fd_read = tuple->fd > max_fd_read ? tuple->fd : max_fd_read;
                }

                if ( ( tuple->event_type & XI_EVENT_WANT_WRITE ) > 0 ||
                     ( tuple->event_type & XI_EVENT_WANT_CONNECT ) > 0 )
                {
                    FD_SET( tuple->fd, &wfds );
                    max_fd_write = tuple->fd > max_fd_write ? tuple->fd : max_fd_write;
                }

                if ( ( tuple->event_type & XI_EVENT_ERROR ) > 0 )
                {
                    FD_SET( tuple->fd, &efds );
                    max_fd_error = tuple->fd > max_fd_error ? tuple->fd : max_fd_error;
                }
            }

            was_file_updated |= xi_evtd_update_file_fd_events( event_dispatcher );
        }

        /* If there is any file operation pending, let's not block on select. */
        if ( was_file_updated > 0 )
        {
            tv.tv_sec  = 0;
            tv.tv_usec = 0;
        }

        const int max_fd = XI_MAX( XI_MAX( max_fd_read, max_fd_write ), max_fd_error );

        const int result = select( max_fd + 1, &rfds, &wfds, &efds, &tv );

        if ( result > 0 )
        {
            it_evtd = 0;
            for ( ; it_evtd < num_evtds; ++it_evtd )
            {
                xi_evtd_instance_t* event_dispatcher = event_dispatchers[it_evtd];

                for ( i = 0; i < event_dispatcher->handles_and_socket_fd->elem_no; ++i )
                {
                    xi_evtd_fd_tuple_t* tuple =
                        ( xi_evtd_fd_tuple_t* )event_dispatcher->handles_and_socket_fd
                            ->array[i]
                            .selector_t.ptr_value;

                    assert( XI_EVTD_FD_TYPE_SOCKET == tuple->fd_type );

                    if ( FD_ISSET( tuple->fd, &rfds ) )
                    {
                        xi_evtd_update_event_on_socket( event_dispatcher, tuple->fd );
                    }

                    if ( FD_ISSET( tuple->fd, &wfds ) )
                    {
                        xi_evtd_update_event_on_socket( event_dispatcher, tuple->fd );
                    }

                    if ( FD_ISSET( tuple->fd, &efds ) )
                    {
                        xi_evtd_update_event_on_socket( event_dispatcher, tuple->fd );
                    }
                }
            }
        }

        it_evtd = 0;
        for ( ; it_evtd < num_evtds; ++it_evtd )
        {
            xi_evtd_step( event_dispatchers[it_evtd], xi_getcurrenttime_seconds() );
        }

#ifdef XI_MEMORY_LIMITER_ENABLED
        xi_debug_format( "--== Memory usage: %zu ==--",
                         xi_memory_limiter_get_allocated_space(
                             XI_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION ) );
#endif
    }
}
