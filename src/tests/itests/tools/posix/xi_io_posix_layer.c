/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "xi_io_posix_layer.h"
#include "xi_io_posix_layer_state.h"
#include "xi_io_posix_layer_compat.h"
#include "xi_allocator.h"
#include "xi_err.h"
#include "xi_macros.h"
#include "xi_debug.h"
#include "xi_layer_api.h"
#include "xi_common.h"
#include "xi_connection_data.h"
#include "xi_coroutine.h"
#include "xi_event_dispatcher_api.h"
#include "xi_io_timeouts.h"
#include "xi_globals.h"

#ifdef __cplusplus
extern "C" {
#endif

#if ( ( XI_IO_LAYER_POSIX_COMPAT == 2 ) || ( XI_IO_LAYER_POSIX_COMPAT == 3 ) )
/* Neither the TI SimpleLink posix-lite layer nor lwip provide errno */
#ifndef LWIP_PROVIDE_ERRNO
int errno = 0;
#endif /* LWIP_PROVIDE_ERRNO */
#endif /*(XI_IO_LAYER_POSIX_COMPAT == 2) */

xi_state_t xi_io_posix_layer_push( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( in_out_state );

    xi_io_posix_layer_state_t* layer_data =
        ( xi_io_posix_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    xi_data_desc_t* buffer = ( xi_data_desc_t* )data;
    int errval = 0;
    XI_UNUSED( errval );
#if ( XI_IO_LAYER_POSIX_COMPAT != 2 )
    /* SimpleLink does not have or need this */
    socklen_t lon = sizeof( int );
#endif /*(XI_IO_LAYER_POSIX_COMPAT != 2) */

    /* check if the layer has been disconnected */
    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) || layer_data == NULL )
    {
        xi_debug_logger( "layer_data not operational" );

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

    if ( buffer != 0 && buffer->capacity > 0 )
    {
        size_t left;

        do
        {
#if ( XI_IO_LAYER_POSIX_COMPAT != 2 )
            /* SimpleLink does not have this functionality */
            if ( getsockopt( layer_data->socket_fd, SOL_SOCKET, SO_ERROR,
                             ( void* )( &errval ), &lon ) < 0 )
            {
                errval = errno;

                xi_debug_logger( "Error while getsockopt" );

                XI_CR_EXIT( layer_data->layer_push_cs,
                            XI_PROCESS_PUSH_ON_NEXT_LAYER( context, 0,
                                                           XI_SOCKET_GETSOCKOPT_ERROR ) );
            }

            if ( errval != 0 )
            {
                xi_debug_format( "Error on socket! %d", errval );
                goto err_val_eval;
            }
#endif /* (XI_IO_LAYER_POSIX_COMPAT != 2) */
            int len = write( layer_data->socket_fd, buffer->data_ptr + buffer->curr_pos,
                             buffer->capacity - buffer->curr_pos );

            xi_debug_format( "written: %d bytes", len );

            if ( len < 0 )
            {
                /* SimpleLink does not have or need this */
                errval = errno;

            err_val_eval:
                if ( errval == EAGAIN ) /* that can happen */
                {
                    XI_CR_EXIT( layer_data->layer_push_cs,
                                xi_evtd_continue_when_evt_on_socket(
                                    XI_CONTEXT_DATA( context )->evtd_instance,
                                    XI_EVENT_WANT_WRITE,
                                    xi_make_handle( &xi_io_posix_layer_push, context,
                                                    data, XI_STATE_WANT_WRITE ),
                                    layer_data->socket_fd ) );
                }
#if ( XI_IO_LAYER_POSIX_COMPAT != 2 )
                else if ( errval == ECONNRESET )
                {
                    /* connection reset by peer */
                    xi_free_desc( &buffer );
                    xi_debug_logger( "connection reset by peer [ECONNRESET]" );
                    XI_CR_EXIT( layer_data->layer_push_cs,
                                XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(
                                    context, 0, XI_CONNECTION_RESET_BY_PEER_ERROR ) );
                }
                else if ( errval == EPIPE )
                {
                    /* connection reset by peer */
                    xi_free_desc( &buffer );
                    xi_debug_logger( "connection reset by peer [EPIPE]" );
                    XI_CR_EXIT( layer_data->layer_push_cs,
                                XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(
                                    context, 0, XI_CONNECTION_RESET_BY_PEER_ERROR ) );
                }
#else
                /* TI's SimpleLink write() returns errors in the return value */
                errval = len;
#endif /* (XI_IO_LAYER_POSIX_COMPAT != 2) */
                xi_debug_printf( "error writing: errno = %d\n", errval );

                xi_free_desc( &buffer );

                XI_CR_EXIT( layer_data->layer_push_cs,
                            XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(
                                context, data, XI_SOCKET_WRITE_ERROR ) );
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

xi_state_t xi_io_posix_layer_pull( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    xi_io_posix_layer_state_t* layer_data =
        ( xi_io_posix_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    unsigned char* data_buffer  = 0;
    xi_data_desc_t* buffer_desc = 0;
    int len                     = 0;

    XI_UNUSED( data_buffer );

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

        data_buffer = buffer_desc->data_ptr;

        assert( data_buffer != 0 );

        buffer_desc->curr_pos = 0;
        buffer_desc->length   = 0;
    }
    else
    {
        buffer_desc = xi_make_empty_desc_alloc( XI_IO_BUFFER_SIZE );
        XI_CHECK_MEMORY( buffer_desc, in_out_state );
    }

    len = read( layer_data->socket_fd, buffer_desc->data_ptr, buffer_desc->capacity );

    xi_debug_format( "read: %d bytes", len );

    if ( len < 0 )
    {
#if ( XI_IO_LAYER_POSIX_COMPAT != 2 )
        /* SimpleLink does not return these values */
        int errval = errno;

        xi_debug_format( "error reading: errno = %d", errval );

        if ( errval == EAGAIN ) /* register for an event on that socket */
        {
            xi_evtd_continue_when_evt_on_socket(
                XI_CONTEXT_DATA( context )->evtd_instance, XI_EVENT_WANT_READ,
                xi_make_handle( &xi_io_posix_layer_pull, context, buffer_desc,
                                in_out_state ),
                layer_data->socket_fd );

            return XI_STATE_WANT_READ;
        }
        else if ( errval == ECONNRESET )
        {
            /* connection reset by peer */
            xi_debug_logger( "connection reset by peer [ECONNRESET]" );
            in_out_state = XI_CONNECTION_RESET_BY_PEER_ERROR;
        }
        else if ( errval == EPIPE )
        {
            /* connection reset by peer */
            xi_debug_logger( "connection reset by peer [EPIPE]" );
            in_out_state = XI_CONNECTION_RESET_BY_PEER_ERROR;
        }
#else
        xi_debug_format( "error reading: return value = %d", len );
#endif /* (XI_IO_LAYER_POSIX_COMPAT != 2) */
        goto err_handling;
    }

    if ( len == 0 ) /* we've been disconnected, so let's roll down */
    {
        xi_debug_logger( "connection reset by peer" );
        xi_free_desc( &buffer_desc );

        in_out_state = XI_CONNECTION_RESET_BY_PEER_ERROR;

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

xi_state_t xi_io_posix_layer_close( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    return XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER( context, data, in_out_state );
}

xi_state_t
xi_io_posix_layer_close_externally( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( in_out_state );
    XI_UNUSED( data );

    xi_io_posix_layer_state_t* layer_data =
        ( xi_io_posix_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    if ( NULL == layer_data )
    {
        return XI_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER( context, data, in_out_state );
    }

#if ( XI_IO_LAYER_POSIX_COMPAT != 2 )
    /* SimpleLink does not have this */
    if ( shutdown( layer_data->socket_fd, SHUT_RDWR ) == -1 )
    {
        close( layer_data->socket_fd ); /* just in case */
    }
#endif /* (XI_IO_LAYER_POSIX_COMPAT != 2) */

    /* close the connection & the socket */
    close( layer_data->socket_fd );

    /* unregister the fd */
    xi_evtd_unregister_socket_fd( XI_CONTEXT_DATA( context )->evtd_instance,
                                  layer_data->socket_fd );

    /* cleanup the memory */
    XI_SAFE_FREE( XI_THIS_LAYER( context )->user_data );

    return XI_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER( context, data, in_out_state );
}

xi_state_t xi_io_posix_layer_init( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( data );
    XI_UNUSED( in_out_state );

    /* PRECONDITIONS */
    assert( context != 0 );
    assert( XI_THIS_LAYER( context )->user_data == 0 );

    xi_layer_t* layer = ( xi_layer_t* )XI_THIS_LAYER( context );

    XI_ALLOC( xi_io_posix_layer_state_t, layer_data, in_out_state );

    layer->user_data = ( void* )layer_data;

    xi_debug_logger( "Creating socket..." );

    layer_data->socket_fd = socket( AF_INET, SOCK_STREAM, 0 );

    if ( layer_data->socket_fd == -1 )
    {
        xi_debug_logger( "Socket creation [failed]" );
        in_out_state = XI_SOCKET_INITIALIZATION_ERROR;
        goto err_handling;
    }

#if ( XI_IO_LAYER_POSIX_COMPAT == 2 )
#if ( 0 )
    /* This is where we WANT to set nonblocking, however, Revision 0.5.3
     * has some issues. If we enable this here, write() returns -1
     * Check this again if you are using a newer revision of SimpleLink */

    /* Enable nonblocking mode for the SimpleLink socket AFTER the connect() call */

    xi_debug_logger( "Setting EARLY socket non blocking behaviour..." );
    SlSockNonblocking_t sl_nonblockingOption;
    sl_nonblockingOption.NonblockingEnabled = 1;

    int flags = sl_SetSockOpt( layer_data->socket_fd, SOL_SOCKET, SL_SO_NONBLOCKING,
                               &sl_nonblockingOption, sizeof( sl_nonblockingOption ) );

    if ( flags < 0 )
    {
        xi_debug_logger( "Socket non blocking behaviour [failed]" );
        in_out_state = XI_SOCKET_INITIALIZATION_ERROR;
        goto err_handling;
    }
#endif
#endif
#if ( XI_IO_LAYER_POSIX_COMPAT != 2 )
    xi_debug_logger( "Setting socket non blocking behaviour..." );

    /* Enable nonblocking mode for a posix socket */
    int flags = fcntl( layer_data->socket_fd, F_GETFL, 0 );

    if ( flags == -1 )
    {
        xi_debug_logger( "Socket non blocking behaviour [failed]" );
        in_out_state = XI_SOCKET_INITIALIZATION_ERROR;
        goto err_handling;
    }

    if ( fcntl( layer_data->socket_fd, F_SETFL, flags | O_NONBLOCK ) == -1 )
    {
        xi_debug_logger( "Socket non blocking behaviour [failed]" );
        in_out_state = XI_SOCKET_INITIALIZATION_ERROR;
        goto err_handling;
    }
#endif /* (XI_IO_LAYER_POSIX_COMPAT != 2) */

    xi_debug_logger( "Socket creation [ok]" );

    /* POSTCONDITIONS */
    assert( layer->user_data != 0 );
    assert( layer_data->socket_fd != -1 );

    return XI_PROCESS_CONNECT_ON_THIS_LAYER( context, data, XI_STATE_OK );

err_handling:
    /* cleanup the memory */
    if ( layer_data )
    {
        close( layer_data->socket_fd );
    }
    if ( layer->user_data )
    {
        XI_SAFE_FREE( layer->user_data );
    }

    return XI_PROCESS_CONNECT_ON_THIS_LAYER( context, data, in_out_state );
}

xi_state_t xi_io_posix_layer_connect( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( data );
    XI_UNUSED( in_out_state );

    /* check the state before doing anything */
    if ( XI_STATE_OK != in_out_state )
    {
        return XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, NULL, in_out_state );
    }

    xi_layer_t* layer = XI_THIS_LAYER( context );
    xi_io_posix_layer_state_t* layer_data =
        ( xi_io_posix_layer_state_t* )layer->user_data;

    /* if layer_data is null on this stage it means internal error */
    if ( NULL == layer_data )
    {
        return XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, NULL, XI_INTERNAL_ERROR );
    }

    xi_connection_data_t* connection_data = ( xi_connection_data_t* )data;
    xi_evtd_instance_t* event_dispatcher  = XI_CONTEXT_DATA( context )->evtd_instance;

#if ( XI_IO_LAYER_POSIX_COMPAT != 2 )
    /* SimpleLink does not have or need these */
    int valopt    = 0;
    socklen_t lon = sizeof( int );
#endif /* (XI_IO_LAYER_POSIX_COMPAT != 2) */
    int errval = 0;

#if ( XI_IO_LAYER_POSIX_COMPAT == 2 )
    unsigned long uiIP;
    long retval;
#endif /* (XI_IO_LAYER_POSIX_COMPAT == 2) */

    XI_CR_START( layer_data->layer_connect_cs )

    xi_debug_format( "Connecting layer [%d] to the endpoint: %s:%hu",
                     layer->layer_type_id, connection_data->host, connection_data->port );

    /* socket specific data */
    struct sockaddr_in name;
#if ( XI_IO_LAYER_POSIX_COMPAT != 2 )
    /* SimpleLink does not have this */
    struct hostent* hostinfo;
#endif /* (XI_IO_LAYER_POSIX_COMPAT != 2) */

/* get the hostaddress */
#if ( XI_IO_LAYER_POSIX_COMPAT == 2 )
    retval =
        sl_NetAppDnsGetHostByName( ( _i8* )connection_data->host,
                                   strlen( connection_data->host ), &uiIP, SL_AF_INET );
#else
    hostinfo = gethostbyname( connection_data->host );
#endif /* (XI_IO_LAYER_POSIX_COMPAT == 2) */

#if ( XI_IO_LAYER_POSIX_COMPAT == 2 )
    if ( retval != 0 )
#else
    /* if null it means that the address has not been found */
    if ( hostinfo == NULL )
#endif /* (XI_IO_LAYER_POSIX_COMPAT == 2) */
    {
        xi_debug_logger( "Getting Host by name [failed]" );

        XI_CR_EXIT( layer_data->layer_connect_cs,
                    XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, data,
                                                      XI_SOCKET_GETHOSTBYNAME_ERROR ) );
    }

    xi_debug_logger( "Getting Host by name [ok]" );

    /* set the address and the port for further connection attempt */
    memset( &name, 0, sizeof( struct sockaddr_in ) );
    name.sin_family = AF_INET;
#if ( XI_IO_LAYER_POSIX_COMPAT == 2 )
    /* TI's SimpleLink domain name lookup function
     * sl_NetAppDnsGetHostByName() returns the IP address in host byte order.
     * as opposed to the posix gethostbyname() which returns the IP address in
     * network byte order. We need to byte swap it before use. */
    name.sin_addr.s_addr = htonl( uiIP );
#else
    name.sin_addr = *( ( struct in_addr* )hostinfo->h_addr_list[0] );
#endif /* (XI_IO_LAYER_POSIX_COMPAT == 2) */
    name.sin_port = htons( connection_data->port );

    xi_debug_logger( "Connecting to the endpoint..." );

    {
        xi_evtd_register_socket_fd(
            event_dispatcher, layer_data->socket_fd,
            xi_make_handle( &xi_io_posix_layer_pull, context, 0, XI_STATE_OK ) );
    }

#if ( XI_IO_LAYER_POSIX_COMPAT == 2 )
    /* TI's SimpleLink does not have an errno, so we'll put
     * the return code into errno since they perform similar functions */
    if ( ( errno = connect( layer_data->socket_fd, ( struct sockaddr* )&name,
                            sizeof( struct sockaddr ) ) == -1 ) )
    {
        errval = errno;

        if ( errval <= 0 )
        {
            xi_debug_printf( "connect returned: %d", errval );
            xi_debug_logger( "Connecting to the endpoint [failed]" );

            XI_CR_EXIT( layer_data->layer_connect_cs,
                        XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, data,
                                                          XI_SOCKET_CONNECTION_ERROR ) );
        }
    }
#else
    if ( connect( layer_data->socket_fd, ( struct sockaddr* )&name,
                  sizeof( struct sockaddr ) ) == -1 )
    {
        errval = errno;

        if ( errval != EINPROGRESS )
        {
            xi_debug_printf( "errno: %d", errval );
            xi_debug_logger( "Connecting to the endpoint [failed]" );

            XI_CR_EXIT( layer_data->layer_connect_cs,
                        XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, data,
                                                          XI_SOCKET_CONNECTION_ERROR ) );
        }
        else
        {
            xi_debug_printf( "errno: %d", errval );
            xi_debug_logger( "EINPROGRESS" );
            xi_evtd_continue_when_evt_on_socket(
                event_dispatcher, XI_EVENT_WANT_CONNECT,
                xi_make_handle( &xi_io_posix_layer_connect, ( void* )context, data,
                                XI_STATE_OK ),
                layer_data->socket_fd );

            XI_CR_YIELD( layer_data->layer_connect_cs,
                         XI_STATE_OK ); // return here whenever we can write
        }
    }

    if ( getsockopt( layer_data->socket_fd, SOL_SOCKET, SO_ERROR, ( void* )( &valopt ),
                     &lon ) < 0 )
    {
        xi_debug_format( "Error while calling getsockopt %s",
                         xi_io_get_error_string( errno ) );

        XI_CR_EXIT( layer_data->layer_connect_cs,
                    XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, data,
                                                      XI_SOCKET_GETSOCKOPT_ERROR ) );
    }

    if ( valopt )
    {
        xi_debug_format( "Error while connecting %s", xi_io_get_error_string( valopt ) );

        XI_CR_EXIT( layer_data->layer_connect_cs,
                    XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, data,
                                                      XI_SOCKET_CONNECTION_ERROR ) );
    }
#endif /* (XI_IO_LAYER_POSIX_COMPAT != 2) */

#if ( XI_IO_LAYER_POSIX_COMPAT == 2 )
#if ( 1 )
    /* Enable nonblocking mode for the SimpleLink socket AFTER the connect() call */

    xi_debug_logger( "Setting LATE socket non blocking behaviour..." );
    SlSockNonblocking_t sl_nonblockingOption;
    sl_nonblockingOption.NonblockingEnabled = 1;

    int flags = sl_SetSockOpt( layer_data->socket_fd, SOL_SOCKET, SL_SO_NONBLOCKING,
                               &sl_nonblockingOption, sizeof( sl_nonblockingOption ) );

    if ( flags < 0 )
    {
        xi_debug_logger( "Socket non blocking behaviour [failed]" );
        XI_CR_EXIT( layer_data->layer_connect_cs,
                    XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, data,
                                                      XI_SOCKET_INITIALIZATION_ERROR ) );
    }
#endif
#endif /* (XI_IO_LAYER_POSIX_COMPAT == 2) */

    /* pass the socket fd to the next layer
     * will be used via ssl layer or any other ssl layer
     * in order to provide asynchronous connect */
    xi_debug_logger( "Connection successful!" );
    XI_CR_EXIT( layer_data->layer_connect_cs,
                XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, &layer_data->socket_fd,
                                                  XI_STATE_OK ) );

    XI_CR_END()
}

#ifdef __cplusplus
}
#endif
