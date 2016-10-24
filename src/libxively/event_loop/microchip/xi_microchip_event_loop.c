/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#include "TCPIP Stack/TCPIP.h"
#include <time.h>

#include "xively.h"
#include "xi_helpers.h"
#include "xi_coroutine.h"
#include "xi_globals.h"
#include "xi_event_dispatcher_api.h"
#include "xi_time.h"

#include "ETHPIC32ExtPhy.h"

void
xi_event_loop_with_evtds( uint32_t num_iterations,
                                                 xi_evtd_instance_t** event_dispatchers,
                                                 uint8_t num_evtds )
{
    num_iterations           = 2;
    uint32_t loops_processed = 0;

    while ( xi_evtd_all_continue( event_dispatchers, num_evtds ) &&
            ( 0 == num_iterations || loops_processed < num_iterations ) )
    {
        ++loops_processed;

        uint8_t it_evtd = 0;
        for ( ; it_evtd < num_evtds; ++it_evtd )
        {
            xi_evtd_instance_t* event_dispatcher = event_dispatchers[it_evtd];

            xi_static_vector_index_type_t i = 0;
            xi_evtd_fd_tuple_t* tuple =
                ( xi_evtd_fd_tuple_t* )event_dispatcher->handles_and_socket_fd->array[i]
                    .selector_t.ptr_value;

            for ( i = 0; i < event_dispatcher->handles_and_socket_fd->elem_no; ++i )
            {
                if ( ( ( tuple->event_type & XI_EVENT_WANT_CONNECT ) > 0 ) &&
                     ( TRUE == TCPIsConnected( tuple->fd ) ) )
                {
                    TCPFlush( tuple->fd );
                    TCPTick();

                    TCPWasReset( tuple->fd );

                    xi_evtd_update_event_on_socket( event_dispatcher, tuple->fd );
                    continue;
                }

                if ( ( tuple->event_type & XI_EVENT_WANT_CONNECT ) > 0 )
                {
                    continue;
                }

                if ( ( tuple->event_type & XI_EVENT_WANT_READ ) > 0 &&
                     ( TCPIsGetReady( tuple->fd ) > 0 ||
                       0 == TCPIsConnected( tuple->fd ) ) )
                {
                    xi_evtd_update_event_on_socket( xi_globals.evtd_instance,
                                                    tuple->fd );
                }
                else if ( ( tuple->event_type & XI_EVENT_WANT_WRITE ) > 0 &&
                          ( TCPIsPutReady( tuple->fd ) > 0 ||
                            0 == TCPIsConnected( tuple->fd ) ) )
                {
                    xi_evtd_update_event_on_socket( event_dispatcher, tuple->fd );
                }
                else if ( ( tuple->event_type & XI_EVENT_ERROR ) > 0 )
                {
                    xi_evtd_update_event_on_socket( event_dispatcher, tuple->fd );
                }

                xi_evtd_update_file_fd_events( event_dispatcher );
            }
        }

        TCPTick();

        it_evtd = 0;
        for ( ; it_evtd < num_evtds; ++it_evtd )
        {
            xi_evtd_step( event_dispatchers[it_evtd], xi_getcurrenttime_seconds() );
        }
    }
}
