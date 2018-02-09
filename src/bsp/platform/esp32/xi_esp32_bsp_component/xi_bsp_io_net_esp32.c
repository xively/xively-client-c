/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <string.h>

#include <lwip/netdb.h>

#include <xi_bsp_io_net.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX
#define MAX( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#endif

xi_bsp_io_net_state_t xi_bsp_io_net_create_socket( xi_bsp_socket_t* xi_socket )
{
    *xi_socket = socket( AF_INET, SOCK_STREAM, 0 );

    if ( -1 == *xi_socket )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    /* Enable nonblocking mode for a posix socket */
    const int flags = fcntl( *xi_socket, F_GETFL, 0 );

    if ( -1 == flags || -1 == fcntl( *xi_socket, F_SETFL, flags | O_NONBLOCK ) )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    return XI_BSP_IO_NET_STATE_OK;
}

xi_bsp_io_net_state_t
xi_bsp_io_net_connect( xi_bsp_socket_t* xi_socket, const char* host, uint16_t port )
{
    struct hostent* hostinfo = gethostbyname( host );
    int errno_cpy = 0; /* errno is reset every time it's read */

    /* if null it means that the address has not been found */
    if ( NULL == hostinfo )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    struct sockaddr_in name = {.sin_family = AF_INET,
                               .sin_port   = htons( port ),
                               .sin_addr =
                                   *( ( struct in_addr* )hostinfo->h_addr_list[0] ),
                               .sin_zero = {0}};

    if ( -1 ==
         connect( *xi_socket, ( struct sockaddr* )&name, sizeof( struct sockaddr ) ) )
    {
        errno_cpy = errno;
        return ( EINPROGRESS == errno_cpy ) ? XI_BSP_IO_NET_STATE_OK
                                            : XI_BSP_IO_NET_STATE_ERROR;
    }
    else
    {
        // todo_atigyi: what to do here?
        // does this mean the socket is BLOCKING?
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    return XI_BSP_IO_NET_STATE_ERROR;
}

xi_bsp_io_net_state_t xi_bsp_io_net_connection_check( xi_bsp_socket_t xi_socket,
                                                      const char* host,
                                                      uint16_t port )
{
    ( void )host;
    ( void )port;

    int valopt    = 0;
    socklen_t lon = sizeof( int );

    int result =
        getsockopt( xi_socket, SOL_SOCKET, SO_ERROR, ( void* )( &valopt ), &lon );

    if ( result < 0 )
    {
        // int errval = errno;
        // printf( "getsockopt failed with ret %d, errval %d", result, errval );
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    if ( valopt )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    return XI_BSP_IO_NET_STATE_OK;
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

    int errval    = 0;
    socklen_t lon = sizeof( int );

    if ( getsockopt( xi_socket, SOL_SOCKET, SO_ERROR, ( void* )( &errval ), &lon ) < 0 )
    {
        errval = errno;
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    if ( errval != 0 )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    *out_written_count = write( xi_socket, buf, count );

    if ( 0 > *out_written_count )
    {
        errval = errno;

        if ( EAGAIN == errval )
        {
            return XI_BSP_IO_NET_STATE_BUSY;
        }

        if ( ECONNRESET == errval || EPIPE == errval )
        {
            return XI_BSP_IO_NET_STATE_CONNECTION_RESET;
        }
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

    int errval      = 0;
    *out_read_count = read( xi_socket, buf, count );

    if ( 0 > *out_read_count )
    {
        errval = errno;

        if ( EAGAIN == errval )
        {
            return XI_BSP_IO_NET_STATE_BUSY;
        }

        if ( ECONNRESET == errval || EPIPE == errval )
        {
            return XI_BSP_IO_NET_STATE_CONNECTION_RESET;
        }
    }

    if ( 0 == *out_read_count )
    {
        return XI_BSP_IO_NET_STATE_CONNECTION_RESET;
    }

    return XI_BSP_IO_NET_STATE_OK;
}

xi_bsp_io_net_state_t xi_bsp_io_net_close_socket( xi_bsp_socket_t* xi_socket )
{
    if ( NULL == xi_socket )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    shutdown( *xi_socket, SHUT_RDWR );

    close( *xi_socket );

    *xi_socket = 0;

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
            FD_SET( socket_events->xi_socket, &rfds );
            max_fd_read = socket_events->xi_socket > max_fd_read
                              ? socket_events->xi_socket
                              : max_fd_read;
        }

        if ( ( 1 == socket_events->in_socket_want_write ) ||
             ( 1 == socket_events->in_socket_want_connect ) )
        {
            FD_SET( socket_events->xi_socket, &wfds );
            max_fd_write = socket_events->xi_socket > max_fd_write
                               ? socket_events->xi_socket
                               : max_fd_write;
        }

        if ( 1 == socket_events->in_socket_want_error )
        {
            FD_SET( socket_events->xi_socket, &efds );
            max_fd_error = socket_events->xi_socket > max_fd_error
                               ? socket_events->xi_socket
                               : max_fd_error;
        }
    }

    /* calculate max fd */
    const int max_fd = MAX( max_fd_read, MAX( max_fd_write, max_fd_error ) );

    tv.tv_sec = timeout_sec;

    /* call the actual posix select */
    const int result = select( max_fd + 1, &rfds, &wfds, &efds, &tv );

    if ( 0 < result )
    {
        /* translate the result back to the socket events structure */
        for ( socket_id = 0; socket_id < socket_events_array_size; ++socket_id )
        {
            xi_bsp_socket_events_t* socket_events = &socket_events_array[socket_id];

            if ( FD_ISSET( socket_events->xi_socket, &rfds ) )
            {
                socket_events->out_socket_can_read = 1;
            }

            if ( FD_ISSET( socket_events->xi_socket, &wfds ) )
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

            if ( FD_ISSET( socket_events->xi_socket, &efds ) )
            {
                socket_events->out_socket_error = 1;
            }
        }

        return XI_BSP_IO_NET_STATE_OK;
    }
    else if ( 0 == result )
    {
        return XI_BSP_IO_NET_STATE_TIMEOUT;
    }

    return XI_BSP_IO_NET_STATE_ERROR;
}

#ifdef __cplusplus
}
#endif
