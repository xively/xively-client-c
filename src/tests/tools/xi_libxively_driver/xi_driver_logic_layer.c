// Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
//
// This is part of the Xively C Client library,
// it is licensed under the BSD 3-Clause license.

#include "xi_driver_logic_layer.h"
#include "xi_layer_macros.h"
#include "xi_debug.h"
#include "xi_layer_api.h"
#include "xi_globals.h"
#include "xively.h"
#include "xi_control_channel_protocol.pb-c.h"
#include "xi_libxively_driver_impl.h"
#include "xi_helpers.h"

#ifdef XI_MULTI_LEVEL_DIRECTORY_STRUCTURE
#include "errno.h"
#include "unistd.h"
#endif

/****************************************************************************
 * libxively callbacks ******************************************************
 ****************************************************************************/
void on_connected( xi_context_handle_t context_handle,
                   void* data,
                   xi_state_t in_out_state )
{
    XI_UNUSED( data );
    XI_UNUSED( in_out_state );
    XI_UNUSED( context_handle );

    xi_connection_data_t* conn_data = ( xi_connection_data_t* )data;

    printf( "[ driver lgc ] %s, result = %d, connection_state = %d\n", __func__,
            in_out_state, conn_data->connection_state );

    switch ( conn_data->connection_state )
    {
        case XI_CONNECTION_STATE_OPENED:
        case XI_CONNECTION_STATE_OPEN_FAILED:
            xi_libxively_driver_send_on_connect_finish( libxively_driver, in_out_state );
            break;
        case XI_CONNECTION_STATE_CLOSED:
            xi_libxively_driver_send_on_disconnect( libxively_driver, in_out_state );
            break;
        default:
            break;
    }
}

void on_message_received( xi_context_handle_t context_handle,
                          xi_sub_call_type_t call_type,
                          const xi_sub_call_params_t* const params,
                          xi_state_t in_out_state,
                          void* user_data )
{
    XI_UNUSED( user_data );
    XI_UNUSED( params );
    XI_UNUSED( in_out_state );
    XI_UNUSED( context_handle );

    printf( "[ driver lgc ] %s, state = %d, call_type = %d, user = %d\n", __func__,
            in_out_state, call_type, ( int )( intptr_t )user_data );

    switch ( call_type )
    {
        case XI_SUB_CALL_SUBACK:
        case XI_SUB_CALL_MESSAGE:
            xi_libxively_driver_send_on_message_received( libxively_driver, call_type,
                                                          params, in_out_state );
            break;
        default:
            break;
    }
}

void on_publish_finish( xi_context_handle_t context, void* data, xi_state_t in_out_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_out_state );

    printf( "[ driver lgc ] %s, state = %d, data = %p\n", __func__, in_out_state, data );

    xi_libxively_driver_send_on_publish_finish( libxively_driver, data, in_out_state );
}

/****************************************************************************
 * driver layer functions ***************************************************
 ****************************************************************************/
xi_state_t
xi_driver_logic_layer_push( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) )
    {
        goto err_handling;
    }

    if ( NULL == data )
    {
        return in_out_state;
    }
    else
    {
        return XI_PROCESS_PUSH_ON_PREV_LAYER( context, data, in_out_state );
    }

err_handling:
    return XI_PROCESS_CLOSE_ON_THIS_LAYER( context, NULL, in_out_state );
}

xi_state_t
xi_driver_logic_layer_pull( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( context );

    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) || NULL == data )
    {
        goto err_handling;
    }

    struct _XiClientFtestFw__XiClientAPI* message_API_call =
        ( struct _XiClientFtestFw__XiClientAPI* )data;

    printf( "[ driver lgc ] *** connect                      = %p\n",
            message_API_call->connect );
    printf( "[ driver lgc ] *** disconnect                   = %p\n",
            message_API_call->disconnect );
    printf( "[ driver lgc ] *** subscribe                    = %p\n",
            message_API_call->subscribe );
    printf( "[ driver lgc ] *** publish_string               = %p\n",
            message_API_call->publish_string );
    printf( "[ driver lgc ] *** publish_binary               = %p\n",
            message_API_call->publish_binary );
    printf( "[ driver lgc ] *** publish_timeseries           = %p\n",
            message_API_call->publish_timeseries );
    printf( "[ driver lgc ] *** publish_formatted_timeseries = %p\n",
            message_API_call->publish_formatted_timeseries );
    printf( "[ driver lgc ] *** setup_tls                    = %p\n",
            message_API_call->setup_tls );

    if ( NULL != message_API_call->connect )
    {
        printf( "[ driver lgc ] --- connecting libxively to [ %s : %d ]\n",
                message_API_call->connect->server_address->host,
                message_API_call->connect->server_address->port );

        xi_connect_to(
            xi_globals.default_context_handle,
            message_API_call->connect->server_address->host,
            message_API_call->connect->server_address->port,
            message_API_call->connect->username, message_API_call->connect->password,
            message_API_call->connect->has_connection_timeout
                ? message_API_call->connect->connection_timeout
                : 10,
            20,
            message_API_call->connect->has_mqtt_session_type &&
                    XI_CLIENT_FTEST_FW__XI_CLIENT_API____MQTT_SESSION_TYPE__UNCLEAN_SESSION ==
                        message_API_call->connect->mqtt_session_type
                ? XI_SESSION_CONTINUE
                : XI_SESSION_CLEAN,
            &on_connected );

        // use buffer allocated by protobuf
        message_API_call->connect->username = NULL;
        message_API_call->connect->password = NULL;
    }
    else if ( NULL != message_API_call->disconnect )
    {
        printf( "[ driver lgc ] --- disconnecting\n" );
        xi_shutdown_connection( xi_globals.default_context_handle );
    }
    else if ( NULL != message_API_call->subscribe )
    {
        printf( "[ driver lgc ] --- subscribing\n" );

        uint8_t topic_id = 0;
        for ( ; topic_id < message_API_call->subscribe->n_topic_qos_list; ++topic_id )
        {
            xi_state_t state = xi_subscribe(
                xi_globals.default_context_handle,
                message_API_call->subscribe->topic_qos_list[topic_id]->topic_name,
                message_API_call->subscribe->topic_qos_list[topic_id]->qos,
                &on_message_received, ( void* )( intptr_t )topic_id );

            printf( "[ driver lgc ] --- subscribe state = %d\n", state );
        }
    }
    else if ( NULL != message_API_call->publish_string )
    {
        printf( "[ driver lgc ] --- publish string\n" );

        xi_publish( xi_globals.default_context_handle,
                    message_API_call->publish_string->publish_common_data->topic_name,
                    message_API_call->publish_string->payload,
                    message_API_call->publish_string->publish_common_data->qos,
                    message_API_call->publish_string->retain, &on_publish_finish, NULL );

        message_API_call->publish_string->payload = NULL;
    }
    else if ( NULL != message_API_call->publish_binary )
    {
        printf( "[ driver lgc ] --- publish binary\n" );

        xi_publish_data(
            xi_globals.default_context_handle,
            message_API_call->publish_binary->publish_common_data->topic_name,
            message_API_call->publish_binary->payload.data,
            message_API_call->publish_binary->payload.len,
            message_API_call->publish_binary->publish_common_data->qos,
            XI_MQTT_RETAIN_FALSE, &on_publish_finish, NULL );

        message_API_call->publish_binary->has_payload  = 0;
        message_API_call->publish_binary->payload.data = NULL;
        message_API_call->publish_binary->payload.len  = 0;
    }
    else if ( NULL != message_API_call->setup_tls )
    {
#ifdef XI_MULTI_LEVEL_DIRECTORY_STRUCTURE
        const int16_t cwd_buffer_size = 512;
        char cwd_buffer[512]          = {0};

        if ( NULL == getcwd( cwd_buffer, cwd_buffer_size ) )
        {
            xi_debug_format( "[ driver lgc ] ERROR: could not get current working "
                             "directory, errno = %d",
                             errno );
            in_out_state = XI_INTERNAL_ERROR;
            goto err_handling;
        }

        size_t cwd_length          = strlen( cwd_buffer );
        const char* libxively_cwds = "/libxively_cwds/";

        xi_str_copy_untiln( cwd_buffer + cwd_length, cwd_buffer_size - cwd_length,
                            libxively_cwds, '\0' );

        cwd_length += strlen( libxively_cwds );

        xi_str_copy_untiln( cwd_buffer + cwd_length, cwd_buffer_size - cwd_length,
                            message_API_call->setup_tls->ca_cert_file
                                ? message_API_call->setup_tls->ca_cert_file
                                : "",
                            '.' );

        if ( 0 != chdir( cwd_buffer ) )
        {
            xi_debug_format(
                "[ driver lgc ] ERROR: could not change dir to %s, errno = %d",
                cwd_buffer, errno );
            in_out_state = XI_INTERNAL_ERROR;
            goto err_handling;
        }

        xi_debug_format(
            "[ driver lgc ] successfully changed current working directory to %s",
            cwd_buffer );
#endif
    }
    else
    {
        printf( "[ driver lgc ] ERROR: unrecognized control channel message, "
                "disconnecting\n" );
        xi_shutdown_connection( xi_globals.default_context_handle );
    }

    xi_client_ftest_fw__xi_client_api__free_unpacked( message_API_call, NULL );

    return in_out_state;

err_handling:

    return XI_PROCESS_CLOSE_ON_THIS_LAYER( context, NULL, in_out_state );
}

xi_state_t
xi_driver_logic_layer_close( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    return XI_PROCESS_CLOSE_ON_PREV_LAYER( context, data, in_out_state );
}

xi_state_t xi_driver_logic_layer_close_externally( void* context,
                                                   void* data,
                                                   xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( data );
    XI_UNUSED( in_out_state );

    xi_evtd_stop( XI_CONTEXT_DATA( context )->evtd_instance );

    return XI_STATE_OK;
}

xi_state_t
xi_driver_logic_layer_init( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    return XI_PROCESS_INIT_ON_PREV_LAYER( context, data, in_out_state );
}

xi_state_t
xi_driver_logic_layer_connect( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( context );
    XI_UNUSED( data );

    return in_out_state;
}
