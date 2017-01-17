/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <stdio.h>
#include <xi_bsp_io_net.h>
#include "wifi_interface.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX
#define MAX( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#endif

typedef enum {
  xi_wifi_state_connected = 0,
  xi_wifi_state_disconnected,
} xi_wifi_state_t;

xi_wifi_state_t xi_wifi_state = xi_wifi_state_connected;

xi_bsp_io_net_state_t xi_bsp_io_net_create_socket( xi_bsp_socket_t* xi_socket )
{
    ( void )xi_socket;

    *xi_socket = 0;

    return XI_BSP_IO_NET_STATE_OK;
}

xi_bsp_io_net_state_t
xi_bsp_io_net_connect( xi_bsp_socket_t* xi_socket, const char* host, uint16_t port )
{
    ( void )xi_socket;
    ( void )host;
    ( void )port;

    char *protocol = "s";//t -> tcp , s-> secure tcp, c-> secure tcp with certs

    WiFi_Status_t status = WiFi_MODULE_SUCCESS;
    status = wifi_socket_client_open((uint8_t *)"broker.dev.xively.io", port, (uint8_t *)protocol, (uint8_t *)xi_socket);

    if ( status == WiFi_MODULE_SUCCESS ) return XI_BSP_IO_NET_STATE_OK;
    else return XI_BSP_IO_NET_STATE_ERROR;
}

xi_bsp_io_net_state_t xi_bsp_io_net_connection_check( xi_bsp_socket_t xi_socket,
                                                      const char* host,
                                                      uint16_t port )
{
    ( void )xi_socket;
    ( void )host;
    ( void )port;

    if ( xi_wifi_state == xi_wifi_state_connected ) return XI_BSP_IO_NET_STATE_OK;
    else return XI_BSP_IO_NET_STATE_ERROR;
}

xi_bsp_io_net_state_t xi_bsp_io_net_write( xi_bsp_socket_t xi_socket,
                                           int* out_written_count,
                                           const uint8_t* buf,
                                           size_t count )
{
    ( void )xi_socket;
    ( void )buf;
    *out_written_count = count;

    WiFi_Status_t status = WiFi_MODULE_SUCCESS;
    status = wifi_socket_client_write(xi_socket, count, (char*) buf);

    if ( status == WiFi_MODULE_SUCCESS ) return XI_BSP_IO_NET_STATE_OK;
    else return XI_BSP_IO_NET_STATE_ERROR;
}


uint8_t read_chunk[100];
uint32_t read_size = 0;


xi_bsp_io_net_state_t xi_bsp_io_net_read( xi_bsp_socket_t xi_socket,
                                          int* out_read_count,
                                          uint8_t* buf,
                                          size_t count )
{
    ( void )xi_socket;
    ( void )buf;
    ( void )count;

    *out_read_count = 0;

    if ( xi_wifi_state == xi_wifi_state_connected )
    {

      if ( read_size > 0 )
      {
          memcpy( buf , read_chunk , read_size );
          *out_read_count = read_size;
          read_size = 0;

          return XI_BSP_IO_NET_STATE_OK;
      }
      else return XI_BSP_IO_NET_STATE_BUSY;

    }
    else return XI_BSP_IO_NET_STATE_CONNECTION_RESET;    
}

xi_bsp_io_net_state_t xi_bsp_io_net_close_socket( xi_bsp_socket_t* xi_socket )
{
    WiFi_Status_t status = WiFi_MODULE_SUCCESS;
    status = wifi_socket_client_close( *xi_socket );

    if ( status == WiFi_MODULE_SUCCESS ) return XI_BSP_IO_NET_STATE_OK;
    else return XI_BSP_IO_NET_STATE_ERROR;
}

xi_bsp_io_net_state_t xi_bsp_io_net_select( xi_bsp_socket_events_t* socket_events_array,
                                            size_t socket_events_array_size,
                                            long timeout_sec )
{
    (void)timeout_sec;

    fd_set rfds;
    fd_set wfds;
    fd_set efds;

    FD_ZERO( &rfds );
    FD_ZERO( &wfds );
    FD_ZERO( &efds );

    size_t socket_id = 0;

    if ( xi_wifi_state == xi_wifi_state_connected )
    {

        for ( socket_id = 0; socket_id < socket_events_array_size; ++socket_id )
        {
            xi_bsp_socket_events_t* socket_events = &socket_events_array[socket_id];

            if ( 1 == socket_events->in_socket_want_read )
            {
                if ( read_size > 0 ) socket_events->out_socket_can_read = 1;
            }

            if ( 1 == socket_events->in_socket_want_connect )
            {
                socket_events->out_socket_connect_finished = 1;
            }

            if ( 1 == socket_events->in_socket_want_write )
            {
                socket_events->out_socket_can_write = 1;
            }

            // if ( FD_ISSET( socket_events->socket, &efds ) )
            // {
            //     socket_events->out_socket_error = 1;
            // }
        }

        return XI_BSP_IO_NET_STATE_OK;

    }
    else
    {
      return XI_BSP_IO_NET_STATE_ERROR;
    }
  }


void ind_wifi_socket_data_received(uint8_t socket_id, uint8_t * data_ptr, uint32_t message_size, uint32_t chunk_size)
{
  (void) socket_id;
  if ( read_size == 0 )
  {
      memcpy( read_chunk , data_ptr, message_size );
      read_size = chunk_size;
  }
}

void ind_wifi_socket_client_remote_server_closed(uint8_t * socket_closed_id)
{
  (void) socket_closed_id;
  xi_wifi_state = xi_wifi_state_disconnected;
}