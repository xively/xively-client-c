/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "xi_io_microchip_layer.h"
#include "xi_io_microchip_layer_state.h"

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


/* This is temporary until we have a better implemetnation for Connect
 * that tracks a socket's connection status */
#include "StackTsk.h"


#ifdef __cplusplus
extern "C" {
#endif

xi_state_t xi_io_microchip_layer_push( void* context, void* data, xi_state_t state )
{
    XI_UNUSED( state );

    xi_io_microchip_layer_state_t* layer_data =
        ( xi_io_microchip_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    xi_data_desc_t* buffer = ( xi_data_desc_t* )data;

    xi_evtd_instance_t* event_dispatcher = XI_CONTEXT_DATA( context )->evtd_instance;

    /* check if the layer has been disconnected */
    if ( NULL == layer_data )
    {
        xi_debug_logger( "Layer Data in microchip_layer_push is NULL" );

        /* if connection has been broken we have to remember about releasing the
         *  memory */
        xi_free_desc( &buffer );

        return XI_STATE_OK;
    }

    /* check if the layer is processing any request */
    if ( XI_CR_IS_RUNNING( layer_data->layer_push_cs ) )
    {
        assert( 0 );
        /* means that there was a request in the middle of processing
         * other request must not happen */
    }

    /* process request */
    XI_CR_START( layer_data->layer_push_cs );

    if ( ( NULL != buffer ) && ( buffer->capacity > 0 ) )
    {
        size_t left = buffer->capacity - buffer->curr_pos;

        do
        {
            BOOL bIsConnected = TCPIsConnected( layer_data->socket_fd );
            if ( FALSE == bIsConnected )
            {
                xi_debug_logger( "connection reset by peer" );
                xi_free_desc( &buffer );
                XI_CR_EXIT( layer_data->layer_push_cs,
                            XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(
                                context, 0, XI_CONNECTION_RESET_BY_PEER_ERROR ) );
            }
            else if ( !TCPIsPutReady( layer_data->socket_fd ) )
            {
                /* that can happen */
                XI_CR_EXIT( layer_data->layer_push_cs,
                            xi_evtd_continue_when_evt_on_socket(
                                event_dispatcher, XI_EVENT_WANT_WRITE,
                                xi_make_handle( &xi_io_microchip_layer_push, context,
                                                data, XI_STATE_WANT_WRITE ),
                                layer_data->socket_fd ) );
            }

            WORD len =
                TCPPutArray( layer_data->socket_fd, buffer->data_ptr + buffer->curr_pos,
                             buffer->capacity - buffer->curr_pos );

            xi_debug_format( "written: %d bytes", len );

            if ( 0 == len )
            {
                /* treat this like EAGAIN */
                XI_CR_EXIT( layer_data->layer_push_cs,
                            xi_evtd_continue_when_evt_on_socket(
                                event_dispatcher, XI_EVENT_WANT_WRITE,
                                xi_make_handle( &xi_io_microchip_layer_push, context,
                                                data, XI_STATE_WANT_WRITE ),
                                layer_data->socket_fd ) );
            }

            buffer->curr_pos += len;
            left = buffer->capacity - buffer->curr_pos;
        } while ( left > 0 );
    }

    xi_free_desc( &buffer );

    XI_CR_EXIT( layer_data->layer_push_cs,
                XI_PROCESS_PUSH_ON_NEXT_LAYER( context, 0, XI_STATE_WRITTEN ) );

    XI_CR_END();

    return XI_STATE_OK;
}

xi_state_t
xi_io_microchip_layer_pull( void* context, void* data, xi_state_t in_out_state )
{
    xi_io_microchip_layer_state_t* layer_data =
        ( xi_io_microchip_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    xi_evtd_instance_t* event_dispatcher = XI_CONTEXT_DATA( context )->evtd_instance;

    unsigned char* data_buffer  = NULL;
    xi_data_desc_t* buffer_desc = NULL;
    int len                     = 0;

    if ( NULL == layer_data )
    {
        if ( data ) /* let's clean the memory */
        {
            buffer_desc = ( xi_data_desc_t* )data;
            xi_free_desc( &buffer_desc );
        }

        return XI_STATE_OK;
    }

    if ( data ) /* let's reuse already allocated buffer */
    {
        buffer_desc = ( xi_data_desc_t* )data;

        memset( buffer_desc->data_ptr, 0, XI_IO_BUFFER_SIZE );
        assert( buffer_desc->capacity == XI_IO_BUFFER_SIZE ); /* sanity check */

        data_buffer = buffer_desc->data_ptr;

        assert( NULL != data_buffer );

        buffer_desc->curr_pos = 0;
        buffer_desc->length   = 0;
    }
    else
    {
        buffer_desc = xi_make_empty_desc_alloc( XI_IO_BUFFER_SIZE );
        XI_CHECK_MEMORY( buffer_desc, in_out_state );
    }

    if ( TCPWasReset( layer_data->socket_fd ) ||
         !TCPIsConnected( layer_data->socket_fd ) )
    {
        xi_debug_logger( "connection reset by peer" );
        xi_free_desc( &buffer_desc );
        XI_CR_EXIT( layer_data->layer_push_cs,
                    XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(
                        context, 0, XI_CONNECTION_RESET_BY_PEER_ERROR ) );
    }

    len = TCPGetArray( layer_data->socket_fd, buffer_desc->data_ptr,
                       buffer_desc->capacity );

    xi_debug_format( "read: %d bytes", len );

    if ( len < 0 )
    {
        if ( TCPWasReset( layer_data->socket_fd ) ||
             !TCPIsConnected( layer_data->socket_fd ) )
        {
            xi_debug_logger( "connection reset by peer" );
            xi_free_desc( &buffer_desc );
            XI_CR_EXIT( layer_data->layer_push_cs,
                        XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(
                            context, 0, XI_CONNECTION_RESET_BY_PEER_ERROR ) );
        }
        else
        {
            xi_evtd_continue_when_evt_on_socket(
                event_dispatcher, XI_EVENT_WANT_READ,
                xi_make_handle( &xi_io_microchip_layer_pull, context, buffer_desc,
                                in_out_state ),
                layer_data->socket_fd );

            return XI_STATE_WANT_READ;
        }

        goto err_handling;
    }
    else if ( NULL == len )
    {
        xi_evtd_continue_when_evt_on_socket( event_dispatcher, XI_EVENT_WANT_READ,
                                             xi_make_handle( &xi_io_microchip_layer_pull,
                                                             context, buffer_desc,
                                                             in_out_state ),
                                             layer_data->socket_fd );

        return XI_STATE_WANT_READ;
    }

    buffer_desc->length   = len;
    buffer_desc->curr_pos = 0;

    return XI_PROCESS_PULL_ON_NEXT_LAYER( context, ( void* )buffer_desc, XI_STATE_OK );

err_handling:
    if ( buffer_desc )
    {
        xi_free_desc( &buffer_desc );
    }

    return XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER( context, 0, in_out_state );
}

xi_state_t
xi_io_microchip_layer_close( void* context, void* data, xi_state_t in_out_state )
{
    return XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER( context, data, in_out_state );
}

xi_state_t xi_io_microchip_layer_close_externally( void* context,
                                                   void* data,
                                                   xi_state_t in_out_state )
{
    XI_UNUSED( in_out_state );

    xi_io_microchip_layer_state_t* layer_data =
        ( xi_io_microchip_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    /* @TODO we can try to avoid that via setting some state within the layer so
     * incoming requets shall not be processed, let's keep it for now it's
     * not that important at the moment */
    if ( NULL == layer_data )
    {
        return XI_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER( context, data, in_out_state );
    }

    /* close the connection & the socket */
    TCPDisconnect( layer_data->socket_fd );
    TCPClose( layer_data->socket_fd );

    /* unregister the fd */
    xi_evtd_unregister_socket_fd( XI_CONTEXT_DATA( context )->evtd_instance,
                                  layer_data->socket_fd );

    /* cleanup the memory */
    XI_SAFE_FREE( XI_THIS_LAYER( context )->user_data );

    return XI_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER( context, data, in_out_state );
}

xi_state_t
xi_io_microchip_layer_init( void* context, void* data, xi_state_t in_out_state )
{
    XI_UNUSED( data );
    XI_UNUSED( in_out_state );

    /* PRECONDITIONS */
    assert( NULL != context );
    assert( NULL == XI_THIS_LAYER( context )->user_data );

    xi_layer_t* layer = ( xi_layer_t* )XI_THIS_LAYER( context );

    XI_ALLOC( xi_io_microchip_layer_state_t, layer_data, in_out_state );

    layer->user_data = ( void* )layer_data;

    return XI_PROCESS_CONNECT_ON_THIS_LAYER( context, data, XI_STATE_OK );

err_handling:
    /* cleanup the memory */
    if ( layer->user_data )
    {
        XI_SAFE_FREE( layer->user_data );
    }

    return XI_PROCESS_CONNECT_ON_THIS_LAYER( context, data, in_out_state );
}

xi_state_t
xi_io_microchip_layer_connect( void* context, void* data, xi_state_t in_out_state )
{
    XI_UNUSED( data );
    XI_UNUSED( in_out_state );

    xi_connection_data_t* connection_data = ( xi_connection_data_t* )data;
    xi_layer_t* layer = ( xi_layer_t* )XI_THIS_LAYER( context );
    xi_io_microchip_layer_state_t* layer_data =
        ( xi_io_microchip_layer_state_t* )layer->user_data;
    xi_evtd_instance_t* event_dispatcher = XI_CONTEXT_DATA( context )->evtd_instance;

    XI_CR_START( layer_data->layer_connect_cs )

    layer_data->socket_fd =
        TCPOpen( ( DWORD )( PTR_BASE )connection_data->host,
                 TCP_OPEN_ROM_HOST /*, TCP_OPEN_RAM_HOST */
                 ,
                 connection_data->port,
                 TCP_PURPOSE_GENERIC_TCP_CLIENT ); /*, TCP_PURPOSE_DEFAULT ); */

    if ( INVALID_SOCKET == layer_data->socket_fd )
    {
        XI_CR_EXIT( layer_data->layer_connect_cs,
                    XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, data,
                                                      XI_SOCKET_CONNECTION_ERROR ) );
    }

    /* Register the Socket FD with the event loop */
    xi_evtd_register_socket_fd(
        event_dispatcher, layer_data->socket_fd,
        xi_make_handle( &xi_io_microchip_layer_pull, context, 0, XI_STATE_OK ) );

    /* Request to be invoked again after connection state change */
    xi_evtd_continue_when_evt_on_socket( event_dispatcher, XI_EVENT_WANT_CONNECT,
                                         xi_make_handle( &xi_io_microchip_layer_connect,
                                                         ( void* )context, data,
                                                         XI_STATE_OK ),
                                         layer_data->socket_fd );

    /* Yield until we've connected */
    XI_CR_YIELD( layer_data->layer_connect_cs, XI_STATE_OK );

    /* pass the socket fd to the next layer
     * will be used via ssl layer or any other ssl layer
     * in order to provide asynchronious connect */
    XI_CR_EXIT( layer_data->layer_connect_cs,
                XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, &layer_data->socket_fd,
                                                  XI_STATE_OK ) );

    XI_CR_END()
}

#ifdef __cplusplus
}
#endif
