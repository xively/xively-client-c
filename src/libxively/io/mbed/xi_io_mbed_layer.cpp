/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "xi_io_mbed_layer.h"
#include "xi_io_mbed_layer_state.h"
#include "xi_helpers.h"
#include "xi_allocator.h"
#include "xi_debug.h"
#include "xi_err.h"
#include "xi_macros.h"
#include "xi_globals.h"
#include "xi_layer_api.h"
#include "xi_common.h"
#include "xi_connection_data.h"

extern "C" {

xi_state_t xi_io_mbed_layer_push(
      struct layer_connectivity_s* context
    , const void* data
    , const layer_hint_t hint )
{
    /* extract the layer specific data */
    xi_io_mbed_layer_state_t* layer_data  = ( xi_io_mbed_layer_state_t* ) context->self->user_data;
    const xi_data_desc_t* buffer    = ( const xi_data_desc_t* ) data;

    if( buffer != 0 && buffer->capacity > 0 )
    {
        xi_debug_format( "sending: [%s]", buffer->data_ptr );
        int len = layer_data->socket_ptr->send_all( ( char* ) buffer->data_ptr, buffer->capacity );
        xi_debug_format( "sent: %d bytes", len );

        if( len < buffer->capacity )
        {
            xi_set_error( XI_SOCKET_WRITE_ERROR );
            return XI_LAYER_STATE_ERROR;
        }
    }

    return XI_STATE_OK;
}

xi_state_t xi_io_mbed_layer_pull(
      struct layer_connectivity_s* context
    , const void* data
    , const layer_hint_t hint )
{
    /* extract the layer specific data */
    xi_io_mbed_layer_state_t* layer_data  = ( xi_io_mbed_layer_state_t* ) context->self->user_data;

    XI_UNUSED( hint );

    xi_data_desc_t* buffer = 0;

    if( data )
    {
        buffer = ( xi_data_desc_t* ) data;
    }
    else
    {
        static char data_buffer[ 32 ];
        memset( data_buffer, 0, sizeof( data_buffer ) );
        static xi_data_desc_t buffer_desc = { data_buffer, sizeof( data_buffer ), 0, 0 };
        buffer = &buffer_desc;
    }

    xi_state_t state = XI_STATE_OK;

    do
    {
        memset( buffer->data_ptr, 0, buffer->capacity );
        buffer->length = layer_data->socket_ptr->receive( buffer->data_ptr, buffer->capacity - 1 );

        xi_debug_format( "received: %d", buffer->length );

        buffer->data_ptr[ buffer->length ] = '\0'; // put guard
        buffer->curr_pos = 0;
        state = XI_PROCESS_PULL_ON_NEXT_LAYER( context->self, ( void* ) buffer, LAYER_HINT_MORE_DATA );
    } while( state == XI_STATE_WANT_READ );

    return XI_STATE_OK;
}

xi_state_t xi_io_mbed_layer_close(
    struct layer_connectivity_s* context )
{
    /* extract the layer specific data */
    xi_io_mbed_layer_state_t* layer_data
        = ( xi_io_mbed_layer_state_t* ) context->self->user_data;

    xi_debug_logger( "closing socket..." );

    /* close the connection & the socket */
    if( layer_data->socket_ptr->close() == -1 )
    {
        xi_debug_logger( "error closing socket..." );
        xi_set_error( XI_SOCKET_CLOSE_ERROR );
        goto err_handling;
    }

    /* safely destroy the object */
    if ( layer_data && layer_data->socket_ptr )
    {
        xi_debug_logger( "deleting socket..." );

        delete layer_data->socket_ptr;
        layer_data->socket_ptr = 0;
    }

    if( layer_data ) { XI_SAFE_FREE( layer_data ); }

    return XI_STATE_OK;

    /* cleanup the memory */
err_handling:
    /* safely destroy the object */
    if ( layer_data && layer_data->socket_ptr )
    {
        delete layer_data->socket_ptr;
        layer_data->socket_ptr = 0;
    }

    if( layer_data ) { XI_SAFE_FREE( layer_data ); }

    return XI_LAYER_STATE_ERROR;
}

xi_state_t xi_io_mbed_layer_close_externally( struct layer_connectivity_s* context )
{
    return XI_PROCESS_CLOSE_ON_NEXT_LAYER( context->self );
}

xi_state_t xi_io_mbed_layer_init(
      struct layer_connectivity_s* context
    , const void* data
    , const layer_hint_t hint )
{
    xi_layer_t* layer                     = context->self;
    xi_io_mbed_layer_state_t* layer_data  = 0;

    /* allocation of the socket connector */
    TCPSocketConnection* socket_ptr
        = new TCPSocketConnection();
    XI_CHECK_MEMORY( socket_ptr );

    /* set the timeout for non-blocking operations */
    socket_ptr->set_blocking( false, xi_globals.network_timeout );

    /* allocate memory for the mbed data specific structure */
    layer_data = ( xi_io_mbed_layer_state_t* )
            xi_alloc( sizeof( xi_io_mbed_layer_state_t ) );
    XI_CHECK_MEMORY( layer_data );

    layer_data->socket_ptr = socket_ptr;

    /* assign the layer specific data */
    layer->user_data = ( void* ) layer_data;

    return XI_STATE_OK;

err_handling:
    /* safely destroy the object */
    if ( layer_data && layer_data->socket_ptr )
    {
        delete layer_data->socket_ptr;
        layer_data->socket_ptr = 0;
    }

    if( layer_data ) { XI_SAFE_FREE( layer_data ); }

    return XI_LAYER_STATE_ERROR;

}

xi_state_t xi_io_mbed_layer_connect (
      struct layer_connectivity_s* context
    , const void* data
    , const layer_hint_t hint )
{
    xi_layer_t* layer                     = context->self;
    xi_io_mbed_layer_state_t* layer_data  = ( xi_io_mbed_layer_state_t* ) layer->user_data;
    xi_connection_data_t* connection_data = ( xi_connection_data_t* ) data;

    /* try to connect */
    int s = layer_data->socket_ptr->connect(
          connection_data->address
        , connection_data->port );

    /* check if not failed */
    if( s == -1 )
    {
        xi_set_error( XI_SOCKET_CONNECTION_ERROR );
        goto err_handling;
    }

    return XI_STATE_OK;

err_handling:
    /* safely destroy the object */
    if ( layer_data && layer_data->socket_ptr )
    {
        delete layer_data->socket_ptr;
        layer_data->socket_ptr = 0;
    }

    if( layer_data ) { XI_SAFE_FREE( layer_data ); }

    return XI_LAYER_STATE_ERROR;
}

} /* extern "C" */
