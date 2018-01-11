// Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
//
// This is part of the Xively C Client library,
// it is licensed under the BSD 3-Clause license.

#include "xi_driver_codec_protobuf_layer.h"
#include "xi_layer_macros.h"
#include "xi_debug.h"
#include "xi_layer_api.h"
#include "xi_control_channel_protocol.pb-c.h"
#include "xi_driver_codec_protobuf_layer_data.h"
#include "xi_macros.h"
#include "xi_data_desc.h"

void xi_driver_free_protobuf_callback(
    struct _XiClientFtestFw__XiClientCallback* callback )
{
    XI_SAFE_FREE( callback->on_connect_finish );
    XI_SAFE_FREE( callback->on_disconnect );

    if ( NULL != callback->on_subscribe_finish )
    {
        XI_SAFE_FREE( callback->on_subscribe_finish->subscribe_result_list );
    }

    XI_SAFE_FREE( callback->on_subscribe_finish );

    if ( NULL != callback->on_message_received )
    {
        if ( 1 == callback->on_message_received->has_payload )
        {
            XI_SAFE_FREE( callback->on_message_received->payload.data );
        }
    }

    XI_SAFE_FREE( callback->on_message_received );
    XI_SAFE_FREE( callback->on_publish_finish );

    XI_SAFE_FREE( callback );
}

xi_state_t
xi_driver_codec_protobuf_layer_push( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    struct _XiClientFtestFw__XiClientCallback* client_callback = NULL;

    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) )
    {
        goto err_handling;
    }

    if ( NULL == data )
    {
        return XI_PROCESS_PUSH_ON_NEXT_LAYER( context, data, in_out_state );
    }
    else
    {
        client_callback = ( struct _XiClientFtestFw__XiClientCallback* )data;

        const size_t packed_size =
            xi_client_ftest_fw__xi_client_callback__get_packed_size( client_callback );

        xi_data_desc_t* data_desc = xi_make_empty_desc_alloc( packed_size );
        XI_CHECK_MEMORY( data_desc, in_out_state );

        xi_client_ftest_fw__xi_client_callback__pack( client_callback,
                                                      data_desc->data_ptr );
        data_desc->length = packed_size;

        xi_debug_printf( "[ driver cdc ] packed_size = %lu, data_desc->length = %d\n",
                         packed_size, data_desc->length );

        xi_driver_free_protobuf_callback( client_callback );
        return XI_PROCESS_PUSH_ON_PREV_LAYER( context, data_desc, XI_STATE_OK );
    }

err_handling:
    if ( NULL != client_callback )
    {
        xi_driver_free_protobuf_callback( client_callback );
    }

    return XI_PROCESS_CLOSE_ON_THIS_LAYER( context, data, in_out_state );
}

xi_state_t
xi_driver_codec_protobuf_layer_pull( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    xi_driver_codec_protobuf_layer_data_t* layer_data = NULL;

    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) )
    {
        goto err_handling;
    }

    xi_data_desc_t* encoded_protobuf_chunk = ( xi_data_desc_t* )data;

    xi_debug_printf( "[ driver cdc ] data length = %d\n",
                     encoded_protobuf_chunk->length );

    layer_data =
        ( xi_driver_codec_protobuf_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    XI_CHECK_CND_DBGMESSAGE(
        NULL == layer_data, XI_NOT_INITIALIZED, in_out_state,
        "driver's codec layer has no layer data allocated. Uninitialized." );

    { // accumulate chunks
        if ( layer_data->encoded_protobuf_accumulated == NULL )
        { // first chunk
            layer_data->encoded_protobuf_accumulated = encoded_protobuf_chunk;

            encoded_protobuf_chunk = NULL;
        }
        else
        { // trailing chunks -> accumulate them
            xi_state_t append_bytes_result = xi_data_desc_append_data_resize(
                layer_data->encoded_protobuf_accumulated,
                ( const char* )encoded_protobuf_chunk->data_ptr,
                encoded_protobuf_chunk->length );

            XI_CHECK_CND_DBGMESSAGE( XI_STATE_OK != append_bytes_result,
                                     append_bytes_result, in_out_state,
                                     "could not accumulate incoming probobuf chunks" );

            xi_free_desc( &encoded_protobuf_chunk );
        }
    }

    xi_debug_printf( "[ driver cdc ] accumulated protobuf length = %d\n",
                     layer_data->encoded_protobuf_accumulated->length );

    // try to unpack
    struct _XiClientFtestFw__XiClientAPI* message_API_call =
        xi_client_ftest_fw__xi_client_api__unpack(
            NULL, layer_data->encoded_protobuf_accumulated->length,
            layer_data->encoded_protobuf_accumulated->data_ptr );

    // protobuf does not support unpacking data in chunks:
    // https://code.google.com/p/protobuf-c/issues/detail?id=67
    // so concatenate data until it can be unpacked successfully

    xi_debug_printf( "[ driver cdc ] unpacked message ptr = %p\n", message_API_call );

    if ( NULL != message_API_call )
    { // successful unpack
        xi_free_desc( &layer_data->encoded_protobuf_accumulated );

        return XI_PROCESS_PULL_ON_NEXT_LAYER( context, message_API_call, XI_STATE_OK );
    }
    else
    {
        return XI_PROCESS_PULL_ON_PREV_LAYER( context, NULL, XI_STATE_WANT_READ );
    }

err_handling:
    if ( NULL != layer_data )
    {
        xi_free_desc( &layer_data->encoded_protobuf_accumulated );
    }
    // close layer chain in case of error: not doing recovery
    return XI_PROCESS_CLOSE_ON_THIS_LAYER( context, NULL, XI_SERIALIZATION_ERROR );
}

xi_state_t
xi_driver_codec_protobuf_layer_close( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    return XI_PROCESS_CLOSE_ON_PREV_LAYER( context, data, in_out_state );
}

xi_state_t xi_driver_codec_protobuf_layer_close_externally( void* context,
                                                            void* data,
                                                            xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    xi_driver_codec_protobuf_layer_data_t* layer_data =
        ( xi_driver_codec_protobuf_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    if ( NULL != layer_data )
    {
        xi_free_desc( &layer_data->encoded_protobuf_accumulated );

        XI_SAFE_FREE( XI_THIS_LAYER( context )->user_data );
    }

    return XI_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER( context, data, in_out_state );
}

xi_state_t
xi_driver_codec_protobuf_layer_init( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    assert( XI_THIS_LAYER( context )->user_data == 0 );

    XI_ALLOC_AT( xi_driver_codec_protobuf_layer_data_t,
                 XI_THIS_LAYER( context )->user_data, in_out_state );

    return XI_PROCESS_INIT_ON_PREV_LAYER( context, data, in_out_state );

err_handling:
    XI_SAFE_FREE( XI_THIS_LAYER( context )->user_data );

    return XI_PROCESS_INIT_ON_NEXT_LAYER( context, data, in_out_state );
}

xi_state_t xi_driver_codec_protobuf_layer_connect( void* context,
                                                   void* data,
                                                   xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    return XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, data, in_out_state );
}
