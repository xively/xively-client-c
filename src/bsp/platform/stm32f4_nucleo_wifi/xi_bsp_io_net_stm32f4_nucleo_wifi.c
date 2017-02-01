/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <stdio.h>
#include <string.h>
#include <xi_bsp_io_net.h>
#include "xi_debug.h"
#include "wifi_interface.h"
#include "xi_bsp_time_stm32f4_nucleo_wifi_sntp.h"

#ifdef __cplusplus
extern "C" {
#endif 
#ifndef MAX
#define MAX( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#endif

#define XI_BSP_IO_NET_BUFFER_SIZE 512

typedef enum {
    xi_wifi_state_connected = 0,
    xi_wifi_state_disconnected,
    xi_wifi_state_error,
} xi_bsp_stm_wifi_state_t;

typedef struct _xi_bsp_stm_wifi_buffer_t
{
    uint8_t bytes[XI_BSP_IO_NET_BUFFER_SIZE];
    uint32_t size;
} xi_bsp_stm_wifi_buffer_t;

xi_bsp_stm_wifi_state_t xi_wifi_state   = xi_wifi_state_connected;
xi_bsp_stm_wifi_buffer_t xi_wifi_buffer = {0};

xi_bsp_io_net_state_t xi_bsp_io_net_create_socket( xi_bsp_socket_t* xi_socket )
{
    *xi_socket = 0;

    return XI_BSP_IO_NET_STATE_OK;
}

xi_bsp_io_net_state_t
xi_bsp_io_net_connect( xi_bsp_socket_t* xi_socket, const char* host, uint16_t port )
{
    char* protocol = "s"; // t -> tcp , s-> secure tcp, c-> secure tcp with certs

    WiFi_Status_t status = WiFi_MODULE_SUCCESS;
    status = wifi_socket_client_open( ( uint8_t* )host, port,
                                      ( uint8_t* )protocol, ( uint8_t* )xi_socket );

    if ( status == WiFi_MODULE_SUCCESS )
    {
        return XI_BSP_IO_NET_STATE_OK;
    }
    else
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }
}

xi_bsp_io_net_state_t xi_bsp_io_net_connection_check( xi_bsp_socket_t xi_socket,
                                                      const char* host,
                                                      uint16_t port )
{
    ( void )xi_socket;
    ( void )host;
    ( void )port;

    if ( xi_wifi_state == xi_wifi_state_connected )
    {
        return XI_BSP_IO_NET_STATE_OK;
    }
    else
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }
}

xi_bsp_io_net_state_t xi_bsp_io_net_write( xi_bsp_socket_t xi_socket,
                                           int* out_written_count,
                                           const uint8_t* buf,
                                           size_t count )
{
    *out_written_count = count;

    WiFi_Status_t status = WiFi_MODULE_SUCCESS;
    status               = wifi_socket_client_write( xi_socket, count, ( char* )buf );

    if ( status == WiFi_MODULE_SUCCESS )
    {
        return XI_BSP_IO_NET_STATE_OK;
    }
    else
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }
}

xi_bsp_io_net_state_t xi_bsp_io_net_read( xi_bsp_socket_t xi_socket,
                                          int* out_read_count,
                                          uint8_t* buf,
                                          size_t count )
{
    ( void )xi_socket;

    *out_read_count = 0;

    if ( xi_wifi_state == xi_wifi_state_connected )
    {
        if ( xi_wifi_buffer.size == 0 )
        {
            return XI_BSP_IO_NET_STATE_BUSY;
        }
        else if ( xi_wifi_buffer.size > count )
        {
            return XI_BSP_IO_NET_STATE_ERROR;
        }
        else
        {
            memcpy( buf, xi_wifi_buffer.bytes, xi_wifi_buffer.size );
            *out_read_count     = xi_wifi_buffer.size;
            xi_wifi_buffer.size = 0;
        }
    }
    else if ( xi_wifi_state == xi_wifi_state_error )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }
    else if ( xi_wifi_state == xi_wifi_state_connected )
    {
        return XI_BSP_IO_NET_STATE_CONNECTION_RESET;
    }

    return XI_BSP_IO_NET_STATE_OK;
}

xi_bsp_io_net_state_t xi_bsp_io_net_close_socket( xi_bsp_socket_t* xi_socket )
{
    WiFi_Status_t status = WiFi_MODULE_SUCCESS;
    status               = wifi_socket_client_close( *xi_socket );

    if ( status == WiFi_MODULE_SUCCESS )
    {
        return XI_BSP_IO_NET_STATE_OK;
    }
    else
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }
}

xi_bsp_io_net_state_t xi_bsp_io_net_select( xi_bsp_socket_events_t* socket_events_array,
                                            size_t socket_events_array_size,
                                            long timeout_sec )
{
    ( void )timeout_sec;

    size_t socket_id = 0;

    if ( xi_wifi_state == xi_wifi_state_connected )
    {
        for ( socket_id = 0; socket_id < socket_events_array_size; ++socket_id )
        {
            xi_bsp_socket_events_t* socket_events = &socket_events_array[socket_id];

            if ( 1 == socket_events->in_socket_want_read )
            {
                if ( xi_wifi_buffer.size > 0 )
                {
                    socket_events->out_socket_can_read = 1;
                }
            }

            if ( 1 == socket_events->in_socket_want_connect )
            {
                socket_events->out_socket_connect_finished = 1;
            }

            if ( 1 == socket_events->in_socket_want_write )
            {
                socket_events->out_socket_can_write = 1;
            }
        }

        return XI_BSP_IO_NET_STATE_OK;
    }
    else
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }
}

void xi_bsp_io_net_socket_data_received_proxy( uint8_t socket_id,
                                               uint8_t* data_ptr,
                                               uint32_t message_size,
                                               uint32_t chunk_size )
{
    if(socket_id == sntp_sock_id)
    {
        xi_debug_printf("\r\n>>Received message from SNTP socket");
        sntp_socket_data_callback(socket_id, data_ptr, message_size, chunk_size);
        return;
    }

    if ( xi_wifi_buffer.size == 0 )
    {
        if ( chunk_size > XI_BSP_IO_NET_BUFFER_SIZE )
        {
            // payload is too big
            xi_wifi_state = xi_wifi_state_error;
        }
        else
        {
            memcpy( xi_wifi_buffer.bytes, data_ptr, message_size );
            xi_wifi_buffer.size = chunk_size;
        }
    }
    else
    {
        // previous data is not processed yet
        xi_wifi_state = xi_wifi_state_error;
    }
}

void xi_bsp_io_net_socket_client_remote_server_closed_proxy( uint8_t* socket_closed_id )
{
    if(*socket_closed_id == sntp_sock_id)
    {
        xi_debug_printf("\r\n\tUnexpected 'remote disconnection' by SNTP server");
        return;
    }

    xi_wifi_state = xi_wifi_state_disconnected;
}
