/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_event_loop.h"
#include "xi_bsp_io_net.h"
#include "xi_bsp_time.h"
#include "xi_event_dispatcher_api.h"


/**
 * @brief xi_bsp_event_loop_count_all_sockets
 * @param event_dispatchers
 * @param num_evtds
 * @return number of sockets that require update
 */
static size_t xi_bsp_event_loop_count_all_sockets( xi_evtd_instance_t** event_dispatchers,
                                                   uint8_t num_evtds )
{
    if ( NULL == event_dispatchers || 0 == num_evtds )
    {
        return XI_INVALID_PARAMETER;
    }

    size_t ret_num_of_sockets = 0;

    uint8_t evtd_id = 0;
    for ( ; evtd_id < num_evtds; ++evtd_id )
    {
        const xi_evtd_instance_t* event_dispatcher = event_dispatchers[evtd_id];
        assert( NULL != event_dispatcher );

        ret_num_of_sockets += ( size_t )event_dispatcher->handles_and_socket_fd->elem_no;
    }

    return ret_num_of_sockets;
}

xi_state_t
xi_bsp_event_loop_transform_to_bsp_select( xi_evtd_instance_t** in_event_dispatchers,
                                           uint8_t in_num_evtds,
                                           xi_bsp_socket_events_t* in_socket_events_array,
                                           size_t in_socket_events_array_length,
                                           xi_time_t* out_timeout )
{
    if ( NULL == out_timeout || NULL == in_event_dispatchers ||
         NULL == in_socket_events_array || 0 == in_num_evtds )
    {
        return XI_INVALID_PARAMETER;
    }

    size_t socket_id                  = 0;
    uint8_t was_file_updated          = 0;
    uint8_t was_timeout_candidate_set = 0;
    xi_time_t timeout_candidate       = 0;

    uint8_t evtd_id = 0;
    for ( evtd_id = 0; evtd_id < in_num_evtds; ++evtd_id )
    {
        xi_evtd_instance_t* event_dispatcher = in_event_dispatchers[evtd_id];
        assert( NULL != event_dispatcher );

        xi_vector_index_type_t i = 0;

        /* pick the smallest possible timeout with respect to all dispatchers */
        {
            xi_time_t tmp_timeout = 0;
            xi_state_t state =
                xi_evtd_get_time_of_earliest_event( event_dispatcher, &tmp_timeout );

            /* if the heap wasn't empty */
            if ( XI_STATE_OK == state )
            {
                /* if the timeout candidate has been initialised */
                if ( 1 == was_timeout_candidate_set )
                {
                    timeout_candidate = XI_MIN( timeout_candidate, tmp_timeout );
                }
                else /* if it hasn't been initialised */
                {
                    timeout_candidate = tmp_timeout;
                }

                was_timeout_candidate_set = 1;
            }
        }

        for ( i = 0; i < event_dispatcher->handles_and_socket_fd->elem_no; ++i )
        {
            xi_evtd_fd_tuple_t* tuple =
                ( xi_evtd_fd_tuple_t* )event_dispatcher->handles_and_socket_fd->array[i]
                    .selector_t.ptr_value;

            assert( NULL != tuple );
            assert( socket_id < in_socket_events_array_length );

            XI_UNUSED( in_socket_events_array_length );

            xi_bsp_socket_events_t* socket_to_update = &in_socket_events_array[socket_id];
            assert( NULL != socket_to_update );

            socket_to_update->xi_socket = tuple->fd;
            socket_to_update->in_socket_want_read =
                ( ( tuple->event_type & XI_EVENT_WANT_READ ) > 0 ) ? 1 : 0;
            socket_to_update->in_socket_want_write =
                ( ( tuple->event_type & XI_EVENT_WANT_WRITE ) > 0 ) ? 1 : 0;
            socket_to_update->in_socket_want_error =
                ( ( tuple->event_type & XI_EVENT_ERROR ) > 0 ) ? 1 : 0;
            socket_to_update->in_socket_want_connect =
                ( ( tuple->event_type & XI_EVENT_WANT_CONNECT ) > 0 ) ? 1 : 0;

            socket_id += 1;
        }

        was_file_updated |= xi_evtd_update_file_fd_events( event_dispatcher );
    }

    /* store the current time */
    const xi_time_t current_time = xi_bsp_time_getcurrenttime_seconds();

    /* recalculate the timeout */
    if ( was_timeout_candidate_set )
    {
        if ( timeout_candidate >= current_time )
        {
            timeout_candidate = timeout_candidate - current_time;
        }
        else
        {
            /* this is possible if the first event to execute is in the past */
            timeout_candidate = 0;
        }
    }
    else
    {
        timeout_candidate = XI_DEFAULT_IDLE_TIMEOUT;
    }

    /* make it clamped from the top */
    timeout_candidate = XI_MIN( timeout_candidate, XI_MAX_IDLE_TIMEOUT );

    /* update the return parameter */
    *out_timeout = ( was_file_updated != 0 ) ? ( 0 ) : ( timeout_candidate );

    return XI_STATE_OK;
}

xi_state_t
xi_bsp_event_loop_update_event_dispatcher( xi_evtd_instance_t** in_event_dispatchers,
                                           uint8_t in_num_evtds,
                                           xi_bsp_socket_events_t* in_socket_events_array,
                                           size_t in_socket_events_array_length )

{
    if ( NULL == in_event_dispatchers || 0 == in_num_evtds ||
         NULL == in_socket_events_array )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t state = XI_STATE_OK;
    size_t socket_id = 0;

    uint8_t evtd_id = 0;
    for ( evtd_id = 0; evtd_id < in_num_evtds; ++evtd_id )
    {
        xi_evtd_instance_t* event_dispatcher = in_event_dispatchers[evtd_id];

        xi_vector_index_type_t i = 0;

        for ( i = 0; i < event_dispatcher->handles_and_socket_fd->elem_no; ++i )
        {
            xi_evtd_fd_tuple_t* tuple =
                ( xi_evtd_fd_tuple_t* )event_dispatcher->handles_and_socket_fd->array[i]
                    .selector_t.ptr_value;

            /* mark things unused cause release build won't compile assertions */
            XI_UNUSED( tuple );
            XI_UNUSED( in_socket_events_array_length );

            /* sanity check */
            assert( NULL != tuple );

            /* make sure that the socket_id is valid */
            assert( socket_id < in_socket_events_array_length );
            xi_bsp_socket_events_t* socket_to_update = &in_socket_events_array[socket_id];

            /* sanity check on retrieved value */
            assert( NULL != socket_to_update );

            /* sanity check on socket id */
            assert( tuple->fd == socket_to_update->xi_socket );

            if ( 0 != socket_to_update->out_socket_can_read ||
                 0 != socket_to_update->out_socket_can_write ||
                 0 != socket_to_update->out_socket_connect_finished ||
                 0 != socket_to_update->out_socket_error )
            {
                state = xi_evtd_update_event_on_socket( event_dispatcher,
                                                        socket_to_update->xi_socket );
                XI_CHECK_STATE( state );
            }

            socket_id += 1;
        }
    }

err_handling:
    return state;
}

xi_state_t xi_event_loop_with_evtds( uint32_t num_iterations,
                                     xi_evtd_instance_t** event_dispatchers,
                                     uint8_t num_evtds )
{
    if ( NULL == event_dispatchers || 0 == num_evtds )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t state         = XI_STATE_OK;
    uint32_t loops_processed = 0;

    while ( xi_evtd_all_continue( event_dispatchers, num_evtds ) &&
            ( 0 == num_iterations || loops_processed < num_iterations ) )
    {
        loops_processed += 1;

        /* count all sockets that are registered */
        const size_t no_of_sockets_to_update =
            xi_bsp_event_loop_count_all_sockets( event_dispatchers, num_evtds );

        /* allocate and prepare space for the sockets */
        xi_bsp_socket_events_t array_of_sockets_to_update[no_of_sockets_to_update];
        memset( array_of_sockets_to_update, 0,
                sizeof( xi_bsp_socket_events_t ) * no_of_sockets_to_update );

        /* for storing the timeout */
        xi_time_t timeout = 0;

        /* transpose data from event dispatcher to socketd to update array */
        state = xi_bsp_event_loop_transform_to_bsp_select(
            event_dispatchers, num_evtds, array_of_sockets_to_update,
            no_of_sockets_to_update, &timeout );
        XI_CHECK_STATE( state );

        /* call the bsp select function */
        const xi_bsp_io_net_state_t select_state =
            xi_bsp_io_net_select( ( xi_bsp_socket_events_t* )&array_of_sockets_to_update,
                                  no_of_sockets_to_update, timeout );

        if ( XI_BSP_IO_NET_STATE_OK == select_state )
        {
            /* tranform output from bsp select to event dispatcher updates */
            state = xi_bsp_event_loop_update_event_dispatcher(
                event_dispatchers, num_evtds, array_of_sockets_to_update,
                no_of_sockets_to_update );
            XI_CHECK_STATE( state );
        }
        else if ( XI_BSP_IO_NET_STATE_ERROR == select_state )
        {
            state = XI_INTERNAL_ERROR;
            goto err_handling;
        }

        /* update time based events */
        uint8_t evtd_id = 0;
        for ( evtd_id = 0; evtd_id < num_evtds; ++evtd_id )
        {
            xi_evtd_step( event_dispatchers[evtd_id],
                          xi_bsp_time_getcurrenttime_seconds() );
        }
    }

err_handling:
    return state;
}
