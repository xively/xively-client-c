/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <xi_bsp_io_net.h>
#include <stdio.h>
#include <xi_debug.h>
#include <xi_bsp_debug.h>

#ifdef XI_BSP_IO_NET_TLS_SOCKET
/* Use TLS on Hardware */

#include <xi_bsp_time.h>

#include <time.h>

/* crypto headers used for verifying the certificate found on
the flash file system, if it exists. */
#include <ti/drivers/Power.h>
#include <ti/drivers/crypto/CryptoCC32XX.h>

/* path to store the certificate for WolfSSL on-chip TLS consumption */
#define XI_CC32XX_ROOTCACERT_FILE_NAME "/cert/xively_cert_globalsign_rootca.der"

/* helper functions to check if we already have a certificate on FFS.  IF so,
the certificate is hashed and compared to the one in this library's statically
compile data space.  If ther certificate is not on disk, or there's a signature
mismatch, then the certificate in this library is written to the FFS */
long CheckOrInstallCertficiateOnDevice( unsigned long* ulToken );
long WriteCertificateToFFS( unsigned long* ulToken );

#endif /* XI_BSP_IO_NET_TLS_SOCKET */

#ifndef MAX
#define MAX( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#endif /* MAX */

xi_bsp_io_net_state_t xi_bsp_io_net_create_socket( xi_bsp_socket_t* xi_socket )
{
    if ( NULL == xi_socket )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    int retval = 0;

#ifdef XI_BSP_IO_NET_TLS_SOCKET
    unsigned long ulToken;

    if ( ( retval = CheckOrInstallCertficiateOnDevice( &ulToken ) ) < 0 )
    {
        xi_debug_printf( "ERROR: CheckOrInstallCertficiateOnDevice result: %d\n\r",
                         retval );
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    {
        /* set current time for certificate validity check during TLS handshake */
        const time_t current_time_seconds = xi_bsp_time_getcurrenttime_seconds();
        struct tm* const gm_time          = localtime( &current_time_seconds );

        /* conversion between tm datetime representation to Sl datetime representation
           tm years: since 1970, Sl years: since 0
           tm months: 0-11, Sl months: 1-12 */

        SlDateTime_t sl_datetime;
        sl_datetime.tm_year = gm_time->tm_year + 1970;
        sl_datetime.tm_mon  = gm_time->tm_mon + 1;
        sl_datetime.tm_day  = gm_time->tm_mday;
        sl_datetime.tm_hour = gm_time->tm_hour;
        sl_datetime.tm_min  = gm_time->tm_min;
        sl_datetime.tm_sec  = gm_time->tm_sec;

        retval =
            sl_DeviceSet( SL_DEVICE_GENERAL, SL_DEVICE_GENERAL_DATE_TIME,
                          sizeof( SlDateTime_t ), ( unsigned char* )( &sl_datetime ) );

        if ( retval < 0 )
        {
            return XI_BSP_IO_NET_STATE_ERROR;
        }
    }

    /* open a secure socket */
    *xi_socket = sl_Socket( SL_AF_INET, SL_SOCK_STREAM, SL_SEC_SOCKET );

    if ( *xi_socket < 0 )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

#ifdef XI_CC3220SF_UNSAFELY_DISABLE_CERT_STORE
    /* Disable usage of the on-board Certificate Catalog - Also disables the CRL */
    int32_t dummyValue = 0;
    retval =
        sl_SetSockOpt( *xi_socket, SL_SOL_SOCKET, SL_SO_SECURE_DISABLE_CERTIFICATE_STORE,
                       &dummyValue, sizeof( dummyValue ) );
    if ( retval < 0 )
    {
        xi_bsp_debug_logger(
            "[ERROR] Failed to disable certificate catalog validation\n\r" );
        return XI_BSP_IO_NET_STATE_ERROR;
    }
#endif /* XI_CC3220SF_UNSAFELY_DISABLE_CERT_STORE */

    /* set TLS version to 1.2 */
    unsigned char ucMethod = SL_SO_SEC_METHOD_TLSV1_2;
    retval = sl_SetSockOpt( *xi_socket, SL_SOL_SOCKET, SL_SO_SECMETHOD, &ucMethod,
                            sizeof( ucMethod ) );
    if ( retval < 0 )
    {
        xi_bsp_debug_logger( "[ERROR] Failed to set TLSv1.2 socket option\n\r" );
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    /* set trusted Root CA Cert file path */
    retval = sl_SetSockOpt( *xi_socket, SL_SOL_SOCKET, SL_SO_SECURE_FILES_CA_FILE_NAME,
                            XI_CC32XX_ROOTCACERT_FILE_NAME,
                            strlen( XI_CC32XX_ROOTCACERT_FILE_NAME ) );
    if ( retval < 0 )
    {
        xi_bsp_debug_logger( "[ERROR] Failed to set root certificate\n\r" );
        return XI_BSP_IO_NET_STATE_ERROR;
    }

#else
    /* open a socket */
    *xi_socket = sl_Socket( SL_AF_INET, SL_SOCK_STREAM, 0 );

    if ( *xi_socket < 0 )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

#endif /* XI_BSP_IO_NET_TLS_SOCKET */

    int sl_nonblockingOption = 1;
    retval                   = sl_SetSockOpt( *xi_socket, SOL_SOCKET, SL_SO_NONBLOCKING,
                            &sl_nonblockingOption, sizeof( sl_nonblockingOption ) );

    if ( retval < 0 )
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

    unsigned long uiIP;
    int errval = 0;

#ifdef XI_BSP_IO_NET_TLS_SOCKET
    /* Set hostname for verification by the on-board TLS implementation */
    errval = sl_SetSockOpt( *xi_socket, SL_SOL_SOCKET,
                            SL_SO_SECURE_DOMAIN_NAME_VERIFICATION, host, strlen( host ) );
    if ( errval < 0 )
    {
        xi_bsp_debug_logger( "[ERROR] Failed to configure domain name validation\n\r" );
        return XI_BSP_IO_NET_STATE_ERROR;
    }
#endif

    /* Resolve hostname to IP address */
    errval = sl_NetAppDnsGetHostByName( ( _i8* )host, strlen( host ), &uiIP, SL_AF_INET );

    if ( 0 != errval )
    {
        xi_debug_printf( "Error: couldn't resolve hostname\n\r" );
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    struct sockaddr_in name = {.sin_family      = SL_AF_INET, /* SL_AF_INET */
                               .sin_port        = htons( port ),
                               .sin_addr.s_addr = htonl( uiIP ),
                               .sin_zero        = {0}};

    errval =
        sl_Connect( *xi_socket, ( struct sockaddr* )&name, sizeof( struct sockaddr ) );

    if ( SL_ERROR_BSD_ESECUNKNOWNROOTCA == errval )
    {
        xi_bsp_debug_logger( "[SECURITY CHECK FAILED] Unknown Root CA\n\r" );
        xi_bsp_debug_logger( "\tAborting connection to the MQTT broker\n\r" );
        return XI_BSP_IO_NET_STATE_ERROR;
    }
    else if ( errval < 0 && errval != SL_ERROR_BSD_EALREADY )
    {
        xi_debug_printf( "Error on sl_Connect: %d\n\r", errval );
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

    /* TI's SimpleLink write() returns errors in the return value */
    if ( SL_ERROR_BSD_EAGAIN == *out_written_count )
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

    if ( SL_ERROR_BSD_EAGAIN == *out_read_count )
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
    sl_Close( *xi_socket );

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

    /* translate the library socket events settings to the event sets used by posix
       select */
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

    tv.tv_sec = 1;

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
        return XI_BSP_IO_NET_STATE_OK;
    }

    return XI_BSP_IO_NET_STATE_ERROR;
}


#ifdef XI_BSP_IO_NET_TLS_SOCKET

/* compiled in certificate data so that we can write it to the Flash File System
 for on-chip wolfTLS implementation (which reads certs from 'disk'. */
const unsigned char cert_globalsign_rootca_DER[889] = {
    0x30, 0x82, 0x03, 0x75, 0x30, 0x82, 0x02, 0x5d, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02,
    0x0b, 0x04, 0x00, 0x00, 0x00, 0x00, 0x01, 0x15, 0x4b, 0x5a, 0xc3, 0x94, 0x30, 0x0d,
    0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05, 0x05, 0x00, 0x30,
    0x57, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x42, 0x45,
    0x31, 0x19, 0x30, 0x17, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, 0x10, 0x47, 0x6c, 0x6f,
    0x62, 0x61, 0x6c, 0x53, 0x69, 0x67, 0x6e, 0x20, 0x6e, 0x76, 0x2d, 0x73, 0x61, 0x31,
    0x10, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x13, 0x07, 0x52, 0x6f, 0x6f, 0x74,
    0x20, 0x43, 0x41, 0x31, 0x1b, 0x30, 0x19, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x12,
    0x47, 0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x53, 0x69, 0x67, 0x6e, 0x20, 0x52, 0x6f, 0x6f,
    0x74, 0x20, 0x43, 0x41, 0x30, 0x1e, 0x17, 0x0d, 0x39, 0x38, 0x30, 0x39, 0x30, 0x31,
    0x31, 0x32, 0x30, 0x30, 0x30, 0x30, 0x5a, 0x17, 0x0d, 0x32, 0x38, 0x30, 0x31, 0x32,
    0x38, 0x31, 0x32, 0x30, 0x30, 0x30, 0x30, 0x5a, 0x30, 0x57, 0x31, 0x0b, 0x30, 0x09,
    0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x42, 0x45, 0x31, 0x19, 0x30, 0x17, 0x06,
    0x03, 0x55, 0x04, 0x0a, 0x13, 0x10, 0x47, 0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x53, 0x69,
    0x67, 0x6e, 0x20, 0x6e, 0x76, 0x2d, 0x73, 0x61, 0x31, 0x10, 0x30, 0x0e, 0x06, 0x03,
    0x55, 0x04, 0x0b, 0x13, 0x07, 0x52, 0x6f, 0x6f, 0x74, 0x20, 0x43, 0x41, 0x31, 0x1b,
    0x30, 0x19, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x12, 0x47, 0x6c, 0x6f, 0x62, 0x61,
    0x6c, 0x53, 0x69, 0x67, 0x6e, 0x20, 0x52, 0x6f, 0x6f, 0x74, 0x20, 0x43, 0x41, 0x30,
    0x82, 0x01, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01,
    0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00, 0x30, 0x82, 0x01, 0x0a, 0x02,
    0x82, 0x01, 0x01, 0x00, 0xda, 0x0e, 0xe6, 0x99, 0x8d, 0xce, 0xa3, 0xe3, 0x4f, 0x8a,
    0x7e, 0xfb, 0xf1, 0x8b, 0x83, 0x25, 0x6b, 0xea, 0x48, 0x1f, 0xf1, 0x2a, 0xb0, 0xb9,
    0x95, 0x11, 0x04, 0xbd, 0xf0, 0x63, 0xd1, 0xe2, 0x67, 0x66, 0xcf, 0x1c, 0xdd, 0xcf,
    0x1b, 0x48, 0x2b, 0xee, 0x8d, 0x89, 0x8e, 0x9a, 0xaf, 0x29, 0x80, 0x65, 0xab, 0xe9,
    0xc7, 0x2d, 0x12, 0xcb, 0xab, 0x1c, 0x4c, 0x70, 0x07, 0xa1, 0x3d, 0x0a, 0x30, 0xcd,
    0x15, 0x8d, 0x4f, 0xf8, 0xdd, 0xd4, 0x8c, 0x50, 0x15, 0x1c, 0xef, 0x50, 0xee, 0xc4,
    0x2e, 0xf7, 0xfc, 0xe9, 0x52, 0xf2, 0x91, 0x7d, 0xe0, 0x6d, 0xd5, 0x35, 0x30, 0x8e,
    0x5e, 0x43, 0x73, 0xf2, 0x41, 0xe9, 0xd5, 0x6a, 0xe3, 0xb2, 0x89, 0x3a, 0x56, 0x39,
    0x38, 0x6f, 0x06, 0x3c, 0x88, 0x69, 0x5b, 0x2a, 0x4d, 0xc5, 0xa7, 0x54, 0xb8, 0x6c,
    0x89, 0xcc, 0x9b, 0xf9, 0x3c, 0xca, 0xe5, 0xfd, 0x89, 0xf5, 0x12, 0x3c, 0x92, 0x78,
    0x96, 0xd6, 0xdc, 0x74, 0x6e, 0x93, 0x44, 0x61, 0xd1, 0x8d, 0xc7, 0x46, 0xb2, 0x75,
    0x0e, 0x86, 0xe8, 0x19, 0x8a, 0xd5, 0x6d, 0x6c, 0xd5, 0x78, 0x16, 0x95, 0xa2, 0xe9,
    0xc8, 0x0a, 0x38, 0xeb, 0xf2, 0x24, 0x13, 0x4f, 0x73, 0x54, 0x93, 0x13, 0x85, 0x3a,
    0x1b, 0xbc, 0x1e, 0x34, 0xb5, 0x8b, 0x05, 0x8c, 0xb9, 0x77, 0x8b, 0xb1, 0xdb, 0x1f,
    0x20, 0x91, 0xab, 0x09, 0x53, 0x6e, 0x90, 0xce, 0x7b, 0x37, 0x74, 0xb9, 0x70, 0x47,
    0x91, 0x22, 0x51, 0x63, 0x16, 0x79, 0xae, 0xb1, 0xae, 0x41, 0x26, 0x08, 0xc8, 0x19,
    0x2b, 0xd1, 0x46, 0xaa, 0x48, 0xd6, 0x64, 0x2a, 0xd7, 0x83, 0x34, 0xff, 0x2c, 0x2a,
    0xc1, 0x6c, 0x19, 0x43, 0x4a, 0x07, 0x85, 0xe7, 0xd3, 0x7c, 0xf6, 0x21, 0x68, 0xef,
    0xea, 0xf2, 0x52, 0x9f, 0x7f, 0x93, 0x90, 0xcf, 0x02, 0x03, 0x01, 0x00, 0x01, 0xa3,
    0x42, 0x30, 0x40, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x01, 0x01, 0xff, 0x04,
    0x04, 0x03, 0x02, 0x01, 0x06, 0x30, 0x0f, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01, 0x01,
    0xff, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xff, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d,
    0x0e, 0x04, 0x16, 0x04, 0x14, 0x60, 0x7b, 0x66, 0x1a, 0x45, 0x0d, 0x97, 0xca, 0x89,
    0x50, 0x2f, 0x7d, 0x04, 0xcd, 0x34, 0xa8, 0xff, 0xfc, 0xfd, 0x4b, 0x30, 0x0d, 0x06,
    0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05, 0x05, 0x00, 0x03, 0x82,
    0x01, 0x01, 0x00, 0xd6, 0x73, 0xe7, 0x7c, 0x4f, 0x76, 0xd0, 0x8d, 0xbf, 0xec, 0xba,
    0xa2, 0xbe, 0x34, 0xc5, 0x28, 0x32, 0xb5, 0x7c, 0xfc, 0x6c, 0x9c, 0x2c, 0x2b, 0xbd,
    0x09, 0x9e, 0x53, 0xbf, 0x6b, 0x5e, 0xaa, 0x11, 0x48, 0xb6, 0xe5, 0x08, 0xa3, 0xb3,
    0xca, 0x3d, 0x61, 0x4d, 0xd3, 0x46, 0x09, 0xb3, 0x3e, 0xc3, 0xa0, 0xe3, 0x63, 0x55,
    0x1b, 0xf2, 0xba, 0xef, 0xad, 0x39, 0xe1, 0x43, 0xb9, 0x38, 0xa3, 0xe6, 0x2f, 0x8a,
    0x26, 0x3b, 0xef, 0xa0, 0x50, 0x56, 0xf9, 0xc6, 0x0a, 0xfd, 0x38, 0xcd, 0xc4, 0x0b,
    0x70, 0x51, 0x94, 0x97, 0x98, 0x04, 0xdf, 0xc3, 0x5f, 0x94, 0xd5, 0x15, 0xc9, 0x14,
    0x41, 0x9c, 0xc4, 0x5d, 0x75, 0x64, 0x15, 0x0d, 0xff, 0x55, 0x30, 0xec, 0x86, 0x8f,
    0xff, 0x0d, 0xef, 0x2c, 0xb9, 0x63, 0x46, 0xf6, 0xaa, 0xfc, 0xdf, 0xbc, 0x69, 0xfd,
    0x2e, 0x12, 0x48, 0x64, 0x9a, 0xe0, 0x95, 0xf0, 0xa6, 0xef, 0x29, 0x8f, 0x01, 0xb1,
    0x15, 0xb5, 0x0c, 0x1d, 0xa5, 0xfe, 0x69, 0x2c, 0x69, 0x24, 0x78, 0x1e, 0xb3, 0xa7,
    0x1c, 0x71, 0x62, 0xee, 0xca, 0xc8, 0x97, 0xac, 0x17, 0x5d, 0x8a, 0xc2, 0xf8, 0x47,
    0x86, 0x6e, 0x2a, 0xc4, 0x56, 0x31, 0x95, 0xd0, 0x67, 0x89, 0x85, 0x2b, 0xf9, 0x6c,
    0xa6, 0x5d, 0x46, 0x9d, 0x0c, 0xaa, 0x82, 0xe4, 0x99, 0x51, 0xdd, 0x70, 0xb7, 0xdb,
    0x56, 0x3d, 0x61, 0xe4, 0x6a, 0xe1, 0x5c, 0xd6, 0xf6, 0xfe, 0x3d, 0xde, 0x41, 0xcc,
    0x07, 0xae, 0x63, 0x52, 0xbf, 0x53, 0x53, 0xf4, 0x2b, 0xe9, 0xc7, 0xfd, 0xb6, 0xf7,
    0x82, 0x5f, 0x85, 0xd2, 0x41, 0x18, 0xdb, 0x81, 0xb3, 0x04, 0x1c, 0xc5, 0x1f, 0xa4,
    0x80, 0x6f, 0x15, 0x20, 0xc9, 0xde, 0x0c, 0x88, 0x0a, 0x1d, 0xd6, 0x66, 0x55, 0xe2,
    0xfc, 0x48, 0xc9, 0x29, 0x26, 0x69, 0xe0};

#define SL_MAX_FILE_SIZE 64L * 1024L /* 64KB file */

/* Application specific status/error codes */
typedef enum {
    /* Choosing this number to avoid overlap w/ host-driver's error codes */
    FILE_ALREADY_EXIST     = -0x7E0,
    FILE_CLOSE_ERROR       = FILE_ALREADY_EXIST - 1,
    FILE_NOT_MATCHED       = FILE_CLOSE_ERROR - 1,
    FILE_OPEN_READ_FAILED  = FILE_NOT_MATCHED - 1,
    FILE_OPEN_WRITE_FAILED = FILE_OPEN_READ_FAILED - 1,
    FILE_READ_FAILED       = FILE_OPEN_WRITE_FAILED - 1,
    FILE_WRITE_FAILED      = FILE_READ_FAILED - 1,
} e_AppStatusCodes_file;

#define ASSERT_ON_ERROR( error_code )                                                    \
    {                                                                                    \
        if ( error_code < 0 )                                                            \
        {                                                                                \
            xi_debug_printf( "ERROR: %d\n\r", error_code );                              \
            return error_code;                                                           \
        }                                                                                \
    }

long CheckOrInstallCertficiateOnDevice( unsigned long* ulToken )
{
    /* Check to see if the file already exists.
       If it does then do a digest on the contents to ensure
       that this is the same certificate that we have compiled into
       the library.
       If it does not, or if the digest signature fails to match,
       the write the certificate to disk */
    long lRetVal        = -1;
    uint8_t file_exists = 0;
    SlFsFileInfo_t file_info;
    const uint32_t CERTIFICATE_SIZE = sizeof( cert_globalsign_rootca_DER );

    /* Check for certificate existance and size */
    lRetVal = sl_FsGetInfo( ( unsigned char* )XI_CC32XX_ROOTCACERT_FILE_NAME, *ulToken,
                            &file_info );
    if ( lRetVal == 0 && file_info.Len == sizeof( cert_globalsign_rootca_DER ) )
    {
        file_exists = 1;
    }

    int32_t write_file = 1;
    if ( file_exists )
    {
        /* ok, certfiicate exists. Check it's signature. */
        CryptoCC32XX_init();

        /* this index is taken from enumerated types of cryptohmac example, which has
           only one named '0' and assigned a value of '0'.  See
           CC3220SF_LAUNCHXL_CryptoName defined in CC3220SF_LAUNCHXL.h of the cryptohmac
           example provided by TI. */
        const uint32_t cryptoIndex     = 0;
        CryptoCC32XX_HmacMethod config = CryptoCC32XX_HMAC_SHA256; /* hash length : 32 */
        const uint32_t hashLength      = 32;

        CryptoCC32XX_Handle cryptoHandle =
            CryptoCC32XX_open( cryptoIndex, CryptoCC32XX_HMAC );

        CryptoCC32XX_HmacParams params;
        char* hmac_key =
            "xZN3weH8j9FD032TbJp!qV!X1#8l7^ni$rmjGNPkStP!i2&y6r^OmemoqnJ!g8XL";
        params.pKey     = ( uint8_t* )hmac_key[0];
        params.moreData = 0;

        uint8_t rom_cert_hash_result[hashLength];
        memset( rom_cert_hash_result, 0, hashLength );

        /* Create a SHA256 digest signature of the certificate that we have compiled in */
        CryptoCC32XX_sign( cryptoHandle, config, ( void* )&cert_globalsign_rootca_DER[0],
                           889, ( uint8_t* )&rom_cert_hash_result, &params );

        /* Open the file on the FFS */
        long lFileHandle = sl_FsOpen( ( unsigned char* )XI_CC32XX_ROOTCACERT_FILE_NAME,
                                      SL_FS_READ, ulToken );

        if ( lFileHandle < 0 )
        {
            xi_debug_printf(
                "Warning: opening Xively certficate file: \"%s\" error: %d\n\r",
                XI_CC32XX_ROOTCACERT_FILE_NAME, lFileHandle );
        }
        else
        {
            /* Read the file into memory */
            int32_t bytes_read = 0;
            unsigned char certificate_file_buffer[CERTIFICATE_SIZE];

            while ( bytes_read < CERTIFICATE_SIZE )
            {
                int32_t fs_result = sl_FsRead( lFileHandle, bytes_read,
                                               &certificate_file_buffer[bytes_read],
                                               CERTIFICATE_SIZE - bytes_read );

                if ( fs_result < 0 )
                {
                    xi_debug_printf(
                        "Error in certificate read for file \"%s\" error: %d\n\r",
                        XI_CC32XX_ROOTCACERT_FILE_NAME, bytes_read );
                    return fs_result;
                }

                bytes_read += fs_result;
            }

            sl_FsClose( lFileHandle, NULL, NULL, 0 );

            /* Verify the File data with the compiled-in digest sigature */
            if ( CryptoCC32XX_STATUS_SUCCESS ==
                 CryptoCC32XX_verify( cryptoHandle, config, certificate_file_buffer,
                                      CERTIFICATE_SIZE, rom_cert_hash_result, &params ) )
            {
                /* checksum checks out, no need to write the file again. */
                write_file = 0;
            }

            CryptoCC32XX_close( cryptoHandle );
        }
    }

    if ( write_file )
    {
        return WriteCertificateToFFS( ulToken );
    }

    return 0;
}

long WriteCertificateToFFS( unsigned long* ulToken )
{
    /*  create a user file */
    long lRetVal      = -1;
    long lFileHandle  = -1;
    uint32_t max_size = sizeof( cert_globalsign_rootca_DER );

    /* open the file with create flags */
    lFileHandle = sl_FsOpen(
        ( unsigned char* )XI_CC32XX_ROOTCACERT_FILE_NAME,
        SL_FS_CREATE | SL_FS_OVERWRITE | SL_FS_CREATE_MAX_SIZE( max_size ), ulToken );

    lRetVal = lFileHandle;
    if ( lRetVal < 0 )
    {
        lRetVal = sl_FsClose( lFileHandle, 0, 0, 0 );
        xi_debug_printf( "WARNING: error in certificate file creation: \"%s\"\n\r",
                         XI_CC32XX_ROOTCACERT_FILE_NAME );
        ASSERT_ON_ERROR( lRetVal );
    }

    /* Write the certificate data. It's a snap! */
    lRetVal = sl_FsWrite( lFileHandle, 0, ( unsigned char* )cert_globalsign_rootca_DER,
                          sizeof( cert_globalsign_rootca_DER ) );
    if ( lRetVal < 0 )
    {
        /* Error while writing */
        lRetVal = sl_FsClose( lFileHandle, 0, 0, 0 );
        xi_debug_printf( "WARNING: file \"%s\" write failure!  %d\n\r",
                         XI_CC32XX_ROOTCACERT_FILE_NAME, lRetVal );
        ASSERT_ON_ERROR( FILE_WRITE_FAILED );
    }

    /* Close the file */
    lRetVal = sl_FsClose( lFileHandle, 0, 0, 0 );
    if ( SL_RET_CODE_OK != lRetVal )
    {
        xi_debug_printf( "WARNING: file \"%s\" close failure!  %d\n\r",
                         XI_CC32XX_ROOTCACERT_FILE_NAME, lRetVal );
        ASSERT_ON_ERROR( FILE_CLOSE_ERROR );
    }
    return 0;
}

#endif /* XI_BSP_IO_NET_TLS_SOCKET */
