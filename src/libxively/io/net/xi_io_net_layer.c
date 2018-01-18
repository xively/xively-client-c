/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_io_net_layer.h"
#include "xi_io_net_layer_state.h"
#include "xi_bsp_io_net.h"

#include "xi_macros.h"
#include "xi_debug.h"
#include "xi_layer_api.h"
#include "xi_connection_data.h"
#include "xi_types.h"
#include "xi_coroutine.h"

#include "xi_io_timeouts.h"
#include "xi_globals.h"

xi_state_t xi_io_net_layer_connect( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    /* check the state before doing anything */
    if ( XI_STATE_OK != in_out_state )
    {
        return XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, NULL, in_out_state );
    }

    xi_io_net_layer_state_t* layer_data =
        ( xi_io_net_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    /* if layer_data is null on this stage it means internal error */
    if ( NULL == layer_data )
    {
        return XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, NULL, XI_INTERNAL_ERROR );
    }

    xi_bsp_io_net_state_t state = XI_BSP_IO_NET_STATE_OK;

    xi_connection_data_t* connection_data = ( xi_connection_data_t* )data;
    xi_evtd_instance_t* event_dispatcher  = XI_CONTEXT_DATA( context )->evtd_instance;

    XI_CR_START( layer_data->layer_connect_cs )

    xi_debug_format( "Connecting layer [%d] to the endpoint: %s:%hu",
                     XI_THIS_LAYER( context )->layer_type_id, connection_data->host,
                     connection_data->port );

    {
        xi_evtd_register_socket_fd(
            event_dispatcher, layer_data->socket,
            xi_make_handle( &xi_io_net_layer_pull, context, 0, XI_STATE_OK ) );

        xi_evtd_continue_when_evt_on_socket( event_dispatcher, XI_EVENT_WANT_CONNECT,
                                             xi_make_handle( &xi_io_net_layer_connect,
                                                             ( void* )context, data,
                                                             XI_STATE_OK ),
                                             layer_data->socket );
    }

    state = xi_bsp_io_net_connect( &layer_data->socket, connection_data->host,
                                   connection_data->port );

    XI_CHECK_CND_DBGMESSAGE( XI_BSP_IO_NET_STATE_OK != state, XI_SOCKET_CONNECTION_ERROR,
                             in_out_state, "Connecting to the endpoint [failed]" );

    // return here whenever we can write
    XI_CR_YIELD( layer_data->layer_connect_cs, XI_STATE_OK );

    state = xi_bsp_io_net_connection_check( layer_data->socket, connection_data->host,
                                            connection_data->port );

    XI_CHECK_CND_DBGMESSAGE( XI_BSP_IO_NET_STATE_OK != state, XI_SOCKET_GETSOCKOPT_ERROR,
                             in_out_state, "Error while calling getsockopt." );
    xi_debug_logger( "Connection successful!" );

    XI_CR_EXIT( layer_data->layer_connect_cs,
                XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, NULL, XI_STATE_OK ) );

err_handling:

    XI_CR_EXIT( layer_data->layer_connect_cs,
                XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, data, in_out_state ) );

    XI_CR_END();
}

xi_state_t xi_io_net_layer_init( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    if ( NULL == context )
    {
        return XI_INVALID_PARAMETER;
    }

    if ( NULL != XI_THIS_LAYER( context )->user_data )
    {
        return XI_FAILED_INITIALIZATION;
    }

    xi_layer_t* layer               = ( xi_layer_t* )XI_THIS_LAYER( context );
    xi_bsp_io_net_state_t bsp_state = XI_BSP_IO_NET_STATE_OK;

    XI_ALLOC( xi_io_net_layer_state_t, layer_data, in_out_state );

    layer->user_data = ( void* )layer_data;

    xi_debug_logger( "Creating socket..." );

    bsp_state = xi_bsp_io_net_create_socket( &layer_data->socket );

    XI_CHECK_CND_DBGMESSAGE( XI_BSP_IO_NET_STATE_OK != bsp_state,
                             XI_SOCKET_INITIALIZATION_ERROR, in_out_state,
                             "Socket creation [failed]" );

    xi_debug_logger( "Socket creation [ok]" );

    return XI_PROCESS_CONNECT_ON_THIS_LAYER( context, data, XI_STATE_OK );

err_handling:
    /* cleanup the memory */
    xi_bsp_io_net_close_socket( &layer_data->socket );

    if ( layer->user_data )
    {
        XI_SAFE_FREE( layer->user_data );
    }

    return XI_PROCESS_CONNECT_ON_THIS_LAYER( context, data, in_out_state );
}

xi_state_t xi_io_net_layer_push( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( in_out_state );

    xi_io_net_layer_state_t* layer_data =
        ( xi_io_net_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    xi_data_desc_t* buffer          = ( xi_data_desc_t* )data;
    size_t left                     = 0;
    int len                         = 0;
    xi_bsp_io_net_state_t bsp_state = XI_BSP_IO_NET_STATE_OK;

    /* check if the layer has been disconnected */
    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) || layer_data == NULL )
    {
        xi_debug_logger( "layer not operational" );
        xi_free_desc( &buffer );

        return XI_STATE_OK;
    }

    if ( buffer != 0 && buffer->capacity > 0 )
    {
        do
        {
            /* call bsp write */
            bsp_state = xi_bsp_io_net_write( layer_data->socket, &len,
                                             buffer->data_ptr + buffer->curr_pos,
                                             buffer->capacity - buffer->curr_pos );

            /* verify the state if it's an error or a need to wait */
            if ( XI_BSP_IO_NET_STATE_OK != bsp_state )
            {
                if ( XI_BSP_IO_NET_STATE_BUSY ==
                     bsp_state ) /* that can happen in asynch environments */
                {
                    /* mark the socket for wake-up call */
                    if ( 0 > xi_evtd_continue_when_evt_on_socket(
                                 XI_CONTEXT_DATA( context )->evtd_instance,
                                 XI_EVENT_WANT_WRITE,
                                 xi_make_handle( &xi_io_net_layer_push, context, data,
                                                 XI_STATE_WANT_WRITE ),
                                 layer_data->socket ) )
                    {
                        xi_debug_format( "given socket is not registered - [%d]",
                                         ( int )layer_data->socket );
                        return XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(
                            context, 0, XI_INTERNAL_ERROR );
                    }

                    /* this is not an error so we can leave the coroutine within this
                     * state */
                    xi_debug_format( "yield in write - [%d]", ( int )layer_data->socket );
                    return XI_STATE_OK;
                }
                else if ( XI_BSP_IO_NET_STATE_CONNECTION_RESET == bsp_state )
                {
                    xi_free_desc( &buffer );
                    xi_debug_logger( "connection reset" );
                    return XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(
                        context, 0, XI_CONNECTION_RESET_BY_PEER_ERROR );
                }
                else
                {
                    /* any other issue */
                    xi_debug_format( "error writing: BSP error code = %d\n",
                                     ( int )bsp_state );
                    xi_free_desc( &buffer );
                    return XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(
                        context, data, XI_SOCKET_WRITE_ERROR );
                }
            }

            buffer->curr_pos += len;
            left = buffer->capacity - buffer->curr_pos;
        } while ( left > 0 );
    }

    xi_debug_format( "%d bytes written", len );
    xi_free_desc( &buffer );

    return XI_PROCESS_PUSH_ON_NEXT_LAYER( context, 0, XI_STATE_WRITTEN );
}

xi_state_t xi_io_net_layer_pull( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    xi_io_net_layer_state_t* layer_data =
        ( xi_io_net_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    xi_data_desc_t* buffer_desc     = 0;
    int len                         = 0;
    xi_bsp_io_net_state_t bsp_state = XI_BSP_IO_NET_STATE_OK;

    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) || layer_data == NULL )
    {
        if ( data != NULL ) // let's clean the memory
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
        assert( buffer_desc->capacity == XI_IO_BUFFER_SIZE ); // sanity check

        buffer_desc->curr_pos = 0;
        buffer_desc->length   = 0;
    }
    else /* if there was no buffer we have to create new one */
    {
        buffer_desc = xi_make_empty_desc_alloc( XI_IO_BUFFER_SIZE );
        XI_CHECK_MEMORY( buffer_desc, in_out_state );
    }

    bsp_state = xi_bsp_io_net_read( layer_data->socket, &len, buffer_desc->data_ptr,
                                    buffer_desc->capacity );

    // xi_debug_format( "read: %d bytes", len );

    if ( XI_BSP_IO_NET_STATE_OK != bsp_state )
    {
        if ( XI_BSP_IO_NET_STATE_BUSY ==
             bsp_state ) /* register socket to get call when can read */
        {
            /* note for future bsp of select etc. this is the place in the interface of
             * event dispatcher where the socket is the identification value */
            xi_evtd_continue_when_evt_on_socket(
                XI_CONTEXT_DATA( context )->evtd_instance, XI_EVENT_WANT_READ,
                xi_make_handle( &xi_io_net_layer_pull, context, buffer_desc,
                                in_out_state ),
                layer_data->socket );

            return XI_STATE_WANT_READ;
        }

        if ( XI_BSP_IO_NET_STATE_CONNECTION_RESET == bsp_state )
        {
            /* connection reset by peer */
            xi_debug_logger( "connection reset by peer" );
            in_out_state = XI_CONNECTION_RESET_BY_PEER_ERROR;
            goto err_handling;
        }

        xi_debug_format( "error reading on socket %d", ( int )layer_data->socket );
        in_out_state = XI_SOCKET_READ_ERROR;
        goto err_handling;
    }

    /* restart io timeouts if needed */
    if ( XI_CONTEXT_DATA( context )->io_timeouts->elem_no > 0 &&
         XI_CONTEXT_DATA( context )->connection_data->connection_timeout > 0 )
    {
        xi_io_timeouts_restart(
            xi_globals.evtd_instance,
            XI_CONTEXT_DATA( context )->connection_data->connection_timeout,
            XI_CONTEXT_DATA( context )->io_timeouts );
    }

    buffer_desc->length   = len;
    buffer_desc->curr_pos = 0;

    return XI_PROCESS_PULL_ON_NEXT_LAYER( context, ( void* )buffer_desc, XI_STATE_OK );

err_handling:
    xi_free_desc( &buffer_desc );

    return XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER( context, 0, in_out_state );
}

xi_state_t xi_io_net_layer_close( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    return XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER( context, data, in_out_state );
}

xi_state_t
xi_io_net_layer_close_externally( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    xi_io_net_layer_state_t* layer_data =
        ( xi_io_net_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    if ( layer_data == 0 )
    {
        return XI_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER( context, data, in_out_state );
    }

    /* unregister the fd */
    xi_evtd_unregister_socket_fd( XI_CONTEXT_DATA( context )->evtd_instance,
                                  layer_data->socket );

    xi_bsp_io_net_close_socket( &layer_data->socket );

    /* cleanup the memory */
    XI_SAFE_FREE( XI_THIS_LAYER( context )->user_data );

    return XI_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER( context, data, in_out_state );
}
