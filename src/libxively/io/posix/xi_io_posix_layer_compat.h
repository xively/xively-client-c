/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#ifndef __XI_IO_POSIX_LAYER_COMPAT_H__
#define __XI_IO_POSIX_LAYER_COMPAT_H__

#include "xi_config.h"

#if ( !defined( XI_IO_LAYER_POSIX_COMPAT ) ) || ( XI_IO_LAYER_POSIX_COMPAT == 0 )
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#define xi_io_get_error_string( e ) strerror( e )
#elif XI_IO_LAYER_POSIX_COMPAT == 1
#if ( defined( __MBED__ ) && !defined( select ) && !defined( fcntl ) &&                  \
      !defined( gethostbyname ) )
#include <lwip/netdb.h>
#include <lwip/sockets.h>
/* the compatibility macros are currently a little bit broken in mbed SDK
 * current version of the EthernetInterface library is 39:097a9996f8d5
 * - fnctl() seem to had been removed as some code referes to standard
 * library version of this system call
 * - if forced to be anabled, C++ code breaks as it expects select() to be a function
 * as this is really not official supported, reporting this upstream is
 * not likelly to get resolved and we are not intending to use lwip
 * directly on mbed anyway, so a work-around is sort of acceptable */
#define accept( a, b, c ) lwip_accept( a, b, c )
#define bind( a, b, c ) lwip_bind( a, b, c )
#define shutdown( a, b ) lwip_shutdown( a, b )
#define closesocket( s ) lwip_close( s )
#define connect( a, b, c ) lwip_connect( a, b, c )
#define getsockname( a, b, c ) lwip_getsockname( a, b, c )
#define getpeername( a, b, c ) lwip_getpeername( a, b, c )
#define setsockopt( a, b, c, d, e ) lwip_setsockopt( a, b, c, d, e )
#define getsockopt( a, b, c, d, e ) lwip_getsockopt( a, b, c, d, e )
#define listen( a, b ) lwip_listen( a, b )
#define recv( a, b, c, d ) lwip_recv( a, b, c, d )
#define recvfrom( a, b, c, d, e, f ) lwip_recvfrom( a, b, c, d, e, f )
#define send( a, b, c, d ) lwip_send( a, b, c, d )
#define sendto( a, b, c, d, e, f ) lwip_sendto( a, b, c, d, e, f )
#define socket( a, b, c ) lwip_socket( a, b, c )
#define select( a, b, c, d, e ) lwip_select( a, b, c, d, e )
#define ioctlsocket( a, b, c ) lwip_ioctl( a, b, c )
#define read( a, b, c ) lwip_read( a, b, c )
#define write( a, b, c ) lwip_write( a, b, c )
#define close( s ) lwip_close( s )
#define fcntl( a, b, c ) lwip_fcntl( a, b, c )
#define gethostbyname( name ) lwip_gethostbyname( name )
#else /* normal order */
#define LWIP_COMPAT_SOCKETS 1
#define LWIP_POSIX_SOCKETS_IO_NAMES 1
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#endif
#include <lwip/err.h>
#define xi_io_get_error_string( e ) lwip_strerr( e )
#elif XI_IO_LAYER_POSIX_COMPAT == 2
#include <socket.h>
#undef socket
#undef close
#undef accept
#undef bind
#undef listen
#undef connect
#undef select
#undef setsockopt
#undef getsockopt
#undef recv
#undef recvfrom
#undef write
#undef send
#undef sendto
#undef gethostbyname
#undef htonl
#undef ntohl
#undef htons
#undef ntohs
#define accept( a, b, c ) sl_Accept( a, b, c )
#define bind( a, b, c ) sl_Bind( a, b, c )
#define shutdown( a, b ) sl_Shutdown( a, b )
#define closesocket( s ) sl_Close( s )
#define connect( a, b, c ) sl_Connect( a, b, c )
#define getsockname( a, b, c ) sl_Getsockname( a, b, c )
#define getpeername( a, b, c ) sl_Getpeername( a, b, c )
#define setsockopt( a, b, c, d, e ) sl_Setsockopt( a, b, c, d, e )
#define getsockopt( a, b, c, d, e ) sl_Getsockopt( a, b, c, d, e )
#define listen( a, b ) sl_Listen( a, b )
#define recv( a, b, c, d ) sl_Recv( a, b, c, d )
#define recvfrom( a, b, c, d, e, f ) sl_Recvfrom( a, b, c, d, e, f )
#define send( a, b, c, d ) sl_Send( a, b, c, d )
#define sendto( a, b, c, d, e, f ) sl_Sendto( a, b, c, d, e, f )
#define socket( a, b, c ) sl_Socket( a, b, c )
#define select( a, b, c, d, e ) sl_Select( a, b, c, d, e )
#define ioctlsocket( a, b, c ) sl_Ioctl( a, b, c )
#define read( a, b, c ) sl_Recv( a, b, c, 0 )
#define write( a, b, c ) sl_Send( a, b, c, 0 )
#define close( s ) sl_Close( s )

#define gethostbyname( a ) sl_NetAppDnsGetHostByName( a )
#define htonl( a ) sl_Htonl( a )
#define ntohl( a ) sl_Ntohl( a )
#define htons( a ) sl_Htons( a )
#define ntohs( a ) sl_Ntohs( a )

#elif XI_IO_LAYER_POSIX_COMPAT == 3
#define LWIP_COMPAT_SOCKETS 1
#define LWIP_POSIX_SOCKETS_IO_NAMES 1
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include <lwip/err.h>
#define xi_io_get_error_string( e ) lwip_strerr( e )
#endif

#endif /* __XI_IO_POSIX_LAYER_COMPAT_H__ */
