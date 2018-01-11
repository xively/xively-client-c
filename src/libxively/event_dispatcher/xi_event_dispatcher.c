/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <inttypes.h>

#include "xi_event_dispatcher_api.h"
#include "xi_list.h"
#include "xi_helpers.h"

static inline int8_t xi_evtd_cmp_fd( const union xi_vector_selector_u* e0,
                                     const union xi_vector_selector_u* value )
{
    xi_evtd_fd_tuple_t* tuple = ( xi_evtd_fd_tuple_t* )e0->ptr_value;

    assert( sizeof( tuple->fd ) == sizeof( value->iptr_value ) );

    if ( tuple->fd == value->iptr_value )
    {
        return 0;
    }

    return -1;
}

static int8_t xi_evtd_register_fd( xi_evtd_instance_t* instance,
                                   xi_vector_t* container,
                                   xi_event_type_t event_type,
                                   xi_evtd_fd_type_t fd_type,
                                   xi_fd_t fd,
                                   xi_event_handle_t read_handle,
                                   xi_event_handle_t current_handle )
{
    /* PRECONDITIONS */
    assert( NULL != container );
    assert( NULL != instance );
    assert( xi_vector_find( container,
                            XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_IPTR( fd ) ),
                            &xi_evtd_cmp_fd ) == -1 );

    XI_UNUSED( instance );

    xi_state_t state = XI_STATE_OK;

    xi_lock_critical_section( instance->cs );

    /* add an entry with the proper event for file descriptor */
    XI_ALLOC( xi_evtd_fd_tuple_t, tuple, state );

    tuple->fd          = fd;
    tuple->event_type  = event_type;
    tuple->read_handle = read_handle;
    tuple->handle      = current_handle;
    tuple->fd_type     = fd_type;

    /* register within the handles */
    {
        const xi_vector_elem_t* e = xi_vector_push(
            container, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( tuple ) ) );
        if ( NULL == e )
        {
            goto err_handling;
        }
    }

    xi_unlock_critical_section( instance->cs );

    return 1;

err_handling:
    XI_SAFE_FREE( tuple );
    xi_unlock_critical_section( instance->cs );

    return 0;
}

int8_t xi_evtd_register_file_fd( xi_evtd_instance_t* instance,
                                 xi_event_type_t event_type,
                                 xi_fd_t fd,
                                 xi_event_handle_t handle )
{
    return xi_evtd_register_fd( instance, instance->handles_and_file_fd, event_type,
                                XI_EVTD_FD_TYPE_FILE, fd, handle, handle );
}

int8_t xi_evtd_register_socket_fd( xi_evtd_instance_t* instance,
                                   xi_fd_t fd,
                                   xi_event_handle_t read_handle )
{
    return xi_evtd_register_fd( instance, instance->handles_and_socket_fd,
                                XI_EVENT_WANT_READ, XI_EVTD_FD_TYPE_SOCKET, fd,
                                read_handle, read_handle );
}

static int8_t
xi_evtd_unregister_fd( xi_evtd_instance_t* instance, xi_vector_t* container, xi_fd_t fd )
{
    /* PRE-CONDITIONS */
    assert( NULL != instance );
    assert( NULL != container );

    XI_UNUSED( instance );

    xi_lock_critical_section( instance->cs );

    xi_vector_index_type_t id = xi_vector_find(
        container, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_IPTR( fd ) ), &xi_evtd_cmp_fd );

    /* remove from the vector */
    if ( -1 != id )
    {
        assert( NULL != container->array[id].selector_t.ptr_value );
        XI_SAFE_FREE( container->array[id].selector_t.ptr_value );
        xi_vector_del( container, id );

        xi_unlock_critical_section( instance->cs );
        return 1;
    }

    xi_unlock_critical_section( instance->cs );

    return -1;
}

int8_t xi_evtd_unregister_file_fd( xi_evtd_instance_t* instance, xi_fd_t fd )
{
    return xi_evtd_unregister_fd( instance, instance->handles_and_file_fd, fd );
}


int8_t xi_evtd_unregister_socket_fd( xi_evtd_instance_t* instance, xi_fd_t fd )
{
    return xi_evtd_unregister_fd( instance, instance->handles_and_socket_fd, fd );
}

int8_t xi_evtd_continue_when_evt_on_socket( xi_evtd_instance_t* instance,
                                            xi_event_type_t event_type,
                                            xi_event_handle_t handle,
                                            xi_fd_t fd )
{
    /* PRECONDITIONS */
    assert( instance != 0 );

    xi_lock_critical_section( instance->cs );

    xi_vector_index_type_t id = xi_vector_find(
        instance->handles_and_socket_fd,
        XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_IPTR( fd ) ), &xi_evtd_cmp_fd );

    /* set up the values of the tuple */
    if ( -1 != id )
    {
        xi_evtd_fd_tuple_t* tuple =
            ( xi_evtd_fd_tuple_t* )instance->handles_and_socket_fd->array[id]
                .selector_t.ptr_value;

        assert( XI_EVTD_FD_TYPE_SOCKET == tuple->fd_type );

        tuple->event_type = event_type;
        tuple->handle     = handle;

        xi_unlock_critical_section( instance->cs );

        return 1;
    }

    xi_unlock_critical_section( instance->cs );

    return -1;
}

void xi_evtd_continue_when_empty( xi_evtd_instance_t* instance, xi_event_handle_t handle )
{
    instance->on_empty = handle;
}

xi_event_handle_queue_t*
xi_evtd_execute( xi_evtd_instance_t* instance, xi_event_handle_t handle )
{
    xi_state_t state = XI_STATE_OK;
    XI_ALLOC_SYSTEM( xi_event_handle_queue_t, queue_elem, state );

    queue_elem->handle = handle;

    xi_lock_critical_section( instance->cs );

    XI_LIST_PUSH_BACK( xi_event_handle_queue_t, instance->call_queue, queue_elem );

    xi_unlock_critical_section( instance->cs );

    return queue_elem;

err_handling:
    return NULL;
}

xi_state_t xi_evtd_execute_in( xi_evtd_instance_t* instance,
                               xi_event_handle_t handle,
                               xi_time_t time_diff,
                               xi_time_event_handle_t* ret_time_event_handle )
{
    xi_state_t ret_state = XI_STATE_OK;

    XI_ALLOC( xi_time_event_t, time_event, ret_state );

    time_event->event_handle      = handle;
    time_event->time_of_execution = instance->current_step + time_diff;

    xi_lock_critical_section( instance->cs );

    ret_state = xi_time_event_add( instance->time_events_container, time_event,
                                   ret_time_event_handle );

    xi_unlock_critical_section( instance->cs );

    return ret_state;

err_handling:
    return ret_state;
}

xi_state_t
xi_evtd_cancel( xi_evtd_instance_t* instance, xi_time_event_handle_t* time_event_handle )
{
    xi_time_event_t* time_event = NULL;
    xi_state_t ret_state        = XI_STATE_OK;

    xi_lock_critical_section( instance->cs );

    ret_state = xi_time_event_cancel( instance->time_events_container, time_event_handle,
                                      &time_event );

    xi_unlock_critical_section( instance->cs );

    XI_SAFE_FREE( time_event );

    return ret_state;
}

xi_state_t xi_evtd_restart( xi_evtd_instance_t* instance,
                            xi_time_event_handle_t* time_event_handle,
                            xi_time_t new_time )
{
    xi_state_t ret_state = XI_STATE_OK;

    xi_lock_critical_section( instance->cs );

    ret_state = xi_time_event_restart( instance->time_events_container, time_event_handle,
                                       instance->current_step + new_time );

    xi_unlock_critical_section( instance->cs );

    return ret_state;
}

xi_evtd_instance_t* xi_evtd_create_instance( void )
{
    xi_state_t state = XI_STATE_OK;
    XI_ALLOC( xi_evtd_instance_t, evtd_instance, state );

    evtd_instance->time_events_container = xi_vector_create();
    XI_CHECK_MEMORY( evtd_instance->time_events_container, state );

    evtd_instance->handles_and_socket_fd = xi_vector_create();
    XI_CHECK_MEMORY( evtd_instance->handles_and_socket_fd, state );

    evtd_instance->handles_and_file_fd = xi_vector_create();
    XI_CHECK_MEMORY( evtd_instance->handles_and_file_fd, state );

    XI_CHECK_STATE( xi_init_critical_section( &evtd_instance->cs ) );

    return evtd_instance;

err_handling:
    XI_SAFE_FREE( evtd_instance );
    return 0;
}

void xi_evtd_destroy_instance( xi_evtd_instance_t* instance )
{
    if ( instance == NULL )
        return;

    struct xi_critical_section_s* cs = instance->cs;

    XI_UNUSED( cs ); /* yeah, I know */

    xi_lock_critical_section( cs );

    xi_vector_destroy( instance->handles_and_file_fd );
    xi_vector_destroy( instance->handles_and_socket_fd );
    xi_time_event_destroy( instance->time_events_container );
    xi_vector_destroy( instance->time_events_container );

    XI_SAFE_FREE( instance );

    xi_unlock_critical_section( cs );

    xi_destroy_critical_section( &cs );
}

xi_event_handle_return_t xi_evtd_execute_handle( xi_event_handle_t* handle )
{
    switch ( handle->handle_type )
    {
        case XI_EVENT_HANDLE_ARGC0:
            return ( *handle->handlers.h0.fn_argc0 )();
        case XI_EVENT_HANDLE_ARGC1:
            return ( *handle->handlers.h1.fn_argc1 )( handle->handlers.h1.a1 );
        case XI_EVENT_HANDLE_ARGC2:
            return ( *handle->handlers.h2.fn_argc2 )( handle->handlers.h2.a1,
                                                      handle->handlers.h2.a2 );
        case XI_EVENT_HANDLE_ARGC3:
            return ( *handle->handlers.h3.fn_argc3 )(
                handle->handlers.h3.a1, handle->handlers.h3.a2, handle->handlers.h3.a3 );
        case XI_EVENT_HANDLE_ARGC4:
            return ( *handle->handlers.h4.fn_argc4 )(
                handle->handlers.h4.a1, handle->handlers.h4.a2, handle->handlers.h4.a3,
                handle->handlers.h4.a4 );
        case XI_EVENT_HANDLE_ARGC5:
            return ( *handle->handlers.h5.fn_argc5 )(
                handle->handlers.h5.a1, handle->handlers.h5.a2, handle->handlers.h5.a3,
                handle->handlers.h5.a4, handle->handlers.h5.a5 );
        case XI_EVENT_HANDLE_ARGC6:
            return ( *handle->handlers.h6.fn_argc6 )(
                handle->handlers.h6.a1, handle->handlers.h6.a2, handle->handlers.h6.a3,
                handle->handlers.h6.a4, handle->handlers.h6.a5, handle->handlers.h6.a6 );
        case XI_EVENT_HANDLE_UNSET:
            xi_debug_logger( "you are trying to call an unset handler!" );
#if XI_DEBUG_EXTRA_INFO
            xi_debug_format( "handler created in %s:%d",
                             handle->debug_info.debug_file_init,
                             handle->debug_info.debug_line_init );
#endif
            return ( xi_event_handle_return_t )XI_UNSET_HANDLER_ERROR;
    }

    return ( xi_event_handle_return_t )XI_STATE_OK;
}

static uint8_t xi_state_is_fatal( xi_state_t e )
{
    switch ( e )
    {
        case XI_OUT_OF_MEMORY:
        case XI_INTERNAL_ERROR:
        case XI_MQTT_UNKNOWN_MESSAGE_ID:
            return 1;
        default:
            return 0;
    }
}

extern uint8_t
xi_evtd_single_step( xi_evtd_instance_t* evtd_instance, xi_time_t new_step )
{
    if ( evtd_instance == NULL )
        return 0;

    evtd_instance->current_step = new_step;

    xi_event_handle_queue_t* queue_elem = NULL;

    xi_lock_critical_section( evtd_instance->cs );
    if ( !XI_LIST_EMPTY( xi_event_handle_queue_t, evtd_instance->call_queue ) )
    {
        XI_LIST_POP( xi_event_handle_queue_t, evtd_instance->call_queue, queue_elem );
    }
    xi_unlock_critical_section( evtd_instance->cs );

    if ( queue_elem == NULL )
        return 0;

    const xi_state_t result = xi_evtd_execute_handle( &queue_elem->handle );

    if ( xi_state_is_fatal( result ) == 1 )
    {
        xi_debug_logger( "error while processing normal events" );
    }

    XI_SAFE_FREE( queue_elem );

    return 1;
}

void xi_evtd_step( xi_evtd_instance_t* evtd_instance, xi_time_t new_step )
{
    if ( evtd_instance == NULL )
        return;

    evtd_instance->current_step = new_step;
    xi_time_event_t* tmp        = NULL;

#ifdef XI_DEUBG_OUTPUT_EVENT_SYSTEM
    xi_debug_format( "[size of time event queue: %d]",
                     evtd_instance->call_heap->first_free );
    xi_debug_logger( "[calling time events]" );
#endif

    xi_lock_critical_section( evtd_instance->cs );

    /* zero - not NULL elem_no it's a number not a pointer */
    while ( 0 != evtd_instance->time_events_container->elem_no )
    {
        tmp = xi_time_event_peek_top( evtd_instance->time_events_container );
        if ( tmp->time_of_execution <= evtd_instance->current_step )
        {
            tmp = xi_time_event_get_top( evtd_instance->time_events_container );
            xi_event_handle_t* handle = ( xi_event_handle_t* )&tmp->event_handle;

            xi_unlock_critical_section( evtd_instance->cs );

            xi_state_t result = xi_evtd_execute_handle( handle );

            XI_SAFE_FREE( tmp );

            if ( xi_state_is_fatal( result ) == 1 )
            {
                xi_debug_logger( "error while processing timed events" );
                xi_evtd_stop( evtd_instance );
            }

            xi_lock_critical_section( evtd_instance->cs );
        }
        else
        {
#ifdef XI_DEUBG_OUTPUT_EVENT_SYSTEM
            xi_debug_format( "[next key execution time: %" SCNuPTR
                             "\tcurrent time: %" SCNuPTR "]",
                             tmp->key, evtd_instance->current_step );
#endif
            break;
        }
    }

    xi_unlock_critical_section( evtd_instance->cs );

#ifdef XI_DEUBG_OUTPUT_EVENT_SYSTEM
    xi_debug_logger( "[enqueued events]" );
#endif

    /* execute all handlers in call_queue */
    while ( xi_evtd_single_step( evtd_instance, new_step ) )
        ;

    xi_lock_critical_section( evtd_instance->cs );
    /* here we can call the on_empty handler
     * watch out, handler is called only once and
     * it is disposed after that */
    if ( ( 0 == evtd_instance->time_events_container->elem_no ) &&
         ( evtd_instance->on_empty.handle_type != XI_EVENT_HANDLE_UNSET ) )
    {
        xi_debug_logger( "calling on_empty_handler" );

        xi_unlock_critical_section( evtd_instance->cs );
        xi_evtd_execute_handle( &evtd_instance->on_empty );
        xi_lock_critical_section( evtd_instance->cs );

        xi_dispose_handle( &evtd_instance->on_empty );
    }

    xi_unlock_critical_section( evtd_instance->cs );
}

uint8_t xi_evtd_dispatcher_continue( xi_evtd_instance_t* instance )
{
    return instance != NULL && instance->stop != 1;
}

uint8_t xi_evtd_all_continue( xi_evtd_instance_t** event_dispatchers, uint8_t num_evtds )
{
    uint8_t all_continue = 1;

    uint8_t it_evtd = 0;
    for ( ; it_evtd < num_evtds && all_continue; ++it_evtd )
    {
        all_continue = xi_evtd_dispatcher_continue( event_dispatchers[it_evtd] );
    }

    return all_continue;
}

xi_state_t xi_evtd_update_event_on_fd( xi_evtd_instance_t* instance,
                                       xi_vector_t* container,
                                       xi_fd_t fd )
{
    assert( instance != 0 );
    xi_lock_critical_section( instance->cs );

    xi_vector_index_type_t id = xi_vector_find(
        container, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_IPTR( fd ) ), &xi_evtd_cmp_fd );

    if ( id != -1 )
    {
        xi_evtd_fd_tuple_t* tuple =
            ( xi_evtd_fd_tuple_t* )container->array[id].selector_t.ptr_value;

        /* save the handle to execute */
        xi_event_handle_t to_exec = tuple->handle;

        /* set the default one if fd type socket */
        if ( XI_EVTD_FD_TYPE_SOCKET == tuple->fd_type )
        {
            tuple->event_type = XI_EVENT_WANT_READ; // default
            tuple->handle     = tuple->read_handle;
        }

        /* execute previously saved handle
         * we save the handle because the tuple->handle
         * may be overrided within the handle execution
         * so we don't won't to override that again */
        xi_unlock_critical_section( instance->cs );

        xi_evtd_execute_handle( &to_exec );

        xi_lock_critical_section( instance->cs );
    }
    else
    {
        xi_evtd_stop( instance );

        xi_unlock_critical_section( instance->cs );

        return XI_FD_HANDLER_NOT_FOUND;
    }

    xi_unlock_critical_section( instance->cs );
    return XI_STATE_OK;
}


xi_state_t xi_evtd_update_event_on_socket( xi_evtd_instance_t* instance, xi_fd_t fd )
{
    return xi_evtd_update_event_on_fd( instance, instance->handles_and_socket_fd, fd );
}

xi_state_t xi_evtd_update_event_on_file( xi_evtd_instance_t* instance, xi_fd_t fd )
{
    return xi_evtd_update_event_on_fd( instance, instance->handles_and_file_fd, fd );
}

void xi_evtd_stop( xi_evtd_instance_t* instance )
{
    assert( instance != 0 );

    instance->stop = 1;
}

uint8_t xi_evtd_update_file_fd_events( xi_evtd_instance_t* const event_dispatcher )
{
    uint8_t was_there_an_event = 0;
    xi_vector_index_type_t i   = 0;

    for ( i = 0; i < event_dispatcher->handles_and_file_fd->elem_no; ++i )
    {
        xi_evtd_fd_tuple_t* tuple =
            ( xi_evtd_fd_tuple_t* )event_dispatcher->handles_and_file_fd->array[i]
                .selector_t.ptr_value;

        assert( XI_EVTD_FD_TYPE_FILE == tuple->fd_type );

        if ( ( tuple->event_type & XI_EVENT_WANT_READ ) > 0 )
        {
            xi_evtd_update_event_on_file( event_dispatcher, tuple->fd );
            was_there_an_event |= 1;
        }
        else if ( ( tuple->event_type & XI_EVENT_WANT_WRITE ) > 0 )
        {
            xi_evtd_update_event_on_file( event_dispatcher, tuple->fd );
            was_there_an_event |= 1;
        }
        else if ( ( tuple->event_type & XI_EVENT_ERROR ) > 0 )
        {
            xi_evtd_update_event_on_file( event_dispatcher, tuple->fd );
            was_there_an_event |= 1;
        }
    }

    return was_there_an_event;
}

xi_state_t
xi_evtd_get_time_of_earliest_event( xi_evtd_instance_t* instance, xi_time_t* out_timeout )
{
    assert( NULL != instance );
    assert( NULL != out_timeout );
    assert( NULL != instance->time_events_container );

    xi_state_t ret_state = XI_ELEMENT_NOT_FOUND;

    xi_lock_critical_section( instance->cs );

    if ( 0 != instance->time_events_container->elem_no )
    {
        xi_time_event_t* elem = xi_time_event_peek_top( instance->time_events_container );
        *out_timeout          = elem->time_of_execution;
        ret_state             = XI_STATE_OK;
    }

    xi_unlock_critical_section( instance->cs );

    return ret_state;
}
