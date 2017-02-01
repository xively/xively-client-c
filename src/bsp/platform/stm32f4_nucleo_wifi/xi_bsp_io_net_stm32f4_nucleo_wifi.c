/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <stdio.h>
#include <xi_bsp_io_net.h>
#include <xi_data_desc.h>
#include "wifi_interface.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX
#define MAX( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#endif

typedef enum 
{
    xi_wifi_state_connected = 0,
    xi_wifi_state_disconnected,
    xi_wifi_state_error,

} xi_bsp_stm_wifi_state_t;

typedef struct _xi_bsp_stm_net_state_t
{
    xi_data_desc_t* head;
    xi_bsp_stm_wifi_state_t wifi_state;

} xi_bsp_stm_net_state_t;

xi_bsp_stm_net_state_t xi_net_state = {0};

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

    if ( xi_net_state.wifi_state == xi_wifi_state_connected )
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

    if ( xi_net_state.wifi_state == xi_wifi_state_connected )
    {
        if ( NULL == xi_net_state.head )
        {
            return XI_BSP_IO_NET_STATE_BUSY;
        }
        else
        {
            xi_data_desc_t* head = xi_net_state.head;

            size_t bytes_available = head->length - head->curr_pos;
            size_t bytes_to_copy = bytes_available > count ? count : bytes_available;

            memcpy( buf, head->data_ptr + head->curr_pos , bytes_to_copy );

            head->curr_pos += bytes_to_copy;
            *out_read_count = bytes_to_copy;

            if ( head->curr_pos == head->length ) 
            {
                xi_data_desc_t* temp = head;
                xi_net_state.head = head->__next;

                xi_free_desc( &temp );
            }
        }
    }
    else if ( xi_net_state.wifi_state == xi_wifi_state_error )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }
    else if ( xi_net_state.wifi_state == xi_wifi_state_connected )
    {
        return XI_BSP_IO_NET_STATE_CONNECTION_RESET;
    }

    return XI_BSP_IO_NET_STATE_OK;
}

xi_bsp_io_net_state_t xi_bsp_io_net_close_socket( xi_bsp_socket_t* xi_socket )
{
    WiFi_Status_t status = WiFi_MODULE_SUCCESS;
    status               = wifi_socket_client_close( *xi_socket );

    /* free all buffers */

    xi_data_desc_t* head = xi_net_state.head;
    while ( NULL != head )
    {
        xi_data_desc_t* temp = head;
        head = head->__next;
        xi_free_desc( &temp );
    }

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

    if ( xi_net_state.wifi_state == xi_wifi_state_connected )
    {
        for ( socket_id = 0; socket_id < socket_events_array_size; ++socket_id )
        {
            xi_bsp_socket_events_t* socket_events = &socket_events_array[socket_id];

            if ( 1 == socket_events->in_socket_want_read )
            {
                if ( NULL != xi_net_state.head )
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
    ( void )socket_id;
    ( void )message_size;

    xi_data_desc_t* tail = xi_make_desc_from_buffer_copy( data_ptr , chunk_size );

    if ( NULL == xi_net_state.head ) xi_net_state.head = tail;
    else
    {
        xi_data_desc_t* last = xi_net_state.head;
        while ( NULL != last->__next ) last = last->__next;
        last->__next = tail;
    }
}

void xi_bsp_io_net_socket_client_remote_server_closed_proxy( uint8_t* socket_closed_id )
{
    ( void )socket_closed_id;
    xi_net_state.wifi_state = xi_wifi_state_disconnected;
}