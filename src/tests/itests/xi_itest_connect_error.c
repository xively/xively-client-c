#include "xi_itest_connect_error.h"
#include "xi_backoff_status_api.h"

#include "xi_debug.h"
#include "xi_globals.h"
#include "xi_handle.h"

#include "xi_memory_checks.h"
#include "xi_itest_layerchain_ct_ml_mc.h"
#include "xi_itest_mock_broker_layerchain.h"

/**
 * These test cases
 *
 *
 */

/* Depends on the xi_itest_tls_error.c */
extern xi_context_t* xi_context;
extern xi_context_handle_t xi_context_handle;
extern xi_context_t* xi_context_mockbroker;
/* end of dependency */

/*********************************************************************************
 * test fixture ******************************************************************
 ********************************************************************************/
typedef struct xi_itest_connect_error__test_fixture_s
{
    const char* test_topic_name;
    const char* test_full_topic_name;
    const char* control_topic_name;
} xi_itest_connect_error__test_fixture_t;

xi_itest_connect_error__test_fixture_t* xi_itest_connect_error__generate_fixture()
{
    xi_state_t xi_state = XI_STATE_OK;

    XI_ALLOC( xi_itest_connect_error__test_fixture_t, fixture, xi_state );

    fixture->test_topic_name = ( "test/topic/name" );
    fixture->test_full_topic_name =
        ( "xi/blue/v1/xi_itest_connect_error_account_id/d/"
          "xi_itest_connect_error_device_id/test/topic/name" );
    fixture->control_topic_name = ( "xi/ctrl/v1/xi_itest_connect_error_device_id/clt" );

    return fixture;

err_handling:
    fail();

    return NULL;
}

int xi_itest_connect_error_setup( void** fixture_void )
{
    /* clear the external dependencies */
    xi_context            = NULL;
    xi_context_handle     = XI_INVALID_CONTEXT_HANDLE;
    xi_context_mockbroker = NULL;

    xi_memory_limiter_tearup();

    *fixture_void = xi_itest_connect_error__generate_fixture();

    xi_globals.backoff_status.backoff_lut_i = 0;
    xi_cancel_backoff_event();

    xi_initialize( "xi_itest_connect_error_account_id",
                   "xi_itest_connect_error_device_id", "./libxively_consumer.creds" );

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

int xi_itest_connect_error_teardown( void** fixture_void )
{
    xi_delete_context_with_custom_layers(
        &xi_context, itest_ct_ml_mc_layer_chain,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_CT_ML_MC ) );

    xi_delete_context_with_custom_layers(
        &xi_context_mockbroker, itest_mock_broker_codec_layer_chain,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_MOCK_BROKER_CODEC ) );

    xi_shutdown();

    xi_itest_connect_error__test_fixture_t* fixture =
        ( xi_itest_connect_error__test_fixture_t* )*fixture_void;

    XI_SAFE_FREE( fixture );

    xi_memory_limiter_teardown();

    return 0;
}

void connect_error_on_connection_state_changed( xi_context_handle_t in_context_handle,
                                                void* data,
                                                xi_state_t state )
{
    XI_UNUSED( in_context_handle );
    XI_UNUSED( data );
    XI_UNUSED( state );
}

static void xi_itest_connect_error__call_connect( void** fixture_void )
{
    XI_UNUSED( fixture_void );

    const uint16_t connection_timeout = 20;
    const uint16_t keepalive_timeout  = 20;

    const xi_state_t local_state = xi_connect(
        xi_context_handle, "itest_username", "itest_password", connection_timeout,
        keepalive_timeout, XI_SESSION_CLEAN, &connect_error_on_connection_state_changed );

    check_expected( local_state );
}

static void
xi_itest_connect_error__trigger_connect( void** fixture_void, uint8_t init_mock_broker )
{
    if ( 1 == init_mock_broker )
    {
        XI_PROCESS_INIT_ON_THIS_LAYER(
            &xi_context_mockbroker->layer_chain.top->layer_connection, NULL,
            XI_STATE_OK );

        xi_evtd_step( xi_globals.evtd_instance, xi_getcurrenttime_seconds() );
    }

    /* here we expect to connect succesfully */
    expect_value( xi_itest_connect_error__call_connect, local_state, XI_STATE_OK );
    xi_itest_connect_error__call_connect( fixture_void );
}

static void
xi_itest_connect_error__trigger_event_dispatcher( void** fixture_void,
                                                  uint8_t max_evtd_iterations )
{
    XI_UNUSED( fixture_void );

    uint8_t loop_counter = 0;
    while ( xi_evtd_dispatcher_continue( xi_globals.evtd_instance ) == 1 &&
            loop_counter < max_evtd_iterations )
    {
        xi_evtd_step( xi_globals.evtd_instance,
                      xi_getcurrenttime_seconds() + loop_counter );
        ++loop_counter;
    }
}

static void xi_itest_connect_error__trigger_shutdown( void** fixture_void )
{
    XI_UNUSED( fixture_void );

    const xi_state_t local_state = xi_shutdown_connection( xi_context_handle );
    check_expected( local_state );
}

void xi_itest_test_valid_flow__call_connect_function_twice_in_a_row__second_call_returns_error(
    void** fixture_void )
{
    expect_value( xi_itest_connect_error__call_connect, local_state, XI_STATE_OK );
    xi_itest_connect_error__call_connect( fixture_void );
    expect_value( xi_itest_connect_error__call_connect, local_state,
                  XI_ALREADY_INITIALIZED );
    xi_itest_connect_error__call_connect( fixture_void );
}

void xi_itest_test_valid_flow__call_connect_function_twice_with_a_single_evtd_call_in_the_middle__second_call_returns_error(
    void** fixture_void )
{
    uint8_t evtd_loop_count_between_connect_calls = 0;
    for ( ; evtd_loop_count_between_connect_calls < 10;
          ++evtd_loop_count_between_connect_calls )
    {
        const xi_itest_connect_error__test_fixture_t* const fixture =
            ( xi_itest_connect_error__test_fixture_t* )*fixture_void;

        xi_debug_format( "Number of evtd calls: %d", evtd_loop_count_between_connect_calls );

        /* one call for mock broker layer chain init*/
        expect_value( xi_mock_broker_layer_init, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_layer_tls_prev_init, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_layer_tls_prev_connect, in_out_state, XI_STATE_OK );

        expect_value( xi_mock_broker_layer_init, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_broker_layer_connect, in_out_state, XI_STATE_OK );

        /* no problem during CONNECT*/
        expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
        expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );

        /* CONNECT message arrives at mock broker*/
        expect_value( xi_mock_broker_layer_pull, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_broker_layer_pull, recvd_msg_type, XI_MQTT_TYPE_CONNECT );

        /* CONNACK sent*/
        expect_value( xi_mock_broker_secondary_layer_push, in_out_state, XI_STATE_OK );
        will_return( xi_mock_broker_secondary_layer_push, CONTROL_CONTINUE );
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
        will_return( xi_mock_broker_secondary_layer_push, CONTROL_CONTINUE );
        expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );

        /* DISCONNECT MESSAGE */
        expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
        expect_value( xi_mock_broker_layer_pull, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_broker_layer_pull, recvd_msg_type,
                      XI_MQTT_TYPE_DISCONNECT );

        /* SHUTDOWN */
        expect_value( xi_mock_broker_layer_close, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_broker_layer_close, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_layer_tls_prev_close, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_layer_tls_prev_close_externally, in_out_state,
                      XI_STATE_OK );

        xi_itest_connect_error__trigger_connect( fixture_void, 1 );
        xi_itest_connect_error__trigger_event_dispatcher(
            fixture_void, evtd_loop_count_between_connect_calls );

        /* TRY TO CONNECT ONE MORE TIME, IT SHOULD FAIL */
        expect_value( xi_itest_connect_error__call_connect, local_state,
                      XI_ALREADY_INITIALIZED );
        xi_itest_connect_error__call_connect( fixture_void );
        xi_itest_connect_error__trigger_event_dispatcher( fixture_void, 6 );

        expect_value( xi_itest_connect_error__trigger_shutdown, local_state,
                      XI_STATE_OK );
        xi_itest_connect_error__trigger_shutdown( fixture_void );
        xi_itest_connect_error__trigger_event_dispatcher( fixture_void, 10 );

        /* artificially reset test case*/
        xi_itest_connect_error_teardown( fixture_void );
        xi_itest_connect_error_setup( fixture_void );
    }
}

void xi_itest_test_valid_flow__call_disconnect_twice_on_connected_context__second_call_should_return_error(
    void** fixture_void )
{
    uint8_t evtd_loop_count_between_connect_calls = 0;
    for ( ; evtd_loop_count_between_connect_calls < 10;
          ++evtd_loop_count_between_connect_calls )
    {

        const xi_itest_connect_error__test_fixture_t* const fixture =
            ( xi_itest_connect_error__test_fixture_t* )*fixture_void;

        /* one call for mock broker layer chain init*/
        expect_value( xi_mock_broker_layer_init, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_layer_tls_prev_init, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_layer_tls_prev_connect, in_out_state, XI_STATE_OK );

        expect_value( xi_mock_broker_layer_init, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_broker_layer_connect, in_out_state, XI_STATE_OK );

        /* no problem during CONNECT*/
        expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
        expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );

        /* CONNECT message arrives at mock broker*/
        expect_value( xi_mock_broker_layer_pull, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_broker_layer_pull, recvd_msg_type, XI_MQTT_TYPE_CONNECT );

        /* CONNACK sent*/
        expect_value( xi_mock_broker_secondary_layer_push, in_out_state, XI_STATE_OK );
        will_return( xi_mock_broker_secondary_layer_push, CONTROL_CONTINUE );
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
        will_return( xi_mock_broker_secondary_layer_push, CONTROL_CONTINUE );
        expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );

        /* DISCONNECT MESSAGE */
        expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_broker_layer_push, in_out_state, XI_STATE_WRITTEN );
        expect_value( xi_mock_broker_layer_pull, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_broker_layer_pull, recvd_msg_type, XI_MQTT_TYPE_DISCONNECT );

        /* SHUTDOWN */
        expect_value( xi_mock_broker_layer_close, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_broker_layer_close, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_layer_tls_prev_close, in_out_state, XI_STATE_OK );
        expect_value( xi_mock_layer_tls_prev_close_externally, in_out_state, XI_STATE_OK );

        xi_itest_connect_error__trigger_connect( fixture_void, 1 );
        xi_itest_connect_error__trigger_event_dispatcher( fixture_void, 6 );

        /* TRY TO CALL SHUTDOWN TWICE IN A ROW */
        expect_value( xi_itest_connect_error__trigger_shutdown, local_state, XI_STATE_OK );
        xi_itest_connect_error__trigger_shutdown( fixture_void );

        /* HERE GOES THE DELAY */
        xi_itest_connect_error__trigger_event_dispatcher(
            fixture_void, evtd_loop_count_between_connect_calls );

        expect_value( xi_itest_connect_error__trigger_shutdown, local_state, XI_STATE_OK );
        xi_itest_connect_error__trigger_shutdown( fixture_void );

        xi_itest_connect_error__trigger_event_dispatcher( fixture_void, 6 );

        /* artificially reset test case*/
        xi_itest_connect_error_teardown( fixture_void );
        xi_itest_connect_error_setup( fixture_void );
    }
}

void xi_itest_test_valid_flow__call_connect_function_then_disconnect_without_making_a_connection__shutdown_should_unregister_connect(
    void** fixture_void )
{
    /* call initialisation and connect without dispatching any event */
    xi_itest_connect_error__trigger_connect( fixture_void, 0 );

    /* now we trigger shutdown immedietaly */
    expect_value( xi_itest_connect_error__trigger_shutdown, local_state, XI_STATE_OK );
    xi_itest_connect_error__trigger_shutdown( fixture_void );
    xi_itest_connect_error__trigger_event_dispatcher( fixture_void, 5 );
}
