/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#include <socket.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "xi_io_sl_layer.h"
#include "xi_io_sl_layer_state.h"
#include "xi_allocator.h"
#include "xi_err.h"
#include "xi_macros.h"
#include "xi_debug.h"
#include "xi_layer_api.h"
#include "xi_common.h"
#include "xi_connection_data.h"
#include "xi_coroutine.h"
#include "xi_event_dispatcher_api.h"
#include "xi_globals.h"

#ifdef __cplusplus
extern "C" {
#endif

xi_state_t xi_io_sl_layer_push( void* context, void* data, xi_state_t state )
{
    XI_UNUSED( state );

    xi_io_sl_layer_state_t* layer_data =
        ( xi_io_sl_layer_state_t* )XI_THIS_LAYER( context )->user_data;
    xi_data_desc_t* buffer               = ( xi_data_desc_t* )data;
    xi_evtd_instance_t* event_dispatcher = XI_CONTEXT_DATA( context )->evtd_instance;

    if ( layer_data == 0 )
    {
        xi_debug_logger( "layer_data == 0" );

        /* if connection has been broken we have to remember about releasing
         * the memory */
        xi_free_desc( &buffer );

        return XI_LAYER_STATE_ERROR;
    }

    /* check if the layer is processing any request */
    if ( XI_CR_IS_RUNNING( layer_data->layer_cs ) )
    {
        /* means that we have entered in the middle of asynch operation
         * current strategy: try "next time"
         * @TODO extend the register fd to be able
         * to add items to the registration queue */
        {
            xi_evtd_execute_in(
                event_dispatcher,
                xi_make_handle( &xi_io_sl_layer_push, context, data, state ), 1 );
        }
    }

    /* process request */
    XI_CR_START( layer_data->layer_cs );

    if ( buffer != 0 && buffer->capacity > 0 )
    {
        size_t left = buffer->capacity - buffer->curr_pos;

        if ( left == 0 )
        {
            xi_debug_logger( "nothing left to send!" );
        }

        do
        {
            int len = sl_Send( layer_data->socket_fd, buffer->data_ptr + buffer->curr_pos,
                               buffer->capacity - buffer->curr_pos, 0 );

            xi_debug_format( "written: %d", len );

            if ( len < 0 )
            {
                int errval = errno;

                if ( errval == EAGAIN ) /* that can happen */
                {
                    xi_debug_logger( "EAGAIN...." );
                    XI_CR_EXIT( layer_data->layer_cs,
                                xi_evtd_continue_when_evt_on_socket(
                                    event_dispatcher, XI_EVENT_WANT_WRITE,
                                    xi_make_handle( &xi_io_sl_layer_push, context, data,
                                                    XI_STATE_OK ),
                                    layer_data->socket_fd ) );
                }

                xi_debug_printf( "error writing: errno = %d", errval );

                xi_free_desc( &buffer );

                XI_CR_EXIT( layer_data->layer_cs, XI_LAYER_STATE_ERROR );
            }

            if ( len == 0 )
            {
                xi_debug_logger( "connection reset by peer" );
                XI_CR_EXIT( layer_data->layer_cs,
                            XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(
                                context, 0, XI_LAYER_STATE_ERROR ) );
            }

            buffer->curr_pos += len;
            left = buffer->capacity - buffer->curr_pos;
        } while ( left > 0 );
    }

    xi_debug_logger( "exit...." );

    xi_free_desc( &buffer );

    XI_CR_EXIT( layer_data->layer_cs, XI_STATE_OK );

    XI_CR_END();

    return XI_STATE_OK;
}

xi_state_t xi_io_sl_layer_pull( void* context, void* data, xi_state_t in_out_state )
{
    XI_UNUSED( in_out_state );

    xi_debug_logger( "[xi_io_sl_layer_pull]" );

    xi_io_sl_layer_state_t* layer_data = XI_THIS_LAYER( context )->user_data;

    char* data_buffer           = 0;
    xi_data_desc_t* buffer_desc = 0;

    if ( layer_data == 0 )
    {
        if ( data ) /* let's clean the memory */
        {
            buffer_desc = ( xi_data_desc_t* )data;
            data_buffer = buffer_desc->data_ptr;

            assert( data_buffer != 0 );

            XI_SAFE_FREE( data_buffer );
            XI_SAFE_FREE( buffer_desc );
        }

        XI_PROCESS_PULL_ON_NEXT_LAYER( context, data, XI_LAYER_STATE_ERROR );
        return XI_LAYER_STATE_ERROR;
    }

    if ( data ) /* let's reuse already allocated buffer */
    {
        buffer_desc = ( xi_data_desc_t* )data;
        memset( buffer_desc->data_ptr, 0, XI_IO_BUFFER_SIZE );

        assert( buffer_desc->capacity == XI_IO_BUFFER_SIZE ); /* sanity check */

        data_buffer = buffer_desc->data_ptr;

        assert( data_buffer != 0 );

        buffer_desc->curr_pos = 0;
        buffer_desc->length   = 0;
    }
    else
    {
        /* @TODO might be usefull to get back to the solution with
         * one block of the memory for whole data_desc + data_buffer */
        data_buffer = xi_alloc( XI_IO_BUFFER_SIZE );
        XI_CHECK_MEMORY( data_buffer );
        memset( data_buffer, 0, XI_IO_BUFFER_SIZE );

        buffer_desc = xi_alloc( sizeof( xi_data_desc_t ) );
        XI_CHECK_MEMORY( buffer_desc );
        memset( buffer_desc, 0, sizeof( xi_data_desc_t ) );

        buffer_desc->data_ptr = data_buffer;
        buffer_desc->capacity = XI_IO_BUFFER_SIZE;
        buffer_desc->length   = 0;
    }

    int len =
        sl_Recv( layer_data->socket_fd, buffer_desc->data_ptr, buffer_desc->capacity, 0 );

    xi_debug_format( "read: %d", len );

    if ( len < 0 )
    {
        int errval = errno;

        xi_debug_format( "error reading: errno = %d", errval );

        if ( errval == EAGAIN ) /* register for an event on that socket */
        {
            {
                return xi_evtd_continue_when_evt_on_socket(
                    XI_CONTEXT_DATA( context )->evtd_instance, XI_EVENT_WANT_READ,
                    xi_make_handle( &xi_io_sl_layer_pull, context, buffer_desc,
                                    in_out_state ),
                    layer_data->socket_fd );
            }
        }

        goto err_handling;
    }

    if ( len == 0 ) /* we've been disconnected, so let's roll down */
    {
        xi_debug_logger( "connection reset by peer" );
        XI_SAFE_FREE( data_buffer );
        XI_SAFE_FREE( buffer_desc );
        return XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER( context, 0,
                                                          XI_LAYER_STATE_ERROR );
    }

    buffer_desc->length   = len;
    buffer_desc->curr_pos = 0;

    return XI_PROCESS_PULL_ON_NEXT_LAYER( context, ( void* )buffer_desc, XI_STATE_OK );


err_handling:
    XI_SAFE_FREE( data_buffer );
    XI_SAFE_FREE( buffer_desc );

    return XI_PROCESS_PULL_ON_NEXT_LAYER( context, 0, XI_LAYER_STATE_ERROR );
}

xi_state_t xi_io_sl_layer_close( void* context, void* data, xi_state_t in_out_state )
{
    XI_UNUSED( in_out_state );

    return XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER( context, data, XI_STATE_OK );
}

xi_state_t
xi_io_sl_layer_close_externally( void* context, void* data, xi_state_t in_out_state )
{
    XI_UNUSED( in_out_state );
    XI_UNUSED( data );

    xi_io_sl_layer_state_t* layer_data = XI_THIS_LAYER( context )->user_data;

    /* @TODO we can try to avoid that via setting some state within the layer so
     * incoming requets shall not be processed, let's keep it for now it's
     * not that important at the moment */
    if ( layer_data == 0 )
    {
        return XI_LAYER_STATE_ERROR;
    }

    xi_state_t ret = XI_STATE_OK;

    /* if( shutdown( layer_data->socket_fd, SHUT_RDWR ) == -1 )
     * {
     *   xi_set_error( XI_SOCKET_SHUTDOWN_ERROR ); */
    sl_Close( layer_data->socket_fd ); /* just in case */
    ret = XI_LAYER_STATE_ERROR;
    goto err_handling;
    /* } */

    /* close the connection & the socket */
    if ( sl_Close( layer_data->socket_fd ) == -1 )
    {
        xi_set_error( XI_SOCKET_CLOSE_ERROR );
        ret = XI_LAYER_STATE_ERROR;
        goto err_handling;
    }

err_handling:
    /* unregister the fd */
    xi_evtd_unregister_socket_fd( XI_CONTEXT_DATA( context )->evtd_instance,
                                  layer_data->socket_fd );
    /* cleanup the memory */
    XI_SAFE_FREE( XI_THIS_LAYER( context )->user_data );

    return XI_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER( context, data, ret );
}

xi_state_t xi_io_sl_layer_init( void* context, void* data, xi_state_t in_out_state )
{
    XI_UNUSED( data );
    XI_UNUSED( in_out_state );

    /* PRECONDITIONS */
    assert( context != 0 );
    assert( XI_THIS_LAYER( context )->user_data == 0 );

    xi_debug_logger( "[xi_io_sl_layer_init]" );

    xi_layer_t* layer                  = ( xi_layer_t* )XI_THIS_LAYER( context );
    xi_io_sl_layer_state_t* layer_data = xi_alloc( sizeof( xi_io_sl_layer_state_t ) );
    memset( layer_data, 0, sizeof( xi_io_sl_layer_state_t ) );

    XI_CHECK_MEMORY( layer_data );

    layer->user_data = ( void* )layer_data;

    xi_debug_logger( "Creating socket..." );

    layer_data->socket_fd = sl_Socket( AF_INET, SOCK_STREAM, 0 );

    if ( layer_data->socket_fd == -1 )
    {
        xi_debug_logger( "Socket creation [failed]" );
        xi_set_error( XI_SOCKET_INITIALIZATION_ERROR );
        /* return 0; */
    }

    xi_debug_logger( "Setting socket non blocking behaviour..." );

    /* int flags = fcntl( layer_data->socket_fd, F_GETFL, 0 ); */

    /* if( flags == -1 )
     * {
     *    xi_debug_format( "socket_fd=%d", layer_data->socket_fd );
     *    xi_debug_logger( "Socket non blocking behaviour [failed]" );
     *    xi_set_error( XI_SOCKET_INITIALIZATION_ERROR );
     *    goto err_handling;
     * } */

    /* xi_debug_format( "socket_fd=%d", layer_data->socket_fd );
     * if( fcntl( layer_data->socket_fd, F_SETFL, flags | O_NONBLOCK ) == -1 )
     * {
     *    xi_debug_format( "socket_fd=%d", layer_data->socket_fd );
     *    xi_debug_logger( "Socket non blocking behaviour [failed]" );
     *    xi_set_error( XI_SOCKET_INITIALIZATION_ERROR );
     * goto err_handling;
     * } */

    xi_debug_logger( "Socket creation [ok]" );

    /* POSTCONDITIONS */
    assert( layer->user_data != 0 );
    assert( layer_data->socket_fd != -1 );

    return XI_PROCESS_CONNECT_ON_THIS_LAYER( context, data, XI_STATE_OK );

err_handling:
    /* cleanup the memory */
    if ( layer_data )
    {
        sl_Close( layer_data->socket_fd );
    }
    if ( layer->user_data )
    {
        XI_SAFE_FREE( layer->user_data );
    }

    return XI_PROCESS_CONNECT_ON_THIS_LAYER( context, data, XI_LAYER_STATE_ERROR );
}

xi_state_t xi_io_sl_layer_connect( void* context, void* data, xi_state_t in_out_state )
{
    XI_UNUSED( data );
    XI_UNUSED( in_out_state );

    static uint16_t layer_cs = 0; /* local coroutine prereserved state */

    xi_connection_data_t* connection_data = ( xi_connection_data_t* )data;
    xi_layer_t* layer                     = ( xi_layer_t* )XI_THIS_LAYER( context );
    xi_io_sl_layer_state_t* layer_data    = ( xi_io_sl_layer_state_t* )layer->user_data;
    xi_evtd_instance_t* event_dispatcher  = XI_CONTEXT_DATA( context )->evtd_instance;
    int errval                            = 0;

    XI_CR_START( layer_cs );

    xi_debug_format( "Connecting layer [%d] to the endpoint", layer->xi_layer_type_id );

    /* socket specific data */
    struct sockaddr_in name;

#if 0
    struct hostent* hostinfo;
    /* get the hostaddress */
    hostinfo = gethostbyname( connection_data->host );

    /* if null it means that the address has not been founded */
    if( hostinfo == NULL )
    {
        xi_debug_logger( "Getting Host by name [failed]" );
        xi_set_error( XI_SOCKET_GETHOSTBYNAME_ERROR );
        goto err_handling;
    }

    /* xi_debug_logger( "Getting Host by name [ok]" ); */
#endif

    /* set the address and the port for further connection attempt */
    memset( &name, 0, sizeof( struct sockaddr_in ) );
    name.sin_family      = AF_INET;
    name.sin_addr.s_addr = htonl( ( UINT32 )1079906923 );
    name.sin_port        = htons( ( UINT16 )connection_data->port );

    xi_debug_logger( "Connecting to the endpoint..." );

    {
        xi_evtd_register_socket_fd(
            event_dispatcher, layer_data->socket_fd,
            xi_make_handle( &xi_io_sl_layer_pull, context, 0, XI_STATE_OK ) );
    }

    if ( sl_Connect( layer_data->socket_fd, ( struct sockaddr* )&name,
                     sizeof( struct sockaddr ) ) == -1 )
    {
        errval = errno;

        if ( errval != EINPROGRESS )
        {
            xi_debug_printf( "errno: %d", errval );
            xi_debug_logger( "Connecting to the endpoint [failed]" );
            xi_set_error( XI_SOCKET_CONNECTION_ERROR );
            goto err_handling;
        }
        else
        {
            xi_evtd_continue_when_evt_on_socket( event_dispatcher, XI_EVENT_WANT_CONNECT,
                                                 xi_make_handle( &xi_io_sl_layer_connect,
                                                                 ( void* )context, data,
                                                                 XI_STATE_OK ),
                                                 layer_data->socket_fd );

            XI_CR_YIELD( layer_cs, XI_STATE_OK ); // return here whenever we can write
        }
    }

    XI_CR_EXIT( layer_cs,
                XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, data, XI_STATE_OK ) );

err_handling:
    /* cleanup the memory */
    xi_evtd_unregister_socket_fd( event_dispatcher, layer_data->socket_fd );

    if ( layer_data )
    {
        sl_Close( layer_data->socket_fd );
    }
    if ( layer->user_data )
    {
        XI_SAFE_FREE( layer->user_data );
    }

    XI_CR_EXIT( layer_cs, XI_PROCESS_CONNECT_ON_NEXT_LAYER(
                              XI_THIS_LAYER( context ), data, XI_LAYER_STATE_ERROR ) );

    XI_CR_END();
}

#ifdef __cplusplus
}
#endif
