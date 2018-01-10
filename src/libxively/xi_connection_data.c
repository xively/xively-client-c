/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_config.h"
#include "xi_event_dispatcher_api.h"
#include "xi_connection_data.h"
#include "xi_helpers.h"

xi_connection_data_t* xi_alloc_connection_data( const char* host,
                                                uint16_t port,
                                                const char* username,
                                                const char* password,
                                                uint16_t connection_timeout,
                                                uint16_t keepalive_timeout,
                                                xi_session_type_t session_type )
{
    return xi_alloc_connection_data_lastwill(
        host, port, username, password, connection_timeout, keepalive_timeout,
        session_type, NULL, NULL, ( xi_mqtt_qos_t )0, ( xi_mqtt_retain_t )0 );
}

xi_connection_data_t* xi_alloc_connection_data_lastwill( const char* host,
                                                         uint16_t port,
                                                         const char* username,
                                                         const char* password,
                                                         uint16_t connection_timeout,
                                                         uint16_t keepalive_timeout,
                                                         xi_session_type_t session_type,
                                                         const char* will_topic,
                                                         const char* will_message,
                                                         xi_mqtt_qos_t will_qos,
                                                         xi_mqtt_retain_t will_retain )
{
    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_connection_data_t, ret, state );

    if ( host )
    {
        ret->host = xi_str_dup( host );
        XI_CHECK_MEMORY( ret->host, state );
    }

    if ( username )
    {
        ret->username = xi_str_dup( username );
        XI_CHECK_MEMORY( ret->username, state );
    }

    if ( password )
    {
        ret->password = xi_str_dup( password );
        XI_CHECK_MEMORY( ret->password, state );
    }

    if ( will_topic )
    {
        ret->will_topic = xi_str_dup( will_topic );
        XI_CHECK_MEMORY( ret->will_topic, state );
    }

    if ( will_message )
    {
        ret->will_message = xi_str_dup( will_message );
        XI_CHECK_MEMORY( ret->will_message, state );
    }

    ret->port               = port;
    ret->connection_timeout = connection_timeout;
    ret->keepalive_timeout  = keepalive_timeout;
    ret->session_type       = session_type;
    ret->will_qos           = will_qos;
    ret->will_retain        = will_retain;

    return ret;

err_handling:
    if ( ret )
    {
        XI_SAFE_FREE( ret->host );
        XI_SAFE_FREE( ret->username );
        XI_SAFE_FREE( ret->password );
        XI_SAFE_FREE( ret->will_topic );
        XI_SAFE_FREE( ret->will_message );
    }
    XI_SAFE_FREE( ret );
    return 0;
}

xi_state_t xi_connection_data_update( xi_connection_data_t* conn_data,
                                      const char* host,
                                      uint16_t port,
                                      const char* username,
                                      const char* password,
                                      uint16_t connection_timeout,
                                      uint16_t keepalive_timeout,
                                      xi_session_type_t session_type )
{
    return xi_connection_data_update_lastwill(
        conn_data, host, port, username, password, connection_timeout, keepalive_timeout,
        session_type, NULL, NULL, ( xi_mqtt_qos_t )0, ( xi_mqtt_retain_t )0 );
}

xi_state_t xi_connection_data_update_lastwill( xi_connection_data_t* conn_data,
                                               const char* host,
                                               uint16_t port,
                                               const char* username,
                                               const char* password,
                                               uint16_t connection_timeout,
                                               uint16_t keepalive_timeout,
                                               xi_session_type_t session_type,
                                               const char* will_topic,
                                               const char* will_message,
                                               xi_mqtt_qos_t will_qos,
                                               xi_mqtt_retain_t will_retain )
{
    xi_state_t local_state = XI_STATE_OK;

    if ( NULL != host && strcmp( host, conn_data->host ) != 0 )
    {
        XI_SAFE_FREE( conn_data->host );
        conn_data->host = xi_str_dup( host );
        XI_CHECK_MEMORY( conn_data->host, local_state );
    }

    if ( NULL != username && strcmp( username, conn_data->username ) != 0 )
    {
        XI_SAFE_FREE( conn_data->username );
        conn_data->username = xi_str_dup( username );
        XI_CHECK_MEMORY( conn_data->username, local_state );
    }

    if ( NULL != password && strcmp( password, conn_data->password ) != 0 )
    {
        XI_SAFE_FREE( conn_data->password );
        conn_data->password = xi_str_dup( password );
        XI_CHECK_MEMORY( conn_data->password, local_state );
    }

    if ( NULL != will_topic && strcmp( will_topic, conn_data->will_topic ) != 0 )
    {
        XI_SAFE_FREE( conn_data->will_topic );
        conn_data->will_topic = xi_str_dup( will_topic );
        XI_CHECK_MEMORY( conn_data->will_topic, local_state );
    }

    if ( NULL != will_message && strcmp( will_message, conn_data->will_message ) != 0 )
    {
        XI_SAFE_FREE( conn_data->will_message );
        conn_data->will_message = xi_str_dup( will_message );
        XI_CHECK_MEMORY( conn_data->will_message, local_state );
    }

    conn_data->port               = port;
    conn_data->connection_timeout = connection_timeout;
    conn_data->keepalive_timeout  = keepalive_timeout;
    conn_data->session_type       = session_type;
    conn_data->will_qos           = will_qos;
    conn_data->will_retain        = will_retain;

err_handling:
    return local_state;
}

void xi_free_connection_data( xi_connection_data_t** data )
{
    if ( *data )
    {
        XI_SAFE_FREE( ( *data )->host );
        XI_SAFE_FREE( ( *data )->username );
        XI_SAFE_FREE( ( *data )->password );
        XI_SAFE_FREE( ( *data )->will_topic );
        XI_SAFE_FREE( ( *data )->will_message );
    }

    XI_SAFE_FREE( *data );
}
