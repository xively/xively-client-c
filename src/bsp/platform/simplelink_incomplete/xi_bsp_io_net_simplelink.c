/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_io_net.h>

#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

#define __XI_FAKE_SIMPLELINK_BSP_NET_IMPLEMENTATION__

#ifdef __XI_FAKE_SIMPLELINK_BSP_NET_IMPLEMENTATION__

#include <unistd.h>

#define SL_AF_INET AF_INET
#define SL_SO_NONBLOCKING 1

typedef void _i8;

typedef struct
{
    uint8_t NonblockingEnabled;
} SlSockNonblocking_t;

int sl_NetAppDnsGetHostByName( _i8* host,
                               size_t host_len,
                               unsigned long* ip,
                               int inet_family );
int sl_SetSockOpt( intptr_t sock, int a, int b, SlSockNonblocking_t* flags, size_t len );

#endif /* __XI_FAKE_SIMPLELINK_BSP_NET_IMPLEMENTATION__ */

#ifdef __cplusplus
extern "C" {
#endif

xi_bsp_io_net_state_t xi_bsp_io_net_create_socket( xi_bsp_socket_t* xi_socket )
{
    if ( NULL == xi_socket )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    *xi_socket = socket( SL_AF_INET, SOCK_STREAM, 0 );

    if ( -1 == *xi_socket )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    /* This is where we WANT to set nonblocking, however, Revision 0.5.3
     * has some issues. If we enable nonblocking here, write() returns -1
     * Check this again if you are using a newer revision of SimpleLink */

    /* Enable nonblocking mode for the SimpleLink socket AFTER the connect() call */

    return XI_BSP_IO_NET_STATE_OK;
}

xi_bsp_io_net_state_t
xi_bsp_io_net_connect( xi_bsp_socket_t* xi_socket, const char* host, uint16_t port )
{
    if ( NULL == xi_socket || NULL == host )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    unsigned long uiIP;

    int errval =
        sl_NetAppDnsGetHostByName( ( _i8* )host, strlen( host ), &uiIP, SL_AF_INET );

    if ( 0 != errval )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    struct sockaddr_in name = {.sin_family = AF_INET, // SL_AF_INET
                               .sin_port   = htons( port ),
                               .sin_addr   = {htonl( uiIP )},
                               .sin_zero   = {0}};

    errval = connect( *xi_socket, ( struct sockaddr* )&name, sizeof( struct sockaddr ) );

    if ( -1 != errval )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    /* Enable nonblocking mode for the SimpleLink socket AFTER the connect() call */
    SlSockNonblocking_t sl_nonblockingOption;
    sl_nonblockingOption.NonblockingEnabled = 1;

    int flags = sl_SetSockOpt( *xi_socket, SOL_SOCKET, SL_SO_NONBLOCKING,
                               &sl_nonblockingOption, sizeof( sl_nonblockingOption ) );

    if ( flags < 0 )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    return XI_BSP_IO_NET_STATE_OK;
}

xi_bsp_io_net_state_t xi_bsp_io_net_connection_check( xi_bsp_socket_t xi_socket,
                                                      const char* host,
                                                      uint16_t port )
{
    ( void )xi_socket;
    ( void )host;
    ( void )port;

    /* no-operation is needed here for simplelink */

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

    *out_written_count = write( xi_socket, buf, count );

    /* TI's SimpleLink write() returns errors in the return value */
    if ( *out_written_count < 0 )
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

    *out_read_count = read( xi_socket, buf, count );

    if ( *out_read_count < 0 )
    {
        *out_read_count = 0;
        return XI_BSP_IO_NET_STATE_ERROR;
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

    close( *xi_socket );

    *xi_socket = 0;

    return XI_BSP_IO_NET_STATE_OK;
}

#ifdef __cplusplus
}
#endif
