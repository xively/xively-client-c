/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <stdio.h>
#include <string.h>
#include <xi_bsp_io_net.h>
#include <assert.h>
#include "xi_bsp_debug.h"
#include <xi_data_desc.h>
#include <xi_list.h>
#include "wifi_interface.h"
#include "xi_bsp_time_stm32fx_nucleo_wifi_sntp.h"
#include "xi_bsp_tls_certs.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX
#define MAX( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#endif

#ifndef MIN
#define MIN( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#endif

typedef enum {
    xi_wifi_state_connected = 0,
    xi_wifi_state_disconnected,
} xi_bsp_stm_wifi_state_t;

typedef struct _xi_bsp_stm_net_state_t
{
    xi_data_desc_t* head;
    xi_bsp_stm_wifi_state_t wifi_state;
} xi_bsp_stm_net_state_t;

/* this is local variable */
static xi_bsp_stm_net_state_t xi_net_state = {NULL, xi_wifi_state_disconnected};

xi_bsp_io_net_state_t xi_bsp_io_net_create_socket( xi_bsp_socket_t* xi_socket )
{
    /* sanity check */
    assert( NULL == xi_net_state.head );

    *xi_socket = 0;

    return XI_BSP_IO_NET_STATE_OK;
}

static xi_bsp_io_net_state_t xi_bsp_io_net_configure_tls( const char* host )
{
    WiFi_Status_t status          = WiFi_MODULE_SUCCESS;
    posix_time_t current_datetime = xi_bsp_time_sntp_getseconds_posix();
    //const uint8_t* tls_mode       = ( uint8_t* )"o"; /* ["m"utual || "o"ne-way] */
    //const char* tls_cert          = GLOBALSIGN_ROOT_CERT;

    TLS_Certificate stm_cube_tls_certificate;
    stm_cube_tls_certificate.certificate = ( uint8_t* )GLOBALSIGN_ROOT_CERT;
    stm_cube_tls_certificate.certificate_size = sizeof(GLOBALSIGN_ROOT_CERT);

    TLS_Certificate stm_cube_empty_certificate;
    memset( &stm_cube_empty_certificate, 0, sizeof( TLS_Certificate) );

    TLS_Certificate stm_cube_client_domain;
    stm_cube_client_domain.certificate = ( uint8_t* )host;
    stm_cube_client_domain.certificate_size = strlen( host );
    memset( &stm_cube_empty_certificate, 0, sizeof( TLS_Certificate) );

    

    //wifi_socket_client_security(uint8_t* tls_mode, uint8_t* root_ca_server, uint8_t* client_cert, uint8_t* client_key, uint8_t* client_domain, uint32_t tls_epoch_time):
    /*status = wifi_socket_client_security( ( uint8_t* )tls_mode, ( uint8_t* )tls_cert,
                                          NULL, NULL, ( uint8_t* )host,
                                          ( uint32_t )current_datetime );*/

    status = wifi_set_socket_certificates( stm_cube_empty_certificate, /* ca_certificate */
    									   stm_cube_tls_certificate,   /* tls_certificate */
										   stm_cube_empty_certificate, /* certficate_key */
										   stm_cube_client_domain,     /* client_domain */
										   ( uint32_t )current_datetime );

    if ( WiFi_MODULE_SUCCESS != status )
    {
        xi_bsp_debug_format( "TLS configuration [ERROR] %d", status );
        return XI_BSP_IO_NET_STATE_ERROR;
    }
    xi_bsp_debug_format( "TLS configuration [OK] %d", status );
    return XI_BSP_IO_NET_STATE_OK;
}

xi_bsp_io_net_state_t
xi_bsp_io_net_connect( xi_bsp_socket_t* xi_socket, const char* host, uint16_t port )
{
    const char* protocol = "s"; /* t -> tcp , s-> secure tcp, c-> secure tcp with certs */
    WiFi_Status_t status = WiFi_MODULE_SUCCESS;

    if ( XI_BSP_IO_NET_STATE_OK != xi_bsp_io_net_configure_tls( host ) )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    status = wifi_socket_client_open( ( uint8_t* )host, port, ( uint8_t* )protocol,
                                      ( uint8_t* )xi_socket );

    if ( status == WiFi_MODULE_SUCCESS )
    {
        /* reset the wifi state */
        xi_net_state.wifi_state = xi_wifi_state_connected;
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

            const size_t bytes_available = head->length - head->curr_pos;

            /* sanity check */
            assert( 0 != bytes_available );

            /* count is the limit of bytes xively can process*/
            const size_t bytes_to_copy = MIN( bytes_available, count );

            /* copy the data from the temporary buffer to the xively's receive buffer */
            memcpy( buf, head->data_ptr + head->curr_pos, bytes_to_copy );

            /* remember how many bytes were taken from the buffer */
            head->curr_pos += bytes_to_copy;

            /* set the return parameter */
            *out_read_count = bytes_to_copy;

            /* if the whole temporary buffer was taken out release it */
            if ( head->curr_pos == head->length )
            {
                XI_LIST_POP( xi_data_desc_t, xi_net_state.head, head );
                xi_free_desc( &head );
            }
        }
    }
    else if ( xi_net_state.wifi_state == xi_wifi_state_disconnected )
    {
        return XI_BSP_IO_NET_STATE_CONNECTION_RESET;
    }

    return XI_BSP_IO_NET_STATE_OK;
}

xi_bsp_io_net_state_t xi_bsp_io_net_close_socket( xi_bsp_socket_t* xi_socket )
{
    WiFi_Status_t status = WiFi_MODULE_SUCCESS;
    status               = wifi_socket_client_close( *xi_socket );

    /* free all buffers on the list */
    while ( NULL != xi_net_state.head )
    {
        xi_data_desc_t* head = NULL;
        XI_LIST_POP( xi_data_desc_t, xi_net_state.head, head );
        xi_free_desc( &head );
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

void xi_bsp_io_net_socket_data_received_proxy( uint8_t socket_id,
                                               uint8_t* data_ptr,
                                               uint32_t message_size,
                                               uint32_t chunk_size )
{
    ( void )message_size;

    if ( socket_id == sntp_sock_id )
    {
        sntp_socket_data_callback( socket_id, data_ptr, message_size, chunk_size );
        return;
    }

    xi_data_desc_t* tail = xi_make_desc_from_buffer_copy( data_ptr, chunk_size );

    if ( NULL == tail )
    {
        xi_bsp_debug_logger( "Incoming data allocation [ERROR] Not enough memory!" );
        return;
    }

    XI_LIST_PUSH_BACK( xi_data_desc_t, xi_net_state.head, tail );
}

void xi_bsp_io_net_socket_client_remote_server_closed_proxy( uint8_t* socket_closed_id )
{
    if ( NULL == socket_closed_id )
    {
        xi_bsp_debug_logger( "Got invalid NULL as socket_closed_id" );
    }
    if ( *socket_closed_id == sntp_sock_id )
    {
        xi_bsp_debug_logger( "Unexpected 'remote disconnection' by SNTP server" );
        return;
    }

    xi_net_state.wifi_state = xi_wifi_state_disconnected;
}
