// Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
//
// This is part of the Xively C Client library,
// it is licensed under the BSD 3-Clause license.

#include "xi_libxively_driver_impl.h"
#include "xi_driver_control_channel_layerchain.h"
#include "xi_control_channel_protocol.pb-c.h"
#include "xi_globals.h"

xi_libxively_driver_t* libxively_driver = NULL;

// todo: move this function predeclaration to an internal header
xi_state_t
xi_create_context_with_custom_layers_and_evtd( xi_context_t** context,
                                               xi_layer_type_t layer_config[],
                                               xi_layer_type_id_t layer_chain[],
                                               size_t layer_chain_size,
                                               xi_evtd_instance_t* event_dispatcher );

xi_state_t xi_delete_context_with_custom_layers( xi_context_t** context,
                                                 xi_layer_type_t layer_config[],
                                                 size_t layer_chain_size );

xi_state_t
xi_driver_free_protobuf_callback( struct _XiClientFtestFw__XiClientCallback* callback );

xi_libxively_driver_t* xi_libxively_driver_create_instance()
{
    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_libxively_driver_t, driver, state );

    driver->evtd_instance = xi_evtd_create_instance();

    XI_CHECK_CND_DBGMESSAGE(
        driver->evtd_instance == NULL, XI_OUT_OF_MEMORY, state,
        "could not instantiate event dispatcher for libxively driver" );

    XI_CHECK_STATE( xi_create_context_with_custom_layers_and_evtd(
        &driver->context, xi_driver_control_channel_layerchain,
        XI_LAYER_CHAIN_DRIVER_CONTROL_CHANNEL,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_DRIVER_CONTROL_CHANNEL ),
        driver->evtd_instance ) );

    XI_CHECK_CND_DBGMESSAGE( driver->context == NULL, XI_OUT_OF_MEMORY, state,
                             "could not instantiate context for libxively driver" );

    return driver;

err_handling:

    XI_SAFE_FREE( driver );
    return NULL;
}

xi_state_t xi_libxively_driver_destroy_instance( xi_libxively_driver_t** driver )
{
    xi_delete_context_with_custom_layers(
        &( *driver )->context, xi_driver_control_channel_layerchain,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_DRIVER_CONTROL_CHANNEL ) );

    xi_evtd_destroy_instance( ( *driver )->evtd_instance );

    XI_SAFE_FREE( *driver );

    return XI_STATE_OK;
}

xi_state_t xi_libxively_driver_connect_with_callback(
    xi_libxively_driver_t* driver,
    const char* const host,
    uint16_t port,
    xi_libxively_driver_callback_function_t on_connected )
{
    XI_UNUSED( on_connected );

    if ( driver == NULL || driver->context == NULL )
    {
        return XI_INVALID_PARAMETER;
    }

    driver->context->context_data.connection_data = xi_alloc_connection_data(
        host, port, "driver_username", "driver_password", 10, 20, XI_SESSION_CLEAN );

    xi_layer_t* input_layer = driver->context->layer_chain.top;

    xi_evtd_execute(
        driver->context->context_data.evtd_instance,
        xi_make_handle( input_layer->layer_connection.self->layer_funcs->init,
                        &input_layer->layer_connection,
                        driver->context->context_data.connection_data, XI_STATE_OK ) );

    return XI_STATE_OK;
}

#define XI_DRIVER_PROTOBUF_CREATE_DEFAULT_ON_HEAP( type, out_variable_name, stack_init,  \
                                                   state )                               \
    struct type* out_variable_name = NULL;                                               \
    {                                                                                    \
        XI_ALLOC_AT( struct type, out_variable_name, state );                            \
                                                                                         \
        struct type out_variable_name_local = stack_init;                                \
                                                                                         \
        memcpy( out_variable_name, &out_variable_name_local, sizeof( struct type ) );    \
    }


xi_state_t xi_libxively_driver_send_on_connect_finish( xi_libxively_driver_t* driver,
                                                       xi_state_t connect_result )
{
    printf( "[ driver     ] %s, state = %d\n", __func__, connect_result );

    xi_state_t state = XI_STATE_OK;

    XI_DRIVER_PROTOBUF_CREATE_DEFAULT_ON_HEAP(
        _XiClientFtestFw__XiClientCallback, client_callback,
        XI_CLIENT_FTEST_FW__XI_CLIENT_CALLBACK__INIT, state );

    XI_CHECK_CND_DBGMESSAGE( driver == NULL, XI_INTERNAL_ERROR, state,
                             "NULL driver pointer" );

    XI_DRIVER_PROTOBUF_CREATE_DEFAULT_ON_HEAP(
        _XiClientFtestFw__XiClientCallback__OnConnectFinish, on_connect_finish,
        XI_CLIENT_FTEST_FW__XI_CLIENT_CALLBACK__ON_CONNECT_FINISH__INIT, state );

    on_connect_finish->has_connect_result = 1;
    on_connect_finish->connect_result     = connect_result;

    client_callback->on_connect_finish = on_connect_finish;

    xi_layer_t* input_layer = driver->context->layer_chain.top;

    xi_evtd_execute(
        driver->context->context_data.evtd_instance,
        xi_make_handle( input_layer->layer_connection.self->layer_funcs->push,
                        &input_layer->layer_connection, client_callback, XI_STATE_OK ) );

    return XI_STATE_OK;

err_handling:

    xi_driver_free_protobuf_callback( client_callback );
    return state;
}

xi_state_t xi_libxively_driver_send_on_disconnect( xi_libxively_driver_t* driver,
                                                   xi_state_t error_code )
{
    printf( "[ driver     ] %s, state = %d\n", __func__, error_code );

    xi_state_t state = XI_STATE_OK;

    XI_DRIVER_PROTOBUF_CREATE_DEFAULT_ON_HEAP(
        _XiClientFtestFw__XiClientCallback, client_callback,
        XI_CLIENT_FTEST_FW__XI_CLIENT_CALLBACK__INIT, state );

    XI_DRIVER_PROTOBUF_CREATE_DEFAULT_ON_HEAP(
        _XiClientFtestFw__XiClientCallback__OnDisconnect, on_disconnect,
        XI_CLIENT_FTEST_FW__XI_CLIENT_CALLBACK__ON_DISCONNECT__INIT, state );

    on_disconnect->has_error_code = 1;
    on_disconnect->error_code     = error_code;

    client_callback->on_disconnect = on_disconnect;

    xi_layer_t* input_layer = driver->context->layer_chain.top;

    xi_evtd_execute(
        driver->context->context_data.evtd_instance,
        xi_make_handle( input_layer->layer_connection.self->layer_funcs->push,
                        &input_layer->layer_connection, client_callback, XI_STATE_OK ) );

    return XI_STATE_OK;

err_handling:

    xi_driver_free_protobuf_callback( client_callback );
    return state;
}

xi_state_t
xi_libxively_driver_send_on_message_received( xi_libxively_driver_t* driver,
                                              xi_sub_call_type_t call_type,
                                              const xi_sub_call_params_t* const params,
                                              xi_state_t receive_result )
{
    printf( "[ driver     ] %s, state = %d\n", __func__, receive_result );

    xi_state_t state        = XI_STATE_OK;
    xi_layer_t* input_layer = NULL;

    XI_DRIVER_PROTOBUF_CREATE_DEFAULT_ON_HEAP(
        _XiClientFtestFw__XiClientCallback, client_callback,
        XI_CLIENT_FTEST_FW__XI_CLIENT_CALLBACK__INIT, state );

    switch ( call_type )
    {
        case XI_SUB_CALL_SUBACK:
        {
            XI_DRIVER_PROTOBUF_CREATE_DEFAULT_ON_HEAP(
                _XiClientFtestFw__XiClientCallback__OnSubscribeFinish,
                on_subscribe_finish,
                XI_CLIENT_FTEST_FW__XI_CLIENT_CALLBACK__ON_SUBSCRIBE_FINISH__INIT,
                state );

            on_subscribe_finish->n_subscribe_result_list = 1;

            XI_ALLOC_BUFFER_AT( uint32_t, on_subscribe_finish->subscribe_result_list,
                                sizeof( uint32_t ), state );

            on_subscribe_finish->subscribe_result_list[0] = params->suback.suback_status;

            client_callback->on_subscribe_finish = on_subscribe_finish;
        }
        break;
        case XI_SUB_CALL_MESSAGE:
        {
            printf( "--- topic_name = %s\n", params->message.topic );
            printf( "--- qos = %d\n", params->message.qos );
            printf( "--- content length = %ld, content = %s\n",
                    params->message.temporary_payload_data_length,
                    params->message.temporary_payload_data );

            XI_DRIVER_PROTOBUF_CREATE_DEFAULT_ON_HEAP(
                _XiClientFtestFw__XiClientCallback__OnMessageReceived,
                on_message_received,
                XI_CLIENT_FTEST_FW__XI_CLIENT_CALLBACK__ON_MESSAGE_RECEIVED__INIT,
                state );

            on_message_received->topic_name  = ( char* )params->message.topic;
            on_message_received->has_qos     = 1;
            on_message_received->qos         = params->message.qos;
            on_message_received->has_payload = 1;

            /* here we have to copy the pointer data */
            XI_ALLOC_BUFFER_AT( uint8_t, on_message_received->payload.data,
                                params->message.temporary_payload_data_length, state );
            memcpy( on_message_received->payload.data,
                    params->message.temporary_payload_data,
                    params->message.temporary_payload_data_length );
            on_message_received->payload.len =
                params->message.temporary_payload_data_length;

            client_callback->on_message_received = on_message_received;
        }
        break;
        case XI_SUB_CALL_UNKNOWN:
        default:
            goto err_handling;
    }

    input_layer = driver->context->layer_chain.top;

    xi_evtd_execute(
        driver->context->context_data.evtd_instance,
        xi_make_handle( input_layer->layer_connection.self->layer_funcs->push,
                        &input_layer->layer_connection, client_callback, XI_STATE_OK ) );

    return XI_STATE_OK;

err_handling:

    xi_driver_free_protobuf_callback( client_callback );
    return state;
}

xi_state_t xi_libxively_driver_send_on_publish_finish( xi_libxively_driver_t* driver,
                                                       void* data,
                                                       xi_state_t publish_result )
{
    XI_UNUSED( data );

    printf( "[ driver     ] %s, state = %d\n", __func__, publish_result );

    xi_state_t state = XI_STATE_OK;

    XI_DRIVER_PROTOBUF_CREATE_DEFAULT_ON_HEAP(
        _XiClientFtestFw__XiClientCallback, client_callback,
        XI_CLIENT_FTEST_FW__XI_CLIENT_CALLBACK__INIT, state );

    XI_DRIVER_PROTOBUF_CREATE_DEFAULT_ON_HEAP(
        _XiClientFtestFw__XiClientCallback__OnPublishFinish, on_publish_finish,
        XI_CLIENT_FTEST_FW__XI_CLIENT_CALLBACK__ON_PUBLISH_FINISH__INIT, state );

    on_publish_finish->has_return_code = 1;
    on_publish_finish->return_code     = publish_result;

    client_callback->on_publish_finish = on_publish_finish;

    xi_layer_t* input_layer = driver->context->layer_chain.top;

    xi_evtd_execute(
        driver->context->context_data.evtd_instance,
        xi_make_handle( input_layer->layer_connection.self->layer_funcs->push,
                        &input_layer->layer_connection, client_callback, XI_STATE_OK ) );

    return XI_STATE_OK;

err_handling:

    xi_driver_free_protobuf_callback( client_callback );
    return state;
}
