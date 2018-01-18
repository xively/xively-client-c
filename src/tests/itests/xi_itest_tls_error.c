/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_backoff_status_api.h"
#include "xi_globals.h"
#include "xi_itest_helpers.h"
#include "xi_itest_tls_error.h"

#include "xi_handle.h"
#include "xi_itest_layerchain_ct_ml_mc.h"
#include "xi_itest_mock_broker_layerchain.h"
#include "xi_memory_checks.h"
#include "xi_bsp_time.h"

extern char xi_test_load_level;

xi_context_t* xi_context              = NULL;
xi_context_handle_t xi_context_handle = XI_INVALID_CONTEXT_HANDLE;
xi_context_t* xi_context_mockbroker   = NULL;

/**
 * xi_itest_tls_error test suit description
 *
 * System Under Test: 3 layers: Control Topic (CT), Mqtt Logic (ML), Mqtt Codec (MC)
 *
 * Test consists of two layer chains:
 *   - SUT layer chain: CT-ML-MC-MB-TLSPREV, where
 *      - MB      = Mock Broker / Mock TLS layer (in production MC is preceded by TLS)
 *        TLSPREV = an artificial mock layer serving as a previous layer for TLS layer
 *   - Mock Broker (MB) layer chain: MB-MC-MBS, where
 *      - MBS     = Mock Broker Secondary layer, a "helper layer"
 *      - here MC is the libxively's Mqtt Codec layer. Mock Broker uses this production
 *        version Mqtt Codec layer to encode / decode mqtt messages
 *
 * The two layer chains intersect each other:
 *
 * xi_itest_layerchain_ct_ml_mc.h
 *  |
 *  L->   CT - ML - MC - MB - TLSPREV
 *                       |
 *                       MC
 *                       |
 *                       MBS
 *
 *                       A
 *                       |
 *                       |
 *                  xi_itest_mock_broker_layerchain.h
 *
 *
 * The horizontal layer chain acts like libxively, MB intercepts messages from the
 * 3 SUT layers, forwards it to two directions: TLSPREV which behaves like IO layer
 * and MC for decoding mqtt messages.
 * The horizontal layer chain's MB and TLSPREV is "programmed" to mimic errors at
 * various stages of functioning.
 *
 * NOTE: The xi_mock_broker_layer is instantiated twice. That's why we expect to see two
 * invocations of xi_mock_broker_layer_close function. One per each layer chain.
 */

/*********************************************************************************
 * test fixture ******************************************************************
 ********************************************************************************/
typedef struct xi_itest_tls_error__test_fixture_s
{
    const char* test_topic_name;
    const char* test_full_topic_name;
    const char* control_topic_name;

    uint8_t loop_id__control_topic_auto_subscribe;
    uint8_t loop_id__manual_publish;
    uint8_t loop_id__manual_disconnect;

    uint8_t max_loop_count;

} xi_itest_tls_error__test_fixture_t;

xi_itest_tls_error__test_fixture_t* xi_itest_tls_error__generate_fixture()
{
    xi_state_t xi_state = XI_STATE_OK;

    XI_ALLOC( xi_itest_tls_error__test_fixture_t, fixture, xi_state );

    fixture->test_topic_name      = ( "test/topic/name" );
    fixture->test_full_topic_name = ( "xi/blue/v1/xi_itest_tls_error_account_id/d/"
                                      "xi_itest_tls_error_device_id/test/topic/name" );
    fixture->control_topic_name = ( "xi/ctrl/v1/xi_itest_tls_error_device_id/cln" );

/* control_topic_auto_subscribe is 2 because control topic subscription happens in the 3rd
 * loop, this is precise timed simulation, some test cases are sensitive for timing*/
#ifdef XI_CONTROL_TOPIC_ENABLED
    fixture->loop_id__control_topic_auto_subscribe = 2;
#else
    fixture->loop_id__control_topic_auto_subscribe = 3;
#endif

    fixture->loop_id__manual_publish    = 6;
    fixture->loop_id__manual_disconnect = 15;
    fixture->max_loop_count             = 20;

    return fixture;

err_handling:
    fail();

    return NULL;
}

/*********************************************************************************
 * setup / teardown **************************************************************
 ********************************************************************************/
int xi_itest_tls_error_setup( void** fixture_void )
{
    XI_UNUSED( fixture_void );

    xi_memory_limiter_tearup();

    *fixture_void = xi_itest_tls_error__generate_fixture();

    xi_globals.backoff_status.backoff_lut_i = 0;
    xi_cancel_backoff_event();

    xi_initialize( "xi_itest_tls_error_account_id", "xi_itest_tls_error_device_id" );

    XI_CHECK_STATE( xi_create_context_with_custom_layers(
        &xi_context, itest_ct_ml_mc_layer_chain, XI_LAYER_CHAIN_CT_ML_MC,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_CT_ML_MC ) ) );

    xi_find_handle_for_object( xi_globals.context_handles_vector, xi_context,
                               &xi_context_handle );

    XI_CHECK_STATE( xi_create_context_with_custom_layers(
        &xi_context_mockbroker, itest_mock_broker_codec_layer_chain,
        XI_LAYER_CHAIN_MOCK_BROKER_CODEC,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_MOCK_BROKER_CODEC ) ) );

    return 0;

err_handling:
    fail();

    return 1;
}

int xi_itest_tls_error_teardown( void** fixture_void )
{
    XI_UNUSED( fixture_void );

    xi_delete_context( xi_context_handle );
    xi_delete_context_with_custom_layers(
        &xi_context_mockbroker, itest_mock_broker_codec_layer_chain,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_MOCK_BROKER_CODEC ) );

    xi_shutdown();

    xi_itest_tls_error__test_fixture_t* fixture =
        ( xi_itest_tls_error__test_fixture_t* )*fixture_void;

    XI_SAFE_FREE( fixture );

    return !xi_memory_limiter_teardown();
}

#ifndef XI_CONTROL_TOPIC_ENABLED
void on_publish_received( xi_context_handle_t in_context_handle,
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
#endif

void tls_error_on_connection_state_changed( xi_context_handle_t in_context_handle,
                                            void* data,
                                            xi_state_t state )
{
    XI_UNUSED( in_context_handle );
    XI_UNUSED( data );
    XI_UNUSED( state );
}

/**
 * @name    xi_itest_tls_error__act
 * @brief   mimics a libxively live environment: drives event dispatcher
 *
 *
 * Does a limited number of iterations, processes some events. Acts
 * like a live libxively environment. Parameters can define custom actions:
 * publish, disconnect.
 *
 * @param [in] fixture_void         test case specific settings
 * @param [in] do_publish_flag      if set a publish is requested on the API
 * @param [in] do_publish_flag      if set a disconnect is requested on the API
 */
static void xi_itest_tls_error__act( void** fixture_void,
                                     char do_publish_flag,
                                     char do_disconnect_flag )
{
    XI_PROCESS_INIT_ON_THIS_LAYER(
        &xi_context_mockbroker->layer_chain.top->layer_connection, NULL, XI_STATE_OK );

    xi_evtd_step( xi_globals.evtd_instance, xi_bsp_time_getcurrenttime_seconds() );

    const xi_itest_tls_error__test_fixture_t* const fixture =
        ( xi_itest_tls_error__test_fixture_t* )*fixture_void;

    const uint16_t connection_timeout = 20;
    const uint16_t keepalive_timeout  = fixture->max_loop_count;
    xi_connect( xi_context_handle, "itest_username", "itest_password", connection_timeout,
                keepalive_timeout, XI_SESSION_CLEAN,
                &tls_error_on_connection_state_changed );

    uint8_t loop_counter = 0;
    while ( xi_evtd_dispatcher_continue( xi_globals.evtd_instance ) == 1 &&
            loop_counter < keepalive_timeout )
    {
        xi_evtd_step( xi_globals.evtd_instance,
                      xi_bsp_time_getcurrenttime_seconds() + loop_counter );
        ++loop_counter;

#ifndef XI_CONTROL_TOPIC_ENABLED
        if ( loop_counter == fixture->loop_id__control_topic_auto_subscribe )
        {
            xi_subscribe( xi_context_handle, fixture->control_topic_name,
                          XI_MQTT_QOS_AT_LEAST_ONCE, on_publish_received, NULL );
        }
#endif

        if ( do_publish_flag && loop_counter == fixture->loop_id__manual_publish )
        {
            xi_publish( xi_context_handle, fixture->test_topic_name, "test message",
                        XI_MQTT_QOS_AT_LEAST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
        }

        if ( do_disconnect_flag && loop_counter == fixture->loop_id__manual_disconnect )
        {
            xi_shutdown_connection( xi_context_handle );
        }
    }
}

uint8_t xi_itest_tls_error__load_level_filter_PUSH( uint8_t xi_state_error_code )
{
    return ( 0 < xi_test_load_level ||
             /* here probable error codes are collected to reduce runtime for rapid*/
             /* testing*/
             xi_state_error_code == XI_STATE_FAILED_WRITING ||
             xi_state_error_code == XI_CONNECTION_RESET_BY_PEER_ERROR ||
             xi_state_error_code == XI_SOCKET_WRITE_ERROR )
               ? 1
               : 0;
}

/*********************************************************************************
 * test cases ********************************************************************
 ********************************************************************************/
void xi_itest_tls_error__tls_pull_PUBACK_errors__graceful_error_handling(
    void** fixture_void )
{
    uint8_t xi_state_error_code = XI_STATE_OK + 1;
    for ( ; xi_state_error_code < XI_ERROR_COUNT; ++xi_state_error_code )
    {
        if ( xi_state_error_code != XI_OUT_OF_MEMORY &&
             xi_state_error_code != XI_INTERNAL_ERROR &&
             xi_state_error_code != XI_MQTT_UNKNOWN_MESSAGE_ID &&
             xi_itest_tls_error__load_level_filter_PUSH( xi_state_error_code ) )
        {
            const xi_itest_tls_error__test_fixture_t* const fixture =
                ( xi_itest_tls_error__test_fixture_t* )*fixture_void;

            /* one call for mock broker layer chain init*/
            expect_value( xi_mock_broker_layer_init, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_layer_tls_prev_init, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_layer_tls_prev_connect, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_connect, in_out_state, XI_STATE_OK );

            expect_value( xi_mock_broker_layer_init, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_connect, in_out_state, XI_STATE_OK );

            /* no problem during CONNECT*/
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
            expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );

            /* CONNECT message arrives at mock broker*/
            expect_value( xi_mock_broker_layer_pull, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_pull, recvd_msg_type,
                          XI_MQTT_TYPE_CONNECT );

            /* CONNACK sent*/
            expect_value( xi_mock_broker_secondary_layer_push, in_out_state,
                          XI_STATE_OK );
            will_return( xi_mock_broker_secondary_layer_push, CONTROL_CONTINUE );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );

            /* second message (probably SUBSCRIBE on a control topic)*/
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
            expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );

            /* SUBSCRIBE message arrives at mock broker*/
            expect_value( xi_mock_broker_layer_pull, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_pull, recvd_msg_type,
                          XI_MQTT_TYPE_SUBSCRIBE );
#ifdef XI_CONTROL_TOPIC_ENABLED
            expect_string( xi_mock_broker_layer_pull, subscribe_topic_name,
                           fixture->control_topic_name );
#else
            expect_any( xi_mock_broker_layer_pull, subscribe_topic_name );
#endif


            /* SUBACK sent*/
            expect_value( xi_mock_broker_secondary_layer_push, in_out_state,
                          XI_STATE_OK );
            will_return( xi_mock_broker_secondary_layer_push, CONTROL_CONTINUE );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );

            /* PUBLISH HEADER*/
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
            expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );

            /* PUBLISH PAYLOAD*/
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
            expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );

            /* PUBLISH message arrives at mock broker*/
            expect_value( xi_mock_broker_layer_pull, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_pull, recvd_msg_type,
                          XI_MQTT_TYPE_PUBLISH );
            const char* test_topic_name = NULL;
            if ( NULL != fixture )
            {
#ifdef XI_MANGLE_TOPIC
                test_topic_name = fixture->test_full_topic_name;
#else
                test_topic_name = fixture->test_topic_name;
#endif
            }
            expect_string( xi_mock_broker_layer_pull, publish_topic_name,
                           test_topic_name );

            /* PUBACK sent*/
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );

            /* PUBACK receive error*/
            expect_value( xi_mock_broker_secondary_layer_push, in_out_state,
                          XI_STATE_OK );
            /* --- ERROR --- HERE --- ERROR --- HERE --- ERROR ---*/
            will_return( xi_mock_broker_secondary_layer_push, CONTROL_ERROR );
            will_return( xi_mock_broker_secondary_layer_push, xi_state_error_code );

            /* First close is called upon  */
            expect_value( xi_mock_broker_layer_close, in_out_state, xi_state_error_code );
            expect_value( xi_mock_broker_layer_close, in_out_state, xi_state_error_code );
            expect_value( xi_mock_layer_tls_prev_close, in_out_state,
                          xi_state_error_code );
            expect_value( xi_mock_layer_tls_prev_close_externally, in_out_state,
                          xi_state_error_code );

            xi_itest_tls_error__act( fixture_void, 1, 1 );

            /* artificially reset test case*/
            xi_itest_tls_error_teardown( fixture_void );
            xi_itest_tls_error_setup( fixture_void );
        }
    }
}

void xi_itest_tls_error__tls_pull_SUBACK_errors__graceful_error_handling(
    void** fixture_void )
{
    uint8_t xi_state_error_code = XI_STATE_OK + 1;
    for ( ; xi_state_error_code < XI_ERROR_COUNT; ++xi_state_error_code )
    {
        if ( xi_state_error_code != XI_OUT_OF_MEMORY &&
             xi_state_error_code != XI_INTERNAL_ERROR &&
             xi_state_error_code != XI_MQTT_UNKNOWN_MESSAGE_ID &&
             xi_itest_tls_error__load_level_filter_PUSH( xi_state_error_code ) )
        {
            /* one call for mock broker layer chain init*/
            expect_value( xi_mock_broker_layer_init, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_layer_tls_prev_init, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_layer_tls_prev_connect, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_connect, in_out_state, XI_STATE_OK );

            expect_value( xi_mock_broker_layer_init, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_connect, in_out_state, XI_STATE_OK );

            /* no problem during CONNECT*/
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
            expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );

            /* CONNECT message arrives at mock broker*/
            expect_value( xi_mock_broker_layer_pull, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_pull, recvd_msg_type,
                          XI_MQTT_TYPE_CONNECT );

            /* CONNACK sent*/
            expect_value( xi_mock_broker_secondary_layer_push, in_out_state,
                          XI_STATE_OK );
            will_return( xi_mock_broker_secondary_layer_push, CONTROL_CONTINUE );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );

            /* second message (probably SUBSCRIBE on a control topic)*/
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
            expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );

            /* SUBSCRIBE message arrives at mock broker*/
            expect_value( xi_mock_broker_layer_pull, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_pull, recvd_msg_type,
                          XI_MQTT_TYPE_SUBSCRIBE );

#ifdef XI_CONTROL_TOPIC_ENABLED
            const xi_itest_tls_error__test_fixture_t* const fixture =
                ( xi_itest_tls_error__test_fixture_t* )*fixture_void;

            expect_string( xi_mock_broker_layer_pull, subscribe_topic_name,
                           fixture->control_topic_name );
#else
            expect_any( xi_mock_broker_layer_pull, subscribe_topic_name );
#endif

            /* SUBACK receive error*/
            expect_value( xi_mock_broker_secondary_layer_push, in_out_state,
                          XI_STATE_OK );
            /* --- ERROR --- HERE --- ERROR --- HERE --- ERROR ---*/
            will_return( xi_mock_broker_secondary_layer_push, CONTROL_ERROR );
            will_return( xi_mock_broker_secondary_layer_push, xi_state_error_code );
            /* SUBACK sent*/
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
            will_return( xi_mock_broker_layer_push, CONTROL_CONTINUE );

            expect_value( xi_mock_broker_layer_close, in_out_state, xi_state_error_code );
            expect_value( xi_mock_broker_layer_close, in_out_state, xi_state_error_code );
            expect_value( xi_mock_layer_tls_prev_close, in_out_state,
                          xi_state_error_code );
            expect_value( xi_mock_layer_tls_prev_close_externally, in_out_state,
                          xi_state_error_code );

            xi_itest_tls_error__act( fixture_void, 0, 1 );

            /* artificially reset test case*/
            xi_itest_tls_error_teardown( fixture_void );
            xi_itest_tls_error_setup( fixture_void );
        }
    }
}

void xi_itest_tls_error__tls_pull_CONNACK_errors__graceful_error_handling(
    void** fixture_void )
{
    XI_UNUSED( fixture_void );

    uint8_t xi_state_error_code = XI_STATE_OK + 1;
    for ( ; xi_state_error_code < XI_ERROR_COUNT; ++xi_state_error_code )
    {
        if ( xi_state_error_code != XI_OUT_OF_MEMORY &&
             xi_state_error_code != XI_INTERNAL_ERROR &&
             xi_state_error_code != XI_MQTT_UNKNOWN_MESSAGE_ID &&
             xi_itest_tls_error__load_level_filter_PUSH( xi_state_error_code ) )
        {
            /* one call for mock broker layer chain init*/
            expect_value( xi_mock_broker_layer_init, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_layer_tls_prev_init, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_layer_tls_prev_connect, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_connect, in_out_state, XI_STATE_OK );

            expect_value( xi_mock_broker_layer_init, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_connect, in_out_state, XI_STATE_OK );

            /* no problem during CONNECT*/
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
            expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );

            /* CONNECT message arrives at mock broker*/
            expect_value( xi_mock_broker_layer_pull, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_pull, recvd_msg_type,
                          XI_MQTT_TYPE_CONNECT );

            /* CONNACK receive error*/
            expect_value( xi_mock_broker_secondary_layer_push, in_out_state,
                          XI_STATE_OK );
            /* --- ERROR --- HERE --- ERROR --- HERE --- ERROR ---*/
            will_return( xi_mock_broker_secondary_layer_push, CONTROL_ERROR );
            will_return( xi_mock_broker_secondary_layer_push, xi_state_error_code );
            /* CONNACK sent*/
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
            will_return( xi_mock_broker_layer_push, CONTROL_CONTINUE );

            expect_value( xi_mock_broker_layer_close, in_out_state, xi_state_error_code );
            expect_value( xi_mock_broker_layer_close, in_out_state, xi_state_error_code );
            expect_value( xi_mock_layer_tls_prev_close, in_out_state,
                          xi_state_error_code );
            expect_value( xi_mock_layer_tls_prev_close_externally, in_out_state,
                          xi_state_error_code );

            xi_itest_tls_error__act( fixture_void, 0, 1 );

            /* artificially reset test case*/
            xi_itest_tls_error_teardown( fixture_void );
            xi_itest_tls_error_setup( fixture_void );
        }
    }
}

void xi_itest_tls_error__tls_push_CONNECT_errors__graceful_error_handling(
    void** fixture_void )
{
    XI_UNUSED( fixture_void );

    uint8_t xi_state_error_code = XI_STATE_OK + 1;
    for ( ; xi_state_error_code < XI_ERROR_COUNT; ++xi_state_error_code )
    {
        /* noteatigyi: this restriction is because mqtt codec layer push function is not
         * robust enough it only handles XI_STATE_FAILED_WRITING error, otherwise it makes
         * an assert */
        if ( xi_state_error_code == XI_STATE_FAILED_WRITING )
        {
            /* one call for mock broker layer chain init*/
            expect_value( xi_mock_broker_layer_init, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_layer_tls_prev_init, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_layer_tls_prev_connect, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_connect, in_out_state, XI_STATE_OK );

            expect_value( xi_mock_broker_layer_init, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_connect, in_out_state, XI_STATE_OK );

            /* error during CONNECT*/
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
            /* --- ERROR --- HERE --- ERROR --- HERE --- ERROR ---*/
            will_return( xi_mock_broker_layer_push, CONTROL_ERROR );
            will_return( xi_mock_broker_layer_push__ERROR_CHANNEL, xi_state_error_code );

            expect_value( xi_mock_broker_layer_close, in_out_state, xi_state_error_code );

            expect_value( xi_mock_layer_tls_prev_close, in_out_state,
                          xi_state_error_code );
            expect_value( xi_mock_layer_tls_prev_close_externally, in_out_state,
                          xi_state_error_code );
            expect_value( xi_mock_broker_layer_close, in_out_state, xi_state_error_code );

            xi_itest_tls_error__act( fixture_void, 0, 0 );

            /* artificially reset test case*/
            xi_itest_tls_error_teardown( fixture_void );
            xi_itest_tls_error_setup( fixture_void );
        }
    }
}

void xi_itest_tls_error__tls_push_infinite_SUBSCRIBE_errors__reSUBSCRIBE_occurs_once_in_a_second(
    void** fixture_void )
{
    uint8_t xi_state_error_code = XI_STATE_OK + 1;
    for ( ; xi_state_error_code < XI_ERROR_COUNT; ++xi_state_error_code )
    {
        /* noteatigyi: this restriction is because mqtt codec layer push function is not
         * robust enough it only handles XI_STATE_FAILED_WRITING error, otherwise it makes
         * an assert */
        if ( xi_state_error_code == XI_STATE_FAILED_WRITING )
        {
            const xi_itest_tls_error__test_fixture_t* const fixture =
                ( xi_itest_tls_error__test_fixture_t* )*fixture_void;

            /* one call for mock broker layer chain init*/
            expect_value( xi_mock_broker_layer_init, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_layer_tls_prev_init, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_layer_tls_prev_connect, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_connect, in_out_state, XI_STATE_OK );

            expect_value( xi_mock_broker_layer_init, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_connect, in_out_state, XI_STATE_OK );

            /* no problem during CONNECT*/
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
            will_return_count( xi_mock_broker_layer_push, CONTROL_CONTINUE, 2 );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
            expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );

            /* CONNECT message arrives at mock broker*/
            expect_value( xi_mock_broker_layer_pull, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_pull, recvd_msg_type,
                          XI_MQTT_TYPE_CONNECT );

            /* CONNACK sent*/
            expect_value( xi_mock_broker_secondary_layer_push, in_out_state,
                          XI_STATE_OK );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
            will_return( xi_mock_broker_layer_push, CONTROL_CONTINUE );

            /* ERROR HERE: always drive mock TLS layer to report error upon each SUBSCRIBE
             * send*/
            will_return_always( xi_mock_broker_layer_push, CONTROL_ERROR );
            will_return_always( xi_mock_broker_layer_push__ERROR_CHANNEL,
                                xi_state_error_code );

            /* EXPECTATION: here we calculate how many times we expect the
             * xi_mock_blocker_layer_push is going to be called. This value depends on how
             * often the Xively will try to send data. In this test we are taking into
             * account the number of SUBSCRIBES plus the number of DISCONNECTS. Number of
             * SUBSCRIBES depends on the number of loops in which Xively logic will try to
             * send and re-send SUBSCRIBES. Quantity of loops depends on the test setup.
             * In our case it is the difference between the loop number described by
             * fixture->loop_id__manual_disconnect and
             * fixture->loop_id__control_topic_auto_subscribe. This difference however
             * does not take into account the additional DISCONNECT message and one
             * additional re-SUBSCRIBE that will be sent in that last loop iteration
             * that's why we have to increase this value by 2.*/
            const uint8_t expected_number_of_PUSHES =
                fixture->loop_id__manual_disconnect -
                fixture->loop_id__control_topic_auto_subscribe + 2;

            /* expecting only a certain number of message sends*/
            expect_value_count( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK,
                                expected_number_of_PUSHES );

            expect_value( xi_mock_broker_layer_close, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_close, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_layer_tls_prev_close, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_layer_tls_prev_close_externally, in_out_state,
                          XI_STATE_OK );

            /* This suppresses warning messages, helps debugging these test cases.*/
            /*will_return( xi_mock_broker_layer_init, CONTROL_CONTINUE );
            will_return( xi_mock_broker_layer_init, CONTROL_CONTINUE );
            will_return( xi_mock_layer_tls_prev_push, CONTROL_CONTINUE );
            will_return( xi_mock_broker_secondary_layer_push, CONTROL_CONTINUE );*/

            /* In order to release the whole allocated memory we have make the test to
             * start close process on each layer that's why the manual disconnect flag is
             * set */
            xi_itest_tls_error__act( fixture_void, 0, 1 );

            /* artificially reset test case*/
            xi_itest_tls_error_teardown( fixture_void );
            xi_itest_tls_error_setup( fixture_void );
        }
    }
}

void xi_itest_tls_error__tls_push_SUBSCRIBE_errors__graceful_error_handling(
    void** fixture_void )
{
    uint8_t xi_state_error_code = XI_STATE_OK + 1;
    for ( ; xi_state_error_code < XI_ERROR_COUNT; ++xi_state_error_code )
    {
        /* noteatigyi: this restriction is because mqtt codec layer push function is not
         * robust enough it only handles XI_STATE_FAILED_WRITING error, otherwise it makes
         * an assert */
        if ( xi_state_error_code == XI_STATE_FAILED_WRITING )
        {
            const xi_itest_tls_error__test_fixture_t* const fixture =
                ( xi_itest_tls_error__test_fixture_t* )*fixture_void;

            /* one call for mock broker layer chain init*/
            expect_value( xi_mock_broker_layer_init, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_layer_tls_prev_init, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_layer_tls_prev_connect, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_connect, in_out_state, XI_STATE_OK );

            expect_value( xi_mock_broker_layer_init, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_connect, in_out_state, XI_STATE_OK );

            /* no problem during CONNECT*/
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
            will_return_count( xi_mock_broker_layer_push, CONTROL_CONTINUE, 2 );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
            expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );

            /* CONNECT message arrives at mock broker*/
            expect_value( xi_mock_broker_layer_pull, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_pull, recvd_msg_type,
                          XI_MQTT_TYPE_CONNECT );

            /* CONNACK sent*/
            expect_value( xi_mock_broker_secondary_layer_push, in_out_state,
                          XI_STATE_OK );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
            will_return( xi_mock_broker_layer_push, CONTROL_CONTINUE );

            const uint8_t expected_number_of_SUBSCRIBE_errors =
                fixture->loop_id__manual_disconnect -
                1 /* -1: DISCONNECT happens before SUBSCRIBE */
                - fixture->loop_id__control_topic_auto_subscribe -
                1; /* -1: the last SUBSCRIBE succeeds */

            assert_in_range( expected_number_of_SUBSCRIBE_errors, 1,
                             fixture->max_loop_count );

            /* error during SUBSCRIBE */
            expect_value_count( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK,
                                expected_number_of_SUBSCRIBE_errors );
            /* --- ERROR --- HERE --- ERROR --- HERE --- ERROR --- */
            will_return_count( xi_mock_broker_layer_push, CONTROL_ERROR,
                               expected_number_of_SUBSCRIBE_errors );
            will_return_count( xi_mock_broker_layer_push__ERROR_CHANNEL,
                               xi_state_error_code, expected_number_of_SUBSCRIBE_errors );

            /* SUBSCRIBE success */
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
            will_return_count( xi_mock_broker_layer_push, CONTROL_CONTINUE, 2 );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
            expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );

            /* SUBSCRIBE message arrives at mock broker */
            expect_value( xi_mock_broker_layer_pull, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_pull, recvd_msg_type,
                          XI_MQTT_TYPE_SUBSCRIBE );
#ifdef XI_CONTROL_TOPIC_ENABLED
            expect_string( xi_mock_broker_layer_pull, subscribe_topic_name,
                           fixture->control_topic_name );
#else
            expect_any( xi_mock_broker_layer_pull, subscribe_topic_name );
#endif

            /* SUBACK sent*/
            expect_value( xi_mock_broker_secondary_layer_push, in_out_state,
                          XI_STATE_OK );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
            will_return( xi_mock_broker_layer_push, CONTROL_CONTINUE );

            /* third message (probably DISCONNECT on a control topic)*/
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
            will_return_count( xi_mock_broker_layer_push, CONTROL_CONTINUE, 2 );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
            expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );

            /* DISCONNECT message arrives at mock broker*/
            expect_value( xi_mock_broker_layer_pull, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_pull, recvd_msg_type,
                          XI_MQTT_TYPE_DISCONNECT );

            expect_value( xi_mock_broker_layer_close, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_close, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_layer_tls_prev_close, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_layer_tls_prev_close_externally, in_out_state,
                          XI_STATE_OK );

            xi_itest_tls_error__act( fixture_void, 0, 1 );

            /* artificially reset test case*/
            xi_itest_tls_error_teardown( fixture_void );
            xi_itest_tls_error_setup( fixture_void );
        }
    }
}

void xi_itest_tls_error__tls_push_PUBLISH_errors__graceful_error_handling(
    void** fixture_void )
{
    uint8_t xi_state_error_code = XI_STATE_OK + 1;
    for ( ; xi_state_error_code < XI_ERROR_COUNT; ++xi_state_error_code )
    {
        /* noteatigyi: this restriction is because mqtt codec layer push function is not
         * robust enough it only handles XI_STATE_FAILED_WRITING error, otherwise it makes
         * an assert */
        if ( xi_state_error_code == XI_STATE_FAILED_WRITING )
        {
            const xi_itest_tls_error__test_fixture_t* const fixture =
                ( xi_itest_tls_error__test_fixture_t* )*fixture_void;

            /* one call for mock broker layer chain init*/
            expect_value( xi_mock_broker_layer_init, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_layer_tls_prev_init, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_layer_tls_prev_connect, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_connect, in_out_state, XI_STATE_OK );

            expect_value( xi_mock_broker_layer_init, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_connect, in_out_state, XI_STATE_OK );

            /* no problem during CONNECT*/
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
            will_return_count( xi_mock_broker_layer_push, CONTROL_CONTINUE, 2 );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
            expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );

            /* CONNECT message arrives at mock broker*/
            expect_value( xi_mock_broker_layer_pull, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_pull, recvd_msg_type,
                          XI_MQTT_TYPE_CONNECT );

            /* CONNACK sent*/
            expect_value( xi_mock_broker_secondary_layer_push, in_out_state,
                          XI_STATE_OK );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
            will_return( xi_mock_broker_layer_push, CONTROL_CONTINUE );

            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
            will_return_count( xi_mock_broker_layer_push, CONTROL_CONTINUE, 2 );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );

            /* SUBSCRIBE message arrives at mock broker*/
            expect_value( xi_mock_broker_layer_pull, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_pull, recvd_msg_type,
                          XI_MQTT_TYPE_SUBSCRIBE );
#ifdef XI_CONTROL_TOPIC_ENABLED
            expect_string( xi_mock_broker_layer_pull, subscribe_topic_name,
                           fixture->control_topic_name );
#else
            expect_any( xi_mock_broker_layer_pull, subscribe_topic_name );
#endif
            expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );

            /* SUBACK sent*/
            expect_value( xi_mock_broker_secondary_layer_push, in_out_state,
                          XI_STATE_OK );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
            will_return( xi_mock_broker_layer_push, CONTROL_CONTINUE );

            /* noteatigyi: this blocks while PUBLISH fails, this is bad I guess blocks
             * means: does not return to client application*/
            size_t loop_count = 0;
            for ( ; loop_count < 100; ++loop_count )
            {
                /* error during PUBLISH*/
                expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
                /* --- ERROR --- HERE --- ERROR --- HERE --- ERROR ---*/
                will_return( xi_mock_broker_layer_push, CONTROL_ERROR );
                will_return( xi_mock_broker_layer_push__ERROR_CHANNEL,
                             xi_state_error_code );
            }

            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
            will_return_count( xi_mock_broker_layer_push, CONTROL_CONTINUE, 2 );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
            expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );

            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
            will_return_count( xi_mock_broker_layer_push, CONTROL_CONTINUE, 2 );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
            expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );

            /* PUBLISH message arrives at mock broker*/
            expect_value( xi_mock_broker_layer_pull, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_pull, recvd_msg_type,
                          XI_MQTT_TYPE_PUBLISH );
#ifdef XI_MANGLE_TOPIC
            expect_string( xi_mock_broker_layer_pull, publish_topic_name,
                           fixture->test_full_topic_name );
#else
            expect_string( xi_mock_broker_layer_pull, publish_topic_name,
                           fixture->test_topic_name );
#endif

            /* sending PUBACK*/
            expect_value( xi_mock_broker_secondary_layer_push, in_out_state,
                          XI_STATE_OK );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
            will_return( xi_mock_broker_layer_push, CONTROL_CONTINUE );

            /* third message (probably DISCONNECT on a control topic)*/
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
            will_return_count( xi_mock_broker_layer_push, CONTROL_CONTINUE, 2 );
            expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
            expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );

            expect_value( xi_mock_broker_layer_close, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_close, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_layer_tls_prev_close, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_layer_tls_prev_close_externally, in_out_state,
                          XI_STATE_OK );

            /* DISCONNECT message arrives at mock broker*/
            expect_value( xi_mock_broker_layer_pull, in_out_state, XI_STATE_OK );
            expect_value( xi_mock_broker_layer_pull, recvd_msg_type,
                          XI_MQTT_TYPE_DISCONNECT );

            xi_itest_tls_error__act( fixture_void, 1, 1 );

            /* artificially reset test case*/
            xi_itest_tls_error_teardown( fixture_void );
            xi_itest_tls_error_setup( fixture_void );
        }
    }
}

void xi_itest_tls_error__tls_init_and_connect_errors__graceful_error_handling(
    void** fixture_void )
{
    XI_UNUSED( fixture_void );

    uint8_t xi_state_error_code = XI_STATE_OK + 1;
    for ( ; xi_state_error_code < XI_ERROR_COUNT; ++xi_state_error_code )
    {
        if ( 0 < xi_test_load_level ||
             /* here probable error codes are collected to reduce runtime for rapid*/
             /* testing*/
             xi_state_error_code == XI_TLS_INITALIZATION_ERROR ||
             xi_state_error_code == XI_TLS_FAILED_LOADING_CERTIFICATE ||
             xi_state_error_code == XI_TLS_CONNECT_ERROR ||
             xi_state_error_code == XI_SOCKET_INITIALIZATION_ERROR ||
             xi_state_error_code == XI_OUT_OF_MEMORY ||
             xi_state_error_code == XI_SOCKET_GETSOCKOPT_ERROR ||
             xi_state_error_code == XI_SOCKET_CONNECTION_ERROR )
        {
            /* one call for mock broker layer chain init*/
            expect_value( xi_mock_broker_layer_init, in_out_state, XI_STATE_OK );
            will_return( xi_mock_broker_layer_init, CONTROL_CONTINUE );
            expect_value( xi_mock_broker_layer_connect, in_out_state, XI_STATE_OK );

            expect_value( xi_mock_broker_layer_init, in_out_state, XI_STATE_OK );
            /* --- ERROR --- HERE --- ERROR --- HERE --- ERROR ---*/
            will_return( xi_mock_broker_layer_init, CONTROL_ERROR );
            will_return( xi_mock_broker_layer_init, xi_state_error_code );

            expect_value( xi_mock_broker_layer_connect, in_out_state,
                          xi_state_error_code );

            expect_value( xi_mock_broker_layer_close, in_out_state, xi_state_error_code );
            expect_value( xi_mock_layer_tls_prev_close, in_out_state,
                          xi_state_error_code );
            expect_value( xi_mock_layer_tls_prev_close_externally, in_out_state,
                          xi_state_error_code );
            expect_value( xi_mock_broker_layer_close, in_out_state, xi_state_error_code );

            xi_itest_tls_error__act( fixture_void, 0, 0 );

            /* artificially reset test case*/
            xi_itest_tls_error_teardown( fixture_void );
            xi_itest_tls_error_setup( fixture_void );
        }
    }
}

void xi_itest_tls_error__connection_flow__basic_checks( void** fixture_void )
{
    const xi_itest_tls_error__test_fixture_t* const fixture =
        ( xi_itest_tls_error__test_fixture_t* )*fixture_void;

    /* one call for mock broker layer chain init*/
    expect_value( xi_mock_broker_layer_init, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_layer_tls_prev_init, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_layer_tls_prev_connect, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_broker_layer_connect, in_out_state, XI_STATE_OK );

    /* expected activity of the libxively's control topci + mqtt logic + mqtt codec layer
     * stack*/
    expect_value( xi_mock_broker_layer_init, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_broker_layer_connect, in_out_state, XI_STATE_OK );

    /* first message (probably CONNECT)*/
    expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
    expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );

    /* CONNECT message arrives at mock broker*/
    expect_value( xi_mock_broker_layer_pull, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_broker_layer_pull, recvd_msg_type, XI_MQTT_TYPE_CONNECT );

    /* CONNACK sent*/
    expect_value( xi_mock_broker_secondary_layer_push, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );

    /* second message (probably SUBSCRIBE on a control topic)*/
    expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
    expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );

    /* SUBSCRIBE message arrives at mock broker*/
    expect_value( xi_mock_broker_layer_pull, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_broker_layer_pull, recvd_msg_type, XI_MQTT_TYPE_SUBSCRIBE );
#ifdef XI_CONTROL_TOPIC_ENABLED
    expect_string( xi_mock_broker_layer_pull, subscribe_topic_name,
                   fixture->control_topic_name );
#else
    expect_any( xi_mock_broker_layer_pull, subscribe_topic_name );
#endif

    /* SUBACK sent*/
    expect_value( xi_mock_broker_secondary_layer_push, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );

    /* PUBLISH*/
    expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
    expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );

    /* PUBLISH PAYLOAD*/
    expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
    expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );

    /* PUBLISH message arrives at mock broker*/
    expect_value( xi_mock_broker_layer_pull, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_broker_layer_pull, recvd_msg_type, XI_MQTT_TYPE_PUBLISH );
#ifdef XI_MANGLE_TOPIC
    expect_string( xi_mock_broker_layer_pull, publish_topic_name,
                   fixture->test_full_topic_name );
#else
    expect_string( xi_mock_broker_layer_pull, publish_topic_name,
                   fixture->test_topic_name );
#endif

    /* sending PUBACK*/
    expect_value( xi_mock_broker_secondary_layer_push, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );

    /* third message (probably DISCONNECT on a control topic)*/
    expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
    expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );

    expect_value( xi_mock_broker_layer_close, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_broker_layer_close, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_layer_tls_prev_close, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_layer_tls_prev_close_externally, in_out_state, XI_STATE_OK );

    /* DISCONNECT message arrives at mock broker*/
    expect_value( xi_mock_broker_layer_pull, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_broker_layer_pull, recvd_msg_type, XI_MQTT_TYPE_DISCONNECT );

    xi_itest_tls_error__act( fixture_void, 1, 1 );
}
