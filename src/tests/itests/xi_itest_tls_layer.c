/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_globals.h"
#include "xi_itest_helpers.h"
#include "xi_itest_layerchain_tls.h"
#include "xi_itest_tls_layer.h"
#include "xi_memory_checks.h"

#include <time.h>

xi_context_t* xi_context__itest_tls_layer = NULL;

/**
 * xi_itest_tls_layer test suit description
 *
 * System Under Test: 1 layer: TLS layer
 *
 * Test consists of an artificial layer chain: TLSNEXT - TLS - TLSPREV
 * TLSNEXT and TLSPREV are mock layers. They drive and monitor the SUT: TLS layer.
 */

/*********************************************************************************
 * test fixture ******************************************************************
 ********************************************************************************/
typedef struct xi_itest_tls_layer__test_fixture_s
{
    uint8_t loop_id__manual_close;
    uint8_t max_loop_count;
} xi_itest_tls_layer__test_fixture_t;

xi_itest_tls_layer__test_fixture_t* xi_itest_tls_layer__generate_fixture()
{
    xi_state_t xi_state = XI_STATE_OK;

    XI_ALLOC( xi_itest_tls_layer__test_fixture_t, fixture, xi_state );

    fixture->loop_id__manual_close = 15;
    fixture->max_loop_count        = 20;

    return fixture;

err_handling:
    fail();

    return NULL;
}


int xi_itest_tls_layer_setup( void** fixture_void )
{
    xi_memory_limiter_tearup();

    *fixture_void = xi_itest_tls_layer__generate_fixture();

    assert_int_equal( XI_STATE_OK, xi_initialize( "xi_itest_tls_error_account_id",
                                                  "xi_itest_tls_error_device_id" ) );

    XI_CHECK_STATE( xi_create_context_with_custom_layers(
        &xi_context__itest_tls_layer, itest_layer_chain_tls, XI_LAYER_CHAIN_TLS,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_TLS ) ) );

    return 0;

err_handling:
    fail();

    return 1;
}

int xi_itest_tls_layer_teardown( void** fixture_void )
{
    xi_itest_tls_layer__test_fixture_t* fixture =
        ( xi_itest_tls_layer__test_fixture_t* )*fixture_void;

    XI_SAFE_FREE( fixture );

    xi_delete_context_with_custom_layers(
        &xi_context__itest_tls_layer, itest_layer_chain_tls,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_TLS ) );

    xi_shutdown();

    return !xi_memory_limiter_teardown();
}

/**
 * @name    xi_itest_tls_layer__act
 * @brief   mimics a libxively live environment: drives event dispatcher
 *
 *
 * Does a limited number of iterations, processes some events. Acts
 * like a live libxively environment.
 *
 * @param [in] init_connection_data_flag    if not set an uninitialized
 *                                          libxively behavior can be tested
 */
void xi_itest_tls_layer__act( void** fixture_void,
                              char init_connection_data_flag,
                              char close_layer_chain_flag )
{
    xi_itest_tls_layer__test_fixture_t* fixture =
        ( xi_itest_tls_layer__test_fixture_t* )*fixture_void;

    if ( init_connection_data_flag != 0 )
    {
        xi_context__itest_tls_layer->context_data.connection_data =
            xi_alloc_connection_data(
                "target.broker.com", 8883, "tls_layer_itest_username",
                "tls_layer_itest_password", 10, 10, XI_SESSION_CLEAN );
    }

    xi_layer_t* input_layer  = xi_context__itest_tls_layer->layer_chain.top;
    xi_layer_t* output_layer = xi_context__itest_tls_layer->layer_chain.bottom;

    XI_PROCESS_INIT_ON_THIS_LAYER(
        &input_layer->layer_connection,
        xi_context__itest_tls_layer->context_data.connection_data, XI_STATE_OK );

    size_t loop_counter = 0;
    while ( 1 == xi_evtd_dispatcher_continue( xi_globals.evtd_instance ) &&
            loop_counter < fixture->max_loop_count )
    {
        xi_evtd_step( xi_globals.evtd_instance, time( NULL ) + loop_counter );
        xi_evtd_update_file_fd_events( xi_globals.evtd_instance );
        ++loop_counter;

        if ( close_layer_chain_flag && loop_counter == fixture->loop_id__manual_close )
        {
            XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER( &output_layer->layer_connection,
                                                       NULL, XI_STATE_OK );
        }
    }
}

extern int8_t xi_cm_strict_mock;

/*********************************************************************************************
 * test cases
 *********************************************************************************
 *********************************************************************************************/
void xi_itest_tls_layer__bad_handshake_response__graceful_closure( void** fixture_void )
{
    xi_cm_strict_mock = 0;

    // init
    expect_value( xi_mock_layer_tls_next_init, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_layer_tls_prev_init, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_layer_tls_prev_connect, in_out_state, XI_STATE_OK );
    // handshake started by TLS layer
    expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );

    // INTRODUCE ERROR HERE: drive mock IO layer to return unrecognizable
    // message to TLS layer as a handshake reply
    will_return( xi_mock_layer_tls_prev_push, CONTROL_TLS_PREV_PUSH__RETURN_MESSAGE );
    will_return( xi_mock_layer_tls_prev_push, "this is litter for TLS" );

    expect_value( xi_mock_layer_tls_prev_pull, in_out_state, XI_STATE_OK );

#ifdef XI_TLS_LIB_MBEDTLS
    /* here a possible mbedTLS bug is handled. mbedTLS ends up in an infinite
       alert sending loop. Here a manual exit is implemented to make this test
       case valuable for mbedTSL as well. */
    /* expecting the alert notification */
    expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );
    /* ask the prev layer to close */
    will_return( xi_mock_layer_tls_prev_push, CONTROL_TLS_PREV_CLOSE );
    /* setting the close return value to be same as wolfssl's */
    will_return( xi_mock_layer_tls_prev_push, XI_TLS_CONNECT_ERROR );
#endif

    // expect error propagation through close functions too
    expect_value( xi_mock_layer_tls_prev_close, in_out_state, XI_TLS_CONNECT_ERROR );
    expect_value( xi_mock_layer_tls_prev_close_externally, in_out_state,
                  XI_TLS_CONNECT_ERROR );
    expect_value( xi_mock_layer_tls_next_connect, in_out_state, XI_TLS_CONNECT_ERROR );

    // no close_externally calls on layers above since they were not connected

    xi_itest_tls_layer__act( fixture_void, 1, 0 );
}

void xi_itest_tls_layer__null_host__graceful_closure( void** fixture_void )
{
    expect_value( xi_mock_layer_tls_next_init, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_layer_tls_next_connect, in_out_state,
                  XI_TLS_INITALIZATION_ERROR );
    expect_value( xi_mock_layer_tls_prev_close_externally, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_layer_tls_next_close_externally, in_out_state, XI_STATE_OK );

    xi_itest_tls_layer__act( fixture_void, 0, 1 );
}

void xi_itest_tls_layer__valid_dependencies__successful_init( void** fixture_void )
{
    // init
    expect_value( xi_mock_layer_tls_next_init, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_layer_tls_prev_init, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_layer_tls_prev_connect, in_out_state, XI_STATE_OK );
    // handshake started by TLS layer
    expect_value( xi_mock_layer_tls_prev_push, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_layer_tls_prev_close_externally, in_out_state, XI_STATE_OK );
    expect_value( xi_mock_layer_tls_next_close_externally, in_out_state, XI_STATE_OK );

    xi_itest_tls_layer__act( fixture_void, 1, 1 );
}
