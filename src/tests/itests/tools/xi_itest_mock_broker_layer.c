// Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.

#include "xi_globals.h"
#include "xi_itest_helpers.h"
#include "xi_itest_layerchain_ct_ml_mc.h"
#include "xi_itest_mock_broker_layer.h"
#include "xi_itest_mock_broker_layerchain.h"
#include "xi_layer_macros.h"
#include "xi_mqtt_logic_layer_data_helpers.h"
#include "xi_tuples.h"
#include "xi_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern xi_context_t* xi_context;            // Xivley Client context
extern xi_context_t* xi_context_mockbroker; // test mock broker context

/************************************************************************************
 * mock broker secondary layer
*******************************************************
************************************************************************************/
xi_state_t
xi_mock_broker_secondary_layer_push( void* context, void* data, xi_state_t in_out_state )
{
    XI_UNUSED( itest_mock_broker_codec_layer_chain );
    XI_UNUSED( XI_LAYER_CHAIN_MOCK_BROKER_CODECSIZE_SUFFIX );
    XI_UNUSED( itest_ct_ml_mc_layer_chain );
    XI_UNUSED( XI_LAYER_CHAIN_CT_ML_MCSIZE_SUFFIX );
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    check_expected( in_out_state );

    const xi_mock_broker_control_t control = mock_type( xi_mock_broker_control_t );

    if ( control == CONTROL_ERROR )
    {
        xi_data_desc_t* buffer = ( xi_data_desc_t* )data;
        xi_free_desc( &buffer );

        in_out_state = mock_type( xi_state_t );

        xi_time_event_handle_t time_event_handle = xi_make_empty_time_event_handle();

        xi_evtd_execute_in(
            xi_globals.evtd_instance,
            xi_make_handle(
                xi_itest_find_layer( xi_context, XI_LAYER_TYPE_MQTT_CODEC_SUT )
                    ->layer_funcs->pull,
                &xi_itest_find_layer( xi_context, XI_LAYER_TYPE_MQTT_CODEC_SUT )
                     ->layer_connection,
                NULL, in_out_state ),
            1, &time_event_handle );

        return XI_PROCESS_PUSH_ON_NEXT_LAYER( context, NULL, XI_STATE_WRITTEN );
    }

    if ( in_out_state == XI_STATE_OK )
    {
        xi_time_event_handle_t time_event_handle = xi_make_empty_time_event_handle();

        // jump to libxively's codec layer pull function, mimicing incoming encoded
        // message
        xi_evtd_execute_in(
            xi_globals.evtd_instance,
            xi_make_handle(
                xi_itest_find_layer( xi_context, XI_LAYER_TYPE_MQTT_CODEC_SUT )
                    ->layer_funcs->pull,
                &xi_itest_find_layer( xi_context, XI_LAYER_TYPE_MQTT_CODEC_SUT )
                     ->layer_connection,
                data, in_out_state ),
            1, &time_event_handle );

        return XI_PROCESS_PUSH_ON_NEXT_LAYER( context, NULL, XI_STATE_WRITTEN );
    }

    return XI_PROCESS_PUSH_ON_NEXT_LAYER( context, NULL, in_out_state );
}

xi_state_t
xi_mock_broker_secondary_layer_pull( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_out_state );

    /* release the payload */
    xi_data_desc_t* desc = ( xi_data_desc_t* )data;
    xi_free_desc( &desc );

    return XI_STATE_OK;
}

xi_state_t
xi_mock_broker_secondary_layer_close( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_out_state );

    return XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER( context, data, in_out_state );
}

xi_state_t xi_mock_broker_secondary_layer_close_externally( void* context,
                                                            void* data,
                                                            xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_out_state );

    return XI_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER( context, data, in_out_state );
}

xi_state_t
xi_mock_broker_secondary_layer_init( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_out_state );

    return XI_STATE_OK;
}

xi_state_t xi_mock_broker_secondary_layer_connect( void* context,
                                                   void* data,
                                                   xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_out_state );

    return XI_STATE_OK;
}

/************************************************************************************
 * mock broker primary layer
*********************************************************
************************************************************************************/
xi_state_t xi_mock_broker_layer_push__ERROR_CHANNEL()
{
    return mock_type( xi_state_t );
}

xi_state_t xi_mock_broker_layer_push( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    check_expected( in_out_state );

    const xi_mock_broker_control_t control = mock_type( xi_mock_broker_control_t );

    if ( control != CONTROL_CONTINUE )
    {
        xi_data_desc_t* buffer = ( xi_data_desc_t* )data;
        xi_free_desc( &buffer );

        in_out_state = xi_mock_broker_layer_push__ERROR_CHANNEL();

        return XI_PROCESS_PUSH_ON_NEXT_LAYER( context, 0, in_out_state );
    }

    if ( in_out_state == XI_STATE_OK )
    {
        // switching context + layer chain to decode the mqtt encoded message
        // current is the libxivel's layer chain
        // target is the mock broker's layer chain

        /* copy the received data since it will be used in two places */
        xi_data_desc_t* orig = ( xi_data_desc_t* )data;
        xi_data_desc_t* copy =
            xi_make_desc_from_buffer_copy( orig->data_ptr, orig->length );

        xi_time_event_handle_t time_event_handle = xi_make_empty_time_event_handle();

        xi_evtd_execute_in(
            xi_globals.evtd_instance,
            xi_make_handle( xi_itest_find_layer( xi_context_mockbroker,
                                                 XI_LAYER_TYPE_MOCKBROKER_MQTT_CODEC )
                                ->layer_funcs->pull,
                            &xi_itest_find_layer( xi_context_mockbroker,
                                                  XI_LAYER_TYPE_MOCKBROKER_MQTT_CODEC )
                                 ->layer_connection,
                            copy, XI_STATE_OK ),
            1, &time_event_handle );

        // "default" libxively behavior
        return XI_PROCESS_PUSH_ON_PREV_LAYER( context, data, in_out_state );
    }
    else
    {
        if ( XI_NEXT_LAYER( context ) != 0 )
        {
            return XI_PROCESS_PUSH_ON_NEXT_LAYER( context, data, in_out_state );
        }
        else if ( XI_STATE_WRITTEN == in_out_state ||
                  XI_STATE_FAILED_WRITING == in_out_state )
        {
            xi_mqtt_written_data_t* written_data = ( xi_mqtt_written_data_t* )data;
            XI_SAFE_FREE_TUPLE( written_data );
        }

        return XI_STATE_OK;
    }
}

xi_state_t xi_mock_broker_layer_pull( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    // incoming decoded message from the libxively layers

    enum xi_connack_return_code
    {
        XI_CONNACK_ACCEPTED                      = 0,
        XI_CONNACK_REFUSED_PROTOCOL_VERSION      = 1,
        XI_CONNACK_REFUSED_IDENTIFIER_REJECTED   = 2,
        XI_CONNACK_REFUSED_SERVER_UNAVAILABLE    = 3,
        XI_CONNACK_REFUSED_BAD_USERNAME_PASSWORD = 4,
        XI_CONNACK_REFUSED_NOT_AUTHORIZED        = 5
    };

    check_expected( in_out_state );

    xi_mqtt_message_t* recvd_msg = ( xi_mqtt_message_t* )data;

    // mock broker behavior
    if ( in_out_state == XI_STATE_OK )
    {
        assert( NULL != recvd_msg ); // sanity check

        // const uint16_t msg_id = xi_mqtt_get_message_id( recvd_msg );
        const xi_mqtt_type_t recvd_msg_type = recvd_msg->common.common_u.common_bits.type;

        xi_debug_format( "mock broker received message with type %d ", recvd_msg_type );

        check_expected( recvd_msg_type );

        switch ( recvd_msg_type )
        {
            case XI_MQTT_TYPE_CONNECT:
            {
                XI_ALLOC( xi_mqtt_message_t, msg_connack, in_out_state );
                XI_CHECK_STATE( in_out_state = fill_with_connack_data(
                                    msg_connack, XI_CONNACK_ACCEPTED ) );

                xi_mqtt_message_free( &recvd_msg );
                return XI_PROCESS_PUSH_ON_PREV_LAYER( context, msg_connack,
                                                      in_out_state );
            }
            break;

            case XI_MQTT_TYPE_SUBSCRIBE:
            {
                const char* subscribe_topic_name =
                    ( const char* )recvd_msg->subscribe.topics->name->data_ptr;

                xi_debug_format( "subscribe arrived on topic `%s`",
                                 subscribe_topic_name );

                check_expected( subscribe_topic_name );

                XI_ALLOC( xi_mqtt_message_t, msg_suback, in_out_state );
                // note: fill subscribe can be used for fill subsck only because the
                // memory map of the two structs are identical since unions are used
                XI_CHECK_STATE(
                    in_out_state = fill_with_subscribe_data(
                        msg_suback,
                        "unused topic name", // topic name is not used in SUBACK, so this
                                             // string is irrelevant
                        recvd_msg->subscribe.message_id,
                        recvd_msg->subscribe.topics->xi_mqtt_topic_pair_payload_u.qos,
                        XI_MQTT_DUP_FALSE ) );

                msg_suback->common.common_u.common_bits.type = XI_MQTT_TYPE_SUBACK;

                xi_mqtt_message_free( &recvd_msg );
                return XI_PROCESS_PUSH_ON_PREV_LAYER( context, msg_suback, in_out_state );
            }
            break;

            case XI_MQTT_TYPE_PUBLISH:
            {
                const char* publish_topic_name =
                    ( const char* )recvd_msg->publish.topic_name->data_ptr;

                xi_debug_format( "publish arrived on topic `%s`, msgid: %d",
                                 publish_topic_name, recvd_msg->publish.message_id );

                check_expected( publish_topic_name );

                XI_ALLOC( xi_mqtt_message_t, msg_puback, in_out_state );
                XI_CHECK_STATE( in_out_state = fill_with_puback_data(
                                    msg_puback, recvd_msg->publish.message_id ) );

                xi_mqtt_message_free( &recvd_msg );
                return XI_PROCESS_PUSH_ON_PREV_LAYER( context, msg_puback, in_out_state );
            }
            break;

            case XI_MQTT_TYPE_DISCONNECT:
                // do nothing
                xi_mqtt_message_free( &recvd_msg );
                return XI_STATE_OK;
                break;

            default:
                xi_mqtt_message_free( &recvd_msg );
                xi_debug_printf( "*** *** unhandled message arrived\r\n" );
                break;
        }
    }

    xi_mqtt_message_free( &recvd_msg );

err_handling:
    return XI_MQTT_UNKNOWN_MESSAGE_ID;
}

xi_state_t
xi_mock_broker_layer_close( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    check_expected( in_out_state );

    return XI_PROCESS_CLOSE_ON_PREV_LAYER( context, data, in_out_state );
}

xi_state_t xi_mock_broker_layer_close_externally( void* context,
                                                  void* data,
                                                  xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( context );
    XI_UNUSED( data );

    return XI_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER( context, data, in_out_state );
}

xi_state_t xi_mock_broker_layer_init( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    check_expected( in_out_state );

    const xi_mock_broker_control_t control = mock_type( xi_mock_broker_control_t );

    if ( control == CONTROL_ERROR )
    {
        const xi_state_t return_state = mock_type( xi_state_t );
        return XI_PROCESS_CONNECT_ON_THIS_LAYER( context, data, return_state );
    }

    return XI_PROCESS_INIT_ON_PREV_LAYER( context, data, in_out_state );
}

xi_state_t
xi_mock_broker_layer_connect( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    check_expected( in_out_state );

    return XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, data, in_out_state );
}

#ifdef __cplusplus
}
#endif
