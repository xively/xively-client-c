/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <socket.h>
/* note: socket.h has a define socket->sl_Socket,
         this affects xi_bsp_socket_events_t->socket member.
         This is the reason it is before xi_bsp_io_net.h.
         Other solution is to rename the member, the former was chosen. */

#include <xi_bsp_io_net.h>
#include <stdio.h>

#ifndef MAX
#define MAX( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#endif

#define XI_CC3200_ONBOARD_TLS
// #define XI_CC3200_ROOTCACERT_FILE_NAME "/cert/globalsignrootca.der"
#define XI_CC3200_ROOTCACERT_FILE_NAME "/cert/testcacert.der"

xi_bsp_io_net_state_t xi_bsp_io_net_create_socket( xi_bsp_socket_t* xi_socket )
{
    if ( NULL == xi_socket )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

#ifdef XI_CC3200_ONBOARD_TLS

    // open a secure socket
    *xi_socket = sl_Socket( SL_AF_INET, SL_SOCK_STREAM, SL_SEC_SOCKET );

    if ( *xi_socket < 0 )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    unsigned char ucMethod = SL_SO_SEC_METHOD_TLSV1_2;
    int retval_setsockopt  = sl_SetSockOpt( *xi_socket, SL_SOL_SOCKET, SL_SO_SECMETHOD,
                                           &ucMethod, sizeof( ucMethod ) );
    if ( retval_setsockopt < 0 )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    /*unsigned int uiCipher = SL_SEC_MASK_TLS_RSA_WITH_AES_256_CBC_SHA256;
    retval_setsockopt     = sl_SetSockOpt( *xi_socket, SL_SOL_SOCKET, SL_SO_SECURE_MASK,
                                       &uiCipher, sizeof( uiCipher ) );
    if ( retval_setsockopt < 0 )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }*/

    retval_setsockopt = sl_SetSockOpt(
        *xi_socket, SL_SOL_SOCKET, SL_SO_SECURE_FILES_CA_FILE_NAME,
        XI_CC3200_ROOTCACERT_FILE_NAME, strlen( XI_CC3200_ROOTCACERT_FILE_NAME ) );

    if ( retval_setsockopt < 0 )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

#else

    *xi_socket = sl_Socket( SL_AF_INET, SL_SOCK_STREAM, 0 );

    if ( *xi_socket < 0 )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

#endif

    int sl_nonblockingOption = 1;
    retval_setsockopt =
        sl_SetSockOpt( *xi_socket, SOL_SOCKET, SL_SO_NONBLOCKING, &sl_nonblockingOption,
                       sizeof( sl_nonblockingOption ) );

    if ( retval_setsockopt < 0 )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    return XI_BSP_IO_NET_STATE_OK;
}

xi_bsp_io_net_state_t
xi_bsp_io_net_connect( xi_bsp_socket_t* xi_socket, const char* host, uint16_t port )
{
    if ( NULL == xi_socket || NULL == host )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    // printf( "host: %s, port: %d\n", host, port );

    unsigned long uiIP;

    int errval =
        sl_NetAppDnsGetHostByName( ( _i8* )host, strlen( host ), &uiIP, SL_AF_INET );

    if ( 0 != errval )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    struct sockaddr_in name = {.sin_family      = SL_AF_INET, // SL_AF_INET
                               .sin_port        = htons( port ),
                               .sin_addr.s_addr = htonl( uiIP ),
                               .sin_zero        = {0}};

    errval =
        sl_Connect( *xi_socket, ( struct sockaddr* )&name, sizeof( struct sockaddr ) );

    // printf( "connection result, errval = %d\n", errval );

    if ( errval < 0 && SL_EALREADY != errval )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    return XI_BSP_IO_NET_STATE_OK;
}

xi_bsp_io_net_state_t xi_bsp_io_net_connection_check( xi_bsp_socket_t xi_socket,
                                                      const char* host,
                                                      uint16_t port )
{
    return xi_bsp_io_net_connect( &xi_socket, host, port );
}

xi_bsp_io_net_state_t xi_bsp_io_net_write( xi_bsp_socket_t xi_socket,
                                           int* out_written_count,
                                           const uint8_t* buf,
                                           size_t count )
{
    if ( NULL == out_written_count || NULL == buf )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    *out_written_count = sl_Send( xi_socket, buf, count, 0 );

    // printf( "out_written_count: %d, asked count: %lu\n", *out_written_count, count );

    /* TI's SimpleLink write() returns errors in the return value */
    if ( SL_EAGAIN == *out_written_count )
    {
        return XI_BSP_IO_NET_STATE_BUSY;
    }
    else if ( *out_written_count < 0 )
    {
        *out_written_count = 0;
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    return XI_BSP_IO_NET_STATE_OK;
}

xi_bsp_io_net_state_t xi_bsp_io_net_read( xi_bsp_socket_t xi_socket,
                                          int* out_read_count,
                                          uint8_t* buf,
                                          size_t count )
{
    if ( NULL == out_read_count || NULL == buf )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    *out_read_count = sl_Recv( xi_socket, buf, count, 0 );

    // printf( "out_read_count: %d, asked count: %lu\n", *out_read_count, count );

    if ( SL_EAGAIN == *out_read_count )
    {
        return XI_BSP_IO_NET_STATE_BUSY;
    }
    else if ( *out_read_count < 0 )
    {
        *out_read_count = 0;
        return XI_BSP_IO_NET_STATE_ERROR;
    }
    else if ( 0 == *out_read_count )
    {
        return XI_BSP_IO_NET_STATE_CONNECTION_RESET;
    }

    return XI_BSP_IO_NET_STATE_OK;
}

xi_bsp_io_net_state_t xi_bsp_io_net_close_socket( xi_bsp_socket_t* xi_socket )
{
    ( void )xi_socket;

    return XI_BSP_IO_NET_STATE_OK;
}

xi_bsp_io_net_state_t xi_bsp_io_net_select( xi_bsp_socket_events_t* socket_events_array,
                                            size_t socket_events_array_size,
                                            long timeout_sec )
{
    fd_set rfds;
    fd_set wfds;
    fd_set efds;

    FD_ZERO( &rfds );
    FD_ZERO( &wfds );
    FD_ZERO( &efds );

    int max_fd_read  = 0;
    int max_fd_write = 0;
    int max_fd_error = 0;

    struct timeval tv = {0, 0};

    /* translate the library socket events settings to the event sets used by posix select
     */
    size_t socket_id = 0;
    for ( socket_id = 0; socket_id < socket_events_array_size; ++socket_id )
    {
        xi_bsp_socket_events_t* socket_events = &socket_events_array[socket_id];

        if ( NULL == socket_events )
        {
            return XI_BSP_IO_NET_STATE_ERROR;
        }

        if ( 1 == socket_events->in_socket_want_read )
        {
            FD_SET( socket_events->socket, &rfds );
            max_fd_read =
                socket_events->socket > max_fd_read ? socket_events->socket : max_fd_read;
        }

        if ( ( 1 == socket_events->in_socket_want_write ) ||
             ( 1 == socket_events->in_socket_want_connect ) )
        {
            FD_SET( socket_events->socket, &wfds );
            max_fd_write = socket_events->socket > max_fd_write ? socket_events->socket
                                                                : max_fd_write;
        }

        if ( 1 == socket_events->in_socket_want_error )
        {
            FD_SET( socket_events->socket, &efds );
            max_fd_error = socket_events->socket > max_fd_error ? socket_events->socket
                                                                : max_fd_error;
        }
    }

    /* calculate max fd */
    const int max_fd = MAX( max_fd_read, MAX( max_fd_write, max_fd_error ) );

    tv.tv_sec = 1;

    /* call the actual posix select */
    const int result = select( max_fd + 1, &rfds, &wfds, &efds, &tv );

    if ( 0 < result )
    {
        /* translate the result back to the socket events structure */
        for ( socket_id = 0; socket_id < socket_events_array_size; ++socket_id )
        {
            xi_bsp_socket_events_t* socket_events = &socket_events_array[socket_id];

            if ( FD_ISSET( socket_events->socket, &rfds ) )
            {
                socket_events->out_socket_can_read = 1;
            }

            if ( FD_ISSET( socket_events->socket, &wfds ) )
            {
                if ( 1 == socket_events->in_socket_want_connect )
                {
                    socket_events->out_socket_connect_finished = 1;
                }

                if ( 1 == socket_events->in_socket_want_write )
                {
                    socket_events->out_socket_can_write = 1;
                }
            }

            if ( FD_ISSET( socket_events->socket, &efds ) )
            {
                socket_events->out_socket_error = 1;
            }
        }

        return XI_BSP_IO_NET_STATE_OK;
    }
    else if ( 0 == result )
    {
        return XI_BSP_IO_NET_STATE_OK;
        // return XI_BSP_IO_NET_STATE_TIMEOUT;
    }

    return XI_BSP_IO_NET_STATE_ERROR;
}
