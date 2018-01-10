/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_globals.h"
#include "xi_handle.h"
#include "xi_itest_helpers.h"
#include "xi_itest_layerchain_mqttlogic.h"
#include "xi_itest_mqttlogic_layer.h"
#include "xi_memory_checks.h"
#include "xi_mqtt_logic_layer_data_helpers.h"

#include <time.h>

xi_context_t* xi_context__itest_mqttlogic_layer                      = NULL;
const xi_state_t xi_itest_mqttlogic__backoff_terminal_class_errors[] = {
    XI_CONNECTION_RESET_BY_PEER_ERROR, XI_MQTT_UNACCEPTABLE_PROTOCOL_VERSION,
    XI_MQTT_IDENTIFIER_REJECTED, XI_MQTT_BAD_USERNAME_OR_PASSWORD,
    XI_MQTT_NOT_AUTHORIZED};

typedef enum {
    XI_MQTT_QOS_0 = 0,
    XI_MQTT_QOS_1 = 1,
    XI_MQTT_QOS_TEST_LEVELS_COUNT
} xi_itest_mqttlogic_qos_levels_t;

typedef struct xi_itest_mqttlogic_test_msg_what_to_check_s
{
    unsigned char retain : 1;
    unsigned char qos : 1;
    unsigned char dup : 1;
    unsigned char type : 1;
} xi_itest_mqttlogic_test_msg_what_to_check_t;

typedef struct xi_itest_mqttlogic_test_msg_common_bits_check_values_s
{
    unsigned int retain : 1;
    unsigned int qos : 2;
    unsigned int dup : 1;
    unsigned int type : 4;
} xi_itest_mqttlogic_test_msg_common_bits_check_values_t;

typedef struct xi_itest_mqttlogic_test_msg_s
{
    xi_itest_mqttlogic_test_msg_what_to_check_t what_to_check;
    xi_itest_mqttlogic_test_msg_common_bits_check_values_t values_to_check;
} xi_itest_mqttlogic_test_msg_t;

xi_itest_mqttlogic_test_msg_t* xi_itest_mqttlogic_make_msg_test_matrix(
    xi_itest_mqttlogic_test_msg_what_to_check_t what_to_check,
    xi_itest_mqttlogic_test_msg_common_bits_check_values_t values_to_check )
{
    xi_itest_mqttlogic_test_msg_t* test_values = ( xi_itest_mqttlogic_test_msg_t* )malloc(
        sizeof( xi_itest_mqttlogic_test_msg_t ) );

    test_values->what_to_check   = what_to_check;
    test_values->values_to_check = values_to_check;

    return test_values;
}

#define xi_itest_mqttlogic_test( what, msg, test_msg_data, ret )                         \
    if ( test_msg_data->what_to_check.what )                                             \
    {                                                                                    \
        if ( msg->common.common_u.common_bits.what !=                                    \
             test_msg_data->values_to_check.what )                                       \
        {                                                                                \
            fprintf( stderr, "msg bit - [%d] " #what                                     \
                             " does not match the expected value [%d]\n",                \
                     msg->common.common_u.common_bits.what,                              \
                     test_msg_data->values_to_check.what );                              \
            fflush( stderr );                                                            \
            ret = 0;                                                                     \
        }                                                                                \
    }

int check_msg( const LargestIntegralType data,
               const LargestIntegralType check_value_data )
{
    xi_itest_mqttlogic_test_msg_t* test_msg_data =
        ( xi_itest_mqttlogic_test_msg_t* )check_value_data;

    xi_mqtt_message_t* msg = ( xi_mqtt_message_t* )data;

    int ret = 1;

    assert_non_null( msg );
    assert_non_null( test_msg_data );

    xi_itest_mqttlogic_test( retain, msg, test_msg_data, ret );
    xi_itest_mqttlogic_test( qos, msg, test_msg_data, ret );
    xi_itest_mqttlogic_test( dup, msg, test_msg_data, ret );
    xi_itest_mqttlogic_test( type, msg, test_msg_data, ret );

    free( test_msg_data );
    return ret;
}

typedef struct xi_itest_mqttlogic_persistant_session_test_sample_s
{
    xi_mqtt_type_t msg_type;
    xi_state_t ( *api_call )( xi_context_handle_t xih,
                              const char* topic_name,
                              const char* payload );
} xi_itest_mqttlogic_persistant_session_test_sample_t;

xi_state_t xi_itest_mqttlogic_call_publish( xi_context_handle_t xih,
                                            const char* topic_name,
                                            const char* payload )
{
    return xi_publish( xih, topic_name, payload, XI_MQTT_QOS_AT_LEAST_ONCE,
                       XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

void xi_itest_mqttlogic_subscribe_callback( xi_context_handle_t in_context_handle,
                                            xi_sub_call_type_t call_type,
                                            const xi_sub_call_params_t* const params,
                                            xi_state_t state,
                                            void* user_data )
{
    XI_UNUSED( in_context_handle );
    XI_UNUSED( call_type );
    XI_UNUSED( params );
    XI_UNUSED( state );
    XI_UNUSED( user_data );
}

xi_state_t xi_itest_mqttlogic_call_subscribe( xi_context_handle_t xih,
                                              const char* topic_name,
                                              const char* payload )
{
    XI_UNUSED( payload );
    return xi_subscribe( xih, topic_name, XI_MQTT_QOS_AT_LEAST_ONCE,
                         &xi_itest_mqttlogic_subscribe_callback, NULL );
}

static const xi_itest_mqttlogic_persistant_session_test_sample_t
    XI_ITEST_MQTTLOGIC_PERSISTANT_SESSION_TEST_DATA[] = {
        {XI_MQTT_TYPE_PUBLISH, &xi_itest_mqttlogic_call_publish},
        {XI_MQTT_TYPE_SUBSCRIBE, &xi_itest_mqttlogic_call_subscribe}};

int xi_itest_mqttlogic_layer_setup( void** state )
{
    XI_UNUSED( state );

    xi_memory_limiter_tearup();

    assert_int_equal( XI_STATE_OK, xi_initialize( "xi_itest_tls_error_account_id",
                                                  "xi_itest_tls_error_device_id" ) );

    XI_CHECK_STATE( xi_create_context_with_custom_layers(
        &xi_context__itest_mqttlogic_layer, xi_itest_layer_chain_mqttlogic,
        XI_LAYER_CHAIN_MQTTLOGIC,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_MQTTLOGIC ) ) );

    return 0;

err_handling:
    fail();

    return 1;
}

int xi_itest_mqttlogic_layer_teardown( void** state )
{
    XI_UNUSED( state );

    xi_delete_context_with_custom_layers(
        &xi_context__itest_mqttlogic_layer, xi_itest_layer_chain_mqttlogic,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_MQTTLOGIC ) );

    xi_shutdown();

    return !xi_memory_limiter_teardown();
}

void xi_itest_mqttlogic_init_layer( xi_layer_t* top_layer )
{
    // default init process expectations
    expect_value( xi_mock_layer_mqttlogic_prev_init, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_layer_mqttlogic_prev_connect, in_out_state, XI_STATE_OK );
    // mqtt logic layer pushes the CONNECT message immediately
    expect_value( xi_mock_layer_mqttlogic_prev_push, in_out_state, XI_STATE_OK );

    expect_check(
        xi_mock_layer_mqttlogic_prev_push, data, check_msg,
        xi_itest_mqttlogic_make_msg_test_matrix(
            ( xi_itest_mqttlogic_test_msg_what_to_check_t ){
                .retain = 0, .qos = 0, .dup = 0, .type = 1},
            ( xi_itest_mqttlogic_test_msg_common_bits_check_values_t ){
                .retain = 0, .qos = 0, .dup = 0, .type = XI_MQTT_TYPE_CONNECT} ) );

    xi_context__itest_mqttlogic_layer->context_data.connection_data =
        xi_alloc_connection_data(
            "target.broker.com", 8883, "mqttlogic_layer_itest_username",
            "mqttlogic_layer_itest_password", 0, 0, XI_SESSION_CLEAN );

    XI_PROCESS_INIT_ON_PREV_LAYER( &top_layer->layer_connection, NULL, XI_STATE_OK );

    xi_evtd_step( xi_globals.evtd_instance, time( NULL ) );
}

void xi_itest_mqttlogic_prepare_init_and_connect_layer( xi_layer_t* top_layer,
                                                        xi_session_type_t session_type,
                                                        uint16_t keepalive_timeout )
{
    xi_context__itest_mqttlogic_layer->context_data.shutdown_state =
        XI_SHUTDOWN_UNINITIALISED;

    // default init process expectations
    expect_value( xi_mock_layer_mqttlogic_prev_init, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_layer_mqttlogic_prev_connect, in_out_state, XI_STATE_OK );

    // mqtt logic layer pushes the CONNECT message immediately
    expect_value( xi_mock_layer_mqttlogic_prev_push, in_out_state, XI_STATE_OK );

    expect_check(
        xi_mock_layer_mqttlogic_prev_push, data, check_msg,
        xi_itest_mqttlogic_make_msg_test_matrix(
            ( xi_itest_mqttlogic_test_msg_what_to_check_t ){
                .retain = 0, .qos = 0, .dup = 0, .type = 1},
            ( xi_itest_mqttlogic_test_msg_common_bits_check_values_t ){
                .retain = 0, .qos = 0, .dup = 0, .type = XI_MQTT_TYPE_CONNECT} ) );

    xi_context__itest_mqttlogic_layer->context_data.connection_data =
        xi_alloc_connection_data(
            "target.broker.com", 8883, "mqttlogic_layer_itest_username",
            "mqttlogic_layer_itest_password", keepalive_timeout, 0, session_type );

    XI_PROCESS_INIT_ON_PREV_LAYER( &top_layer->layer_connection, NULL, XI_STATE_OK );

    xi_evtd_step( xi_globals.evtd_instance, time( NULL ) );

    /* let's give it back the CONNACK */
    xi_state_t state = XI_STATE_OK;
    XI_ALLOC( xi_mqtt_message_t, connack, state );
    XI_CHECK_STATE( fill_with_connack_data( connack, 0 ) );
    XI_PROCESS_PULL_ON_PREV_LAYER( &top_layer->layer_connection, connack, XI_STATE_OK );

    expect_value( xi_mock_layer_mqttlogic_next_connect, in_out_state, XI_STATE_OK );

err_handling:
    return;
}

void xi_itest_mqttlogic_shutdown_and_disconnect( xi_context_handle_t context )
{
    /* now let's disconnect the layer */
    xi_shutdown_connection( context );

    /* it should begin the shutdown sequence */
    expect_value( xi_mock_layer_mqttlogic_next_push, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_layer_mqttlogic_prev_push, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_layer_mqttlogic_prev_close, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_layer_mqttlogic_prev_close_externally, in_out_state,
                  XI_STATE_OK );
    expect_value( xi_mock_layer_mqttlogic_next_close_externally, in_out_state,
                  XI_STATE_OK );

    expect_check( xi_mock_layer_mqttlogic_prev_push, data, check_msg,
                  xi_itest_mqttlogic_make_msg_test_matrix(
                      ( xi_itest_mqttlogic_test_msg_what_to_check_t ){0, 0, 0, 1},
                      ( xi_itest_mqttlogic_test_msg_common_bits_check_values_t ){
                          0, 0, 0, XI_MQTT_TYPE_DISCONNECT} ) );

    /* let's process shutdown */
    xi_evtd_step( xi_globals.evtd_instance, time( NULL ) );

    xi_free_connection_data(
        &xi_context__itest_mqttlogic_layer->context_data.connection_data );
}

void xi_itest_mqttlogic_layer_act()
{
    size_t loop_counter = 0;
    while ( xi_evtd_dispatcher_continue( xi_globals.evtd_instance ) == 1 &&
            loop_counter < 5 )
    {
        xi_evtd_step( xi_globals.evtd_instance, time( NULL ) + loop_counter );
        ++loop_counter;
    }
}


/*********************************************************************************
 * test cases ********************************************************************
 ********************************************************************************/
void xi_itest_mqttlogic_layer__backoff_class_error_PUSH__layerchain_closure_is_expected(
    void** state )
{
    uint8_t id_error = 0;
    for ( ; id_error < XI_ARRAYSIZE( xi_itest_mqttlogic__backoff_terminal_class_errors );
          ++id_error )
    {
        const xi_state_t error_under_test =
            xi_itest_mqttlogic__backoff_terminal_class_errors[id_error];

        /* __xi_printf("--- --- --- %s, error = %d\n", __func__, error_under_test ); */

        xi_layer_t* top_layer = xi_context__itest_mqttlogic_layer->layer_chain.top;

        xi_itest_mqttlogic_init_layer( top_layer );

        // expected reaction on failure
        expect_value( xi_mock_layer_mqttlogic_prev_close, in_out_state,
                      XI_BACKOFF_TERMINAL );
        expect_value( xi_mock_layer_mqttlogic_prev_close_externally, in_out_state,
                      XI_BACKOFF_TERMINAL );
        // libxively believes error occurred during connection process thus
        // it returns connect layer function with proper error
        expect_value( xi_mock_layer_mqttlogic_next_connect, in_out_state,
                      XI_BACKOFF_TERMINAL );

        expect_value( xi_mock_layer_mqttlogic_prev_close, in_out_state,
                      XI_BACKOFF_TERMINAL );
        expect_value( xi_mock_layer_mqttlogic_prev_close_externally, in_out_state,
                      XI_BACKOFF_TERMINAL );
        expect_value( xi_mock_layer_mqttlogic_next_close_externally, in_out_state,
                      XI_BACKOFF_TERMINAL );

        // FAILURE HERE: error occurs during PUSH
        XI_PROCESS_PUSH_ON_PREV_LAYER( &top_layer->layer_connection, NULL,
                                       error_under_test );

        xi_itest_mqttlogic_layer_act();

        xi_itest_mqttlogic_layer_teardown( state );
        xi_itest_mqttlogic_layer_setup( state );
    }
}

void xi_itest_mqttlogic_layer__backoff_class_error_PULL__layerchain_closure_is_expected(
    void** state )
{
    uint8_t id_error = 0;
    for ( ; id_error < XI_ARRAYSIZE( xi_itest_mqttlogic__backoff_terminal_class_errors );
          ++id_error )
    {
        const xi_state_t error_under_test =
            xi_itest_mqttlogic__backoff_terminal_class_errors[id_error];

        /* __xi_printf("--- --- --- %s, error = %d\n", __func__, error_under_test ); */

        xi_layer_t* top_layer = xi_context__itest_mqttlogic_layer->layer_chain.top;

        xi_itest_mqttlogic_init_layer( top_layer );

        // expected reaction on failure
        expect_value( xi_mock_layer_mqttlogic_prev_close, in_out_state,
                      XI_BACKOFF_TERMINAL );
        expect_value( xi_mock_layer_mqttlogic_prev_close_externally, in_out_state,
                      XI_BACKOFF_TERMINAL );
        // libxively believes error occurred during connection process thus
        // it returns connect layer function with proper error
        expect_value( xi_mock_layer_mqttlogic_next_connect, in_out_state,
                      XI_BACKOFF_TERMINAL );

        expect_value( xi_mock_layer_mqttlogic_prev_close, in_out_state,
                      XI_BACKOFF_TERMINAL );
        expect_value( xi_mock_layer_mqttlogic_prev_close_externally, in_out_state,
                      XI_BACKOFF_TERMINAL );
        expect_value( xi_mock_layer_mqttlogic_next_close_externally, in_out_state,
                      XI_BACKOFF_TERMINAL );

        // FAILURE HERE: error occurs during PULL
        XI_PROCESS_PULL_ON_PREV_LAYER( &top_layer->layer_connection, NULL,
                                       error_under_test );

        xi_itest_mqttlogic_layer_act();

        xi_itest_mqttlogic_layer_teardown( state );
        xi_itest_mqttlogic_layer_setup( state );
    }
}

void xi_itest_mqtt_logic_layer_sub_callback( xi_context_handle_t in_context_handle,
                                             xi_sub_call_type_t call_type,
                                             const xi_sub_call_params_t* const params,
                                             xi_state_t state,
                                             void* user_data )
{
    XI_UNUSED( in_context_handle );
    XI_UNUSED( call_type );
    XI_UNUSED( params );
    XI_UNUSED( state );
    XI_UNUSED( user_data );

    check_expected( call_type );
    check_expected( state );
}

void xi_itest_mqtt_logic_layer__subscribe_success__success_suback_callback_invocation(
    void** state )
{
    XI_UNUSED( state );

    xi_state_t local_state             = XI_STATE_OK;
    xi_mqtt_message_t* suback          = NULL;
    xi_context_handle_t context_handle = XI_INVALID_CONTEXT_HANDLE;

    int it_qos_level = 0;
    for ( ; it_qos_level < XI_MQTT_QOS_TEST_LEVELS_COUNT; ++it_qos_level )
    {
        xi_debug_format( "Testing QoS level: %d", it_qos_level );

        /* initialisation of the layer chain */
        xi_layer_t* top_layer = xi_context__itest_mqttlogic_layer->layer_chain.top;
        xi_itest_mqttlogic_prepare_init_and_connect_layer( top_layer, XI_SESSION_CLEAN,
                                                           0 );
        xi_itest_mqttlogic_layer_act();

        /* let's extract the context handle in order to use it against the API functions
         */
        XI_CHECK_STATE( local_state = xi_find_handle_for_object(
                            xi_globals.context_handles_vector,
                            xi_context__itest_mqttlogic_layer, &context_handle ) );

        /* let's use the xively C library subscribe function */
        xi_subscribe( context_handle, "test_topic", it_qos_level,
                      &xi_itest_mqtt_logic_layer_sub_callback, NULL );

        expect_value( xi_mock_layer_mqttlogic_next_push, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_layer_mqttlogic_prev_push, in_out_state, XI_STATE_OK );

        expect_check(
            xi_mock_layer_mqttlogic_prev_push, data, check_msg,
            xi_itest_mqttlogic_make_msg_test_matrix(
                ( xi_itest_mqttlogic_test_msg_what_to_check_t ){
                    .retain = 0, .qos = 0, .dup = 0, .type = 1},
                ( xi_itest_mqttlogic_test_msg_common_bits_check_values_t ){
                    .retain = 0, .qos = 0, .dup = 0, .type = XI_MQTT_TYPE_SUBSCRIBE} ) );

        /* let's start the subscription process */
        xi_itest_mqttlogic_layer_act();

        /* let's send suback message */
        XI_ALLOC_AT( xi_mqtt_message_t, suback, local_state );
        suback->common.common_u.common_bits.type = XI_MQTT_TYPE_SUBACK;
        suback->suback.message_id                = 1;
        XI_ALLOC_AT( xi_mqtt_topicpair_t, suback->suback.topics, local_state );
        suback->suback.topics->xi_mqtt_topic_pair_payload_u.qos    = 0;
        suback->suback.topics->xi_mqtt_topic_pair_payload_u.status = it_qos_level;
        XI_PROCESS_PULL_ON_PREV_LAYER( &top_layer->layer_connection, suback,
                                       XI_STATE_OK );

        /* Expectations for the callback */
        expect_value( xi_itest_mqtt_logic_layer_sub_callback, call_type,
                      XI_SUB_CALL_SUBACK );
        expect_value( xi_itest_mqtt_logic_layer_sub_callback, state,
                      XI_MQTT_SUBSCRIPTION_SUCCESSFULL );

        /* let's process suback */
        xi_itest_mqttlogic_layer_act();

        /* let's close the connection gracefully */
        xi_itest_mqttlogic_shutdown_and_disconnect( context_handle );
    }

    return;
err_handling:
    xi_mqtt_message_free( &suback );
    xi_itest_mqttlogic_shutdown_and_disconnect( context_handle );
}

void xi_itest_mqtt_logic_layer__subscribe_failure__failed_suback_callback_invocation(
    void** state )
{
    XI_UNUSED( state );

    xi_state_t local_state             = XI_STATE_OK;
    xi_mqtt_message_t* suback          = NULL;
    xi_context_handle_t context_handle = XI_INVALID_CONTEXT_HANDLE;

    int it_qos_level = 0;
    for ( ; it_qos_level < XI_MQTT_QOS_TEST_LEVELS_COUNT; ++it_qos_level )
    {
        xi_debug_format( "Testing QoS level: %d", it_qos_level );

        /* initialisation of the layer chain */
        xi_layer_t* top_layer = xi_context__itest_mqttlogic_layer->layer_chain.top;
        xi_itest_mqttlogic_prepare_init_and_connect_layer( top_layer, XI_SESSION_CLEAN,
                                                           0 );
        xi_itest_mqttlogic_layer_act();

        /* let's extract the context handle in order to use it against the API functions
         */
        XI_CHECK_STATE( local_state = xi_find_handle_for_object(
                            xi_globals.context_handles_vector,
                            xi_context__itest_mqttlogic_layer, &context_handle ) );

        /* let's use the xively C library subscribe function */
        xi_subscribe( context_handle, "test_topic", it_qos_level,
                      &xi_itest_mqtt_logic_layer_sub_callback, NULL );

        expect_value( xi_mock_layer_mqttlogic_next_push, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_layer_mqttlogic_prev_push, in_out_state, XI_STATE_OK );

        expect_check(
            xi_mock_layer_mqttlogic_prev_push, data, check_msg,
            xi_itest_mqttlogic_make_msg_test_matrix(
                ( xi_itest_mqttlogic_test_msg_what_to_check_t ){
                    .retain = 0, .qos = 0, .dup = 0, .type = 1},
                ( xi_itest_mqttlogic_test_msg_common_bits_check_values_t ){
                    .retain = 0, .qos = 0, .dup = 0, .type = XI_MQTT_TYPE_SUBSCRIBE} ) );

        /* let's start the subscription process */
        xi_itest_mqttlogic_layer_act();

        /* let's send suback message */
        XI_ALLOC_AT( xi_mqtt_message_t, suback, local_state );
        suback->common.common_u.common_bits.type = XI_MQTT_TYPE_SUBACK;
        suback->suback.message_id                = 1;
        XI_ALLOC_AT( xi_mqtt_topicpair_t, suback->suback.topics, local_state );
        suback->suback.topics->xi_mqtt_topic_pair_payload_u.qos = 0;
        suback->suback.topics->xi_mqtt_topic_pair_payload_u.status =
            XI_MQTT_SUBACK_FAILED;
        XI_PROCESS_PULL_ON_PREV_LAYER( &top_layer->layer_connection, suback,
                                       XI_STATE_OK );

        /* Expectations for the callback */
        expect_value( xi_itest_mqtt_logic_layer_sub_callback, call_type,
                      XI_SUB_CALL_SUBACK );
        expect_value( xi_itest_mqtt_logic_layer_sub_callback, state,
                      XI_MQTT_SUBSCRIPTION_FAILED );

        /* let's process suback */
        xi_itest_mqttlogic_layer_act();

        /* let's close the connection gracefully */
        xi_itest_mqttlogic_shutdown_and_disconnect( context_handle );
    }

    return;
err_handling:
    xi_mqtt_message_free( &suback );
    xi_itest_mqttlogic_shutdown_and_disconnect( context_handle );
}

void xi_itest_mqtt_logic_layer__subscribe_success__success_message_callback_invocation(
    void** state )
{
    XI_UNUSED( state );

    xi_state_t local_state             = XI_STATE_OK;
    xi_mqtt_message_t* suback          = NULL;
    xi_context_handle_t context_handle = XI_INVALID_CONTEXT_HANDLE;

    /* prepare msg payload  */
    xi_data_desc_t* payload = xi_make_desc_from_string_copy( "some_data" );
    XI_CHECK_MEMORY( payload, local_state );

    /* prepare msg topic */
    const char* const topic = "test_topic";

    int it_qos_level = 0;
    for ( ; it_qos_level < XI_MQTT_QOS_TEST_LEVELS_COUNT; ++it_qos_level )
    {
        xi_debug_format( "Testing QoS level: %d", it_qos_level );

        /* initialisation of the layer chain */
        xi_layer_t* top_layer = xi_context__itest_mqttlogic_layer->layer_chain.top;
        xi_itest_mqttlogic_prepare_init_and_connect_layer( top_layer, XI_SESSION_CLEAN,
                                                           0 );
        xi_itest_mqttlogic_layer_act();

        /* let's extract the context handle in order to use it against the API functions
         */
        XI_CHECK_STATE( local_state = xi_find_handle_for_object(
                            xi_globals.context_handles_vector,
                            xi_context__itest_mqttlogic_layer, &context_handle ) );

        /* let's use the xively C library subscribe function */
        xi_subscribe( context_handle, topic, it_qos_level,
                      &xi_itest_mqtt_logic_layer_sub_callback, NULL );

        expect_value( xi_mock_layer_mqttlogic_next_push, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_layer_mqttlogic_prev_push, in_out_state, XI_STATE_OK );

        /* let's start the subscription process */
        xi_itest_mqttlogic_layer_act();

        /* let's send suback message */
        XI_ALLOC_AT( xi_mqtt_message_t, suback, local_state );
        suback->common.common_u.common_bits.type = XI_MQTT_TYPE_SUBACK;
        suback->suback.message_id                = 1;
        XI_ALLOC_AT( xi_mqtt_topicpair_t, suback->suback.topics, local_state );
        suback->suback.topics->xi_mqtt_topic_pair_payload_u.qos    = 0;
        suback->suback.topics->xi_mqtt_topic_pair_payload_u.status = it_qos_level;
        XI_PROCESS_PULL_ON_PREV_LAYER( &top_layer->layer_connection, suback,
                                       XI_STATE_OK );

        /* Expectations for the callback */
        expect_value( xi_itest_mqtt_logic_layer_sub_callback, call_type,
                      XI_SUB_CALL_SUBACK );
        expect_value( xi_itest_mqtt_logic_layer_sub_callback, state,
                      XI_MQTT_SUBSCRIPTION_SUCCESSFULL );

        /* let's process suback */
        xi_itest_mqttlogic_layer_act();

        /* let's pretend to send a message */
        XI_ALLOC( xi_mqtt_message_t, publish, local_state );

        local_state = fill_with_publish_data(
            publish, topic, payload, ( xi_mqtt_qos_t )it_qos_level, XI_MQTT_RETAIN_FALSE,
            XI_MQTT_DUP_FALSE, 3 );

        XI_CHECK_STATE( local_state );

        XI_PROCESS_PULL_ON_PREV_LAYER( &top_layer->layer_connection, publish,
                                       XI_STATE_OK );

        expect_value( xi_itest_mqtt_logic_layer_sub_callback, call_type,
                      XI_SUB_CALL_MESSAGE );
        expect_value( xi_itest_mqtt_logic_layer_sub_callback, state, XI_STATE_OK );

        if ( XI_MQTT_QOS_1 == it_qos_level )
        {
            expect_value( xi_mock_layer_mqttlogic_prev_push, in_out_state, XI_STATE_OK );
        }

        xi_itest_mqttlogic_layer_act();

        /* let's close the connection gracefully */
        xi_itest_mqttlogic_shutdown_and_disconnect( context_handle );
    }

    xi_free_desc( &payload );

    return;
err_handling:
    xi_free_desc( &payload );
    xi_mqtt_message_free( &suback );
    xi_itest_mqttlogic_shutdown_and_disconnect( context_handle );
}

void xi_itest_mqtt_logic_layer__persistant_session__unsent_messages_not_prereserved(
    void** state )
{
    XI_UNUSED( state );

    xi_state_t local_state             = XI_STATE_OK;
    xi_context_handle_t context_handle = XI_INVALID_CONTEXT_HANDLE;

    /* prepare msg payload  */
    const char* const msg_payload = "this is test message payload";

    /* prepare msg topic */
    const char* const topic = "test_topic";

    /* initialisation of the layer chain */
    xi_layer_t* top_layer = xi_context__itest_mqttlogic_layer->layer_chain.top;

    const size_t test_data_size =
        XI_ARRAYSIZE( XI_ITEST_MQTTLOGIC_PERSISTANT_SESSION_TEST_DATA );

    size_t keepalive_timeout = 0;
    for ( ; keepalive_timeout < 5; ++keepalive_timeout )
    {
        size_t i = 0;
        for ( ; i < test_data_size; ++i )
        {
            xi_debug_format(
                "Ruuning test on: %d msg type\n",
                XI_ITEST_MQTTLOGIC_PERSISTANT_SESSION_TEST_DATA[i].msg_type );

            /* we have to pretend that we are connected with clean session bit set to 0 */
            xi_itest_mqttlogic_prepare_init_and_connect_layer(
                top_layer, XI_SESSION_CONTINUE, keepalive_timeout );
            xi_itest_mqttlogic_layer_act();

            /* let's extract the context handle in order to use it against the API
             * functions
             */
            XI_CHECK_STATE( local_state = xi_find_handle_for_object(
                                xi_globals.context_handles_vector,
                                xi_context__itest_mqttlogic_layer, &context_handle ) );

            /* let's use the xively C library function */
            assert_int_equal( XI_ITEST_MQTTLOGIC_PERSISTANT_SESSION_TEST_DATA[i].api_call(
                                  context_handle, topic, msg_payload ),
                              XI_STATE_OK );

            expect_value( xi_mock_layer_mqttlogic_next_push, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_layer_mqttlogic_prev_push, in_out_state, XI_STATE_OK );

            /* make our mock layer is going to pretend that it has been disconnected */
            will_return( xi_mock_layer_mqttlogic_prev_push,
                         XI_CONNECTION_RESET_BY_PEER_ERROR );
            expect_value( xi_mock_layer_mqttlogic_next_close_externally, in_out_state,
                          XI_CONNECTION_RESET_BY_PEER_ERROR );

            assert_null( xi_context__itest_mqttlogic_layer->context_data
                             .copy_of_q12_unacked_messages_queue );

            /* let's start the publication and disconnection process */
            xi_itest_mqttlogic_layer_act();

            assert_null( xi_context__itest_mqttlogic_layer->context_data
                             .copy_of_q12_unacked_messages_queue );
            assert_non_null( xi_context__itest_mqttlogic_layer->context_data
                                 .copy_of_q12_unacked_messages_queue_dtor_ptr );

            xi_free_connection_data(
                &xi_context__itest_mqttlogic_layer->context_data.connection_data );
        }
    }

    return;
err_handling:
    xi_free_connection_data(
        &xi_context__itest_mqttlogic_layer->context_data.connection_data );
}

void xi_itest_mqtt_logic_layer__persistant_session__failure_unsent_message_are_not_resend_after_reconnect(
    void** state )
{
    XI_UNUSED( state );

    xi_state_t local_state             = XI_STATE_OK;
    xi_context_handle_t context_handle = XI_INVALID_CONTEXT_HANDLE;

    /* prepare msg payload  */
    const char* const msg_payload = "this is test message payload";

    /* prepare msg topic */
    const char* const topic = "test_topic";

    /* initialisation of the layer chain */
    xi_layer_t* top_layer = xi_context__itest_mqttlogic_layer->layer_chain.top;

    const size_t test_data_size =
        XI_ARRAYSIZE( XI_ITEST_MQTTLOGIC_PERSISTANT_SESSION_TEST_DATA );

    size_t keepalive_timeout = 0;
    for ( ; keepalive_timeout < 5; ++keepalive_timeout )
    {
        size_t i = 0;
        for ( ; i < test_data_size; ++i )
        {
            xi_debug_format(
                "Ruuning test on: %d msg type\n",
                XI_ITEST_MQTTLOGIC_PERSISTANT_SESSION_TEST_DATA[i].msg_type );

            /* we have to pretend that we are connected with clean session bit set to 0 */
            xi_itest_mqttlogic_prepare_init_and_connect_layer(
                top_layer, XI_SESSION_CONTINUE, keepalive_timeout );
            xi_itest_mqttlogic_layer_act();

            /* let's extract the context handle in order to use it against the API
             * functions
             */
            XI_CHECK_STATE( local_state = xi_find_handle_for_object(
                                xi_globals.context_handles_vector,
                                xi_context__itest_mqttlogic_layer, &context_handle ) );

            /* let's use the xively C library function */
            assert_int_equal( XI_ITEST_MQTTLOGIC_PERSISTANT_SESSION_TEST_DATA[i].api_call(
                                  context_handle, topic, msg_payload ),
                              XI_STATE_OK );

            expect_value( xi_mock_layer_mqttlogic_next_push, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_layer_mqttlogic_prev_push, in_out_state, XI_STATE_OK );

            /* make our mock layer is going to pretend that it has been disconnected */
            will_return( xi_mock_layer_mqttlogic_prev_push,
                         XI_CONNECTION_RESET_BY_PEER_ERROR );
            expect_value( xi_mock_layer_mqttlogic_next_close_externally, in_out_state,
                          XI_CONNECTION_RESET_BY_PEER_ERROR );

            assert_null( xi_context__itest_mqttlogic_layer->context_data
                             .copy_of_q12_unacked_messages_queue );

            /* let's start the publication and disconnection process */
            xi_itest_mqttlogic_layer_act();

            assert_null( xi_context__itest_mqttlogic_layer->context_data
                             .copy_of_q12_unacked_messages_queue );
            assert_non_null( xi_context__itest_mqttlogic_layer->context_data
                                 .copy_of_q12_unacked_messages_queue_dtor_ptr );

            /* it will be created again within the connect layer helper function */
            xi_free_connection_data(
                &xi_context__itest_mqttlogic_layer->context_data.connection_data );

            /* let's reconnect with session continue no messages resend */
            xi_itest_mqttlogic_prepare_init_and_connect_layer( top_layer,
                                                               XI_SESSION_CONTINUE, 0 );
            xi_itest_mqttlogic_layer_act();

            xi_itest_mqttlogic_shutdown_and_disconnect( context_handle );
        }
    }

    return;
err_handling:
    xi_itest_mqttlogic_shutdown_and_disconnect( context_handle );
}

void xi_itest_mqtt_logic_layer__persistant_session__success_unacked_messages_are_resend_after_reconnect(
    void** state )
{
    XI_UNUSED( state );

    xi_state_t local_state             = XI_STATE_OK;
    xi_context_handle_t context_handle = XI_INVALID_CONTEXT_HANDLE;

    /* prepare msg payload  */
    const char* const msg_payload = "this is test message payload";

    /* prepare msg topic */
    const char* const topic = "test_topic";

    const size_t test_data_size =
        XI_ARRAYSIZE( XI_ITEST_MQTTLOGIC_PERSISTANT_SESSION_TEST_DATA );

    size_t keepalive_timeout = 0;
    for ( ; keepalive_timeout < 5; ++keepalive_timeout )
    {
        size_t i = 0;
        for ( ; i < test_data_size; ++i )
        {
            /* initialisation of the layer chain */
            xi_layer_t* top_layer = xi_context__itest_mqttlogic_layer->layer_chain.top;
            xi_layer_t* bottom_layer =
                xi_context__itest_mqttlogic_layer->layer_chain.bottom;

            xi_mqtt_type_t msg_type =
                XI_ITEST_MQTTLOGIC_PERSISTANT_SESSION_TEST_DATA[i].msg_type;
            xi_debug_format( "Ruuning test on: %d msg type\n", msg_type );

            /* we have to pretend that we are connected with clean session bit set to 0 */
            xi_itest_mqttlogic_prepare_init_and_connect_layer(
                top_layer, XI_SESSION_CONTINUE, keepalive_timeout );

            /* let's extract the context handle in order to use it against the API
             * functions
             */
            XI_CHECK_STATE( local_state = xi_find_handle_for_object(
                                xi_globals.context_handles_vector,
                                xi_context__itest_mqttlogic_layer, &context_handle ) );

            /* let's use the xively C library function */
            assert_int_equal( XI_ITEST_MQTTLOGIC_PERSISTANT_SESSION_TEST_DATA[i].api_call(
                                  context_handle, topic, msg_payload ),
                              XI_STATE_OK );

            /* expectations for the next push function */
            expect_value( xi_mock_layer_mqttlogic_next_push, in_out_state, XI_STATE_OK );

            /* expectations for the prev push  */
            expect_value( xi_mock_layer_mqttlogic_prev_push, in_out_state, XI_STATE_OK );
            expect_check( xi_mock_layer_mqttlogic_prev_push, data, check_msg,
                          xi_itest_mqttlogic_make_msg_test_matrix(
                              ( xi_itest_mqttlogic_test_msg_what_to_check_t ){
                                  .retain = 0, .qos = 1, .dup = 1, .type = 1},
                              ( xi_itest_mqttlogic_test_msg_common_bits_check_values_t ){
                                  .retain = 0, .qos = 1, .dup = 0, .type = msg_type} ) );

            /* make our mock layer is going to pretend that it has been disconnected */
            will_return( xi_mock_layer_mqttlogic_prev_push, XI_STATE_OK );

            assert_null( xi_context__itest_mqttlogic_layer->context_data
                             .copy_of_q12_unacked_messages_queue );

            /* let's start the publication process it should yield on waiting for PUBACK
             */
            xi_itest_mqttlogic_layer_act();

            /* let's pretend that something bad happened like disconnection */
            XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(
                &bottom_layer->layer_connection, NULL,
                XI_CONNECTION_RESET_BY_PEER_ERROR );

            /* our layers should call this up to the top */
            expect_value( xi_mock_layer_mqttlogic_next_close_externally, in_out_state,
                          XI_CONNECTION_RESET_BY_PEER_ERROR );
            expect_value( xi_mock_layer_mqttlogic_prev_close_externally, in_out_state,
                          XI_CONNECTION_RESET_BY_PEER_ERROR );

            /* let's make it happen */
            xi_itest_mqttlogic_layer_act();

            assert_non_null( xi_context__itest_mqttlogic_layer->context_data
                                 .copy_of_q12_unacked_messages_queue );
            assert_non_null( xi_context__itest_mqttlogic_layer->context_data
                                 .copy_of_q12_unacked_messages_queue_dtor_ptr );

            /* during the re-connect our messages are going to be re-send, let's set up a
             * proper
             * expectations */

            /* it will be created again within the connect layer helper function */
            xi_free_connection_data(
                &xi_context__itest_mqttlogic_layer->context_data.connection_data );

            /* let's reconnect with session continue so that our messages should be resend
             */
            xi_itest_mqttlogic_prepare_init_and_connect_layer(
                top_layer, XI_SESSION_CONTINUE, keepalive_timeout );

            expect_value( xi_mock_layer_mqttlogic_prev_push, in_out_state, XI_STATE_OK );
            expect_check( xi_mock_layer_mqttlogic_prev_push, data, check_msg,
                          xi_itest_mqttlogic_make_msg_test_matrix(
                              ( xi_itest_mqttlogic_test_msg_what_to_check_t ){
                                  .retain = 1, .qos = 1, .dup = 1, .type = 1},
                              ( xi_itest_mqttlogic_test_msg_common_bits_check_values_t ){
                                  .retain = 0, .qos = 1, .dup = 1, .type = msg_type} ) );
            will_return( xi_mock_layer_mqttlogic_prev_push, XI_STATE_OK );
            xi_itest_mqttlogic_layer_act();

            xi_itest_mqttlogic_shutdown_and_disconnect( context_handle );

            /* teardown and tearup in order to clean the qos persistance session cache */
            xi_itest_mqttlogic_layer_teardown( NULL );
            xi_itest_mqttlogic_layer_setup( NULL );
        }
    }

    return;
err_handling:
    xi_itest_mqttlogic_shutdown_and_disconnect( context_handle );
}
