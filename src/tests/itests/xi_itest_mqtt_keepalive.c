/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xively.h>
#include <xi_macros.h>
#include "xi_globals.h"
#include "xi_itest_layerchain_ct_ml_mc.h"
#include <xi_itest_mock_broker_layerchain.h>
#include "xi_memory_checks.h"
#include "xi_itest_helpers.h"
#include "xi_handle.h"
#include "xi_backoff_status_api.h"

/* Depends on the xi_itest_tls_error.c */
extern xi_context_t* xi_context;
extern xi_context_handle_t xi_context_handle;
extern xi_context_t* xi_context_mockbroker;
/* end of dependency */


int xi_itest_mqtt_keepalive_setup( void** fixture_void )
{
    XI_UNUSED( fixture_void );

    xi_memory_limiter_tearup();

    xi_globals.backoff_status.backoff_lut_i = 0;
    xi_cancel_backoff_event();

    xi_initialize( "xi_itest_mqtt_keepalive_account_id", "xi_itest_mqtt_keepalive_device_id" );

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

int xi_itest_mqtt_keepalive_teardown( void** fixture_void )
{
    XI_UNUSED( fixture_void );

    xi_delete_context( xi_context_handle );
    xi_delete_context_with_custom_layers(
        &xi_context_mockbroker, itest_mock_broker_codec_layer_chain,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_MOCK_BROKER_CODEC ) );

    xi_shutdown();

    return !xi_memory_limiter_teardown();
}

void _xi_itest_mqtt_keepalive__on_connection_state_changed( xi_context_handle_t in_context_handle,
                                                            void* data,
                                                            xi_state_t state )
{
    XI_UNUSED( in_context_handle );
    XI_UNUSED( data );
    XI_UNUSED( state );

    check_expected( state );
}

/*********************************************************************************
 * act ***************************************************************************
 ********************************************************************************/
static void xi_itest_mqtt_keepalive__act( void** fixture_void,
                                          const uint16_t loop_id_reset_by_peer )
{
    XI_UNUSED( fixture_void );
    {
        /* turn off LAYER and MQTT LEVEL expectation checks to concentrate only on SFT
         * protocol messages */
        will_return_always( xi_mock_broker_layer__check_expected__LAYER_LEVEL,
                            CONTROL_SKIP_CHECK_EXPECTED );

        will_return_always( xi_mock_broker_layer__check_expected__MQTT_LEVEL,
                            CONTROL_SKIP_CHECK_EXPECTED );

        will_return_always( xi_mock_layer_tls_prev__check_expected__LAYER_LEVEL,
                            CONTROL_SKIP_CHECK_EXPECTED );
    }

    XI_PROCESS_INIT_ON_THIS_LAYER(
        &xi_context_mockbroker->layer_chain.top->layer_connection, NULL, XI_STATE_OK );

    xi_evtd_step( xi_globals.evtd_instance, xi_bsp_time_getcurrenttime_seconds() );

    const uint16_t connection_timeout = 20;
    const uint16_t keepalive_timeout  = 5;
    const uint16_t loop_counter_max = 23;
    const uint16_t loop_counter_disconnect = 18;

    xi_connect( xi_context_handle, "itest_username", "itest_password", connection_timeout,
                keepalive_timeout, XI_SESSION_CLEAN,
                &_xi_itest_mqtt_keepalive__on_connection_state_changed );

    uint16_t loop_counter = 1;
    while ( xi_evtd_dispatcher_continue( xi_globals.evtd_instance ) == 1 &&
            loop_counter < loop_counter_max )
    {
        //printf( "loop_counter = %d\n", loop_counter );

        xi_evtd_step( xi_globals.evtd_instance,
                      xi_bsp_time_getcurrenttime_seconds() + loop_counter );
        ++loop_counter;

        if ( loop_id_reset_by_peer == loop_counter )
        {
            XI_PROCESS_CLOSE_ON_THIS_LAYER(
                &xi_context->layer_chain.bottom->layer_connection, NULL,
                XI_CONNECTION_RESET_BY_PEER_ERROR );
        }

        if ( loop_counter_disconnect == loop_counter )
        {
            xi_shutdown_connection( xi_context_handle );
        }
    }
}


void xi_itest_mqtt_keepalive__PINGREQ_failed_to_send__client_disconnects_after_keepalive_seconds( void** state )
{
    /* let CONNECT and SUBSCRIBE writes succeed */
#ifdef XI_CONTROL_TOPIC_ENABLED
    will_return_count( xi_mock_layer_tls_prev_push, CONTROL_TLS_PREV_CONTINUE, 2 );
#else
    will_return_count( xi_mock_layer_tls_prev_push, CONTROL_TLS_PREV_CONTINUE, 1 );
#endif

    /* make first PINGREQ fail to write on the socket */
    will_return( xi_mock_layer_tls_prev_push, CONTROL_TLS_PREV_PUSH__WRITE_ERROR );
    /* set the specific error to be returned by the mock IO layer (here tls_prev) */
    will_return( xi_mock_layer_tls_prev_push, XI_STATE_FAILED_WRITING );

    /* make mock broker not to reply on PINGREQ, this is necessary since the
     * mock broker catches the message before IO layer */
    will_return( xi_mock_broker_layer_pull, CONTROL_PULL_PINGREQ_SUPPRESS_RESPONSE );

#ifndef XI_MODULE_THREAD_ENABLED
    expect_value( _xi_itest_mqtt_keepalive__on_connection_state_changed, state, XI_STATE_OK );
    expect_value( _xi_itest_mqtt_keepalive__on_connection_state_changed, state, XI_STATE_TIMEOUT );
#endif

    xi_itest_mqtt_keepalive__act( state, 0 );
}

void xi_itest_mqtt_keepalive__PINGREQ_failed_to_send__broker_disconnects_first( void** state )
{
    /* let CONNECT and SUBSCRIBE writes succeed */
#ifdef XI_CONTROL_TOPIC_ENABLED
    will_return_count( xi_mock_layer_tls_prev_push, CONTROL_TLS_PREV_CONTINUE, 2 );
#else
    will_return_count( xi_mock_layer_tls_prev_push, CONTROL_TLS_PREV_CONTINUE, 1 );
#endif

    /* make first PINGREQ fail to write on the socket */
    will_return( xi_mock_layer_tls_prev_push, CONTROL_TLS_PREV_PUSH__WRITE_ERROR );
    /* set the specific error to be returned by the mock IO layer (here tls_prev) */
    will_return( xi_mock_layer_tls_prev_push, XI_STATE_FAILED_WRITING );

    /* make mock broker not to reply on PINGREQ, this is necessary since the
     * mock broker catches the message before IO layer */
    will_return( xi_mock_broker_layer_pull, CONTROL_PULL_PINGREQ_SUPPRESS_RESPONSE );

#ifndef XI_MODULE_THREAD_ENABLED
    expect_value( _xi_itest_mqtt_keepalive__on_connection_state_changed, state, XI_STATE_OK );
    expect_value( _xi_itest_mqtt_keepalive__on_connection_state_changed, state,
                  XI_CONNECTION_RESET_BY_PEER_ERROR );
    expect_value( _xi_itest_mqtt_keepalive__on_connection_state_changed, state, XI_STATE_TIMEOUT );
#endif

    xi_itest_mqtt_keepalive__act( state, 11 );
}

void xi_itest_mqtt_keepalive__2nd_PINGREQ_failed_to_send__broker_disconnects_first( void** state )
{
    /* let CONNECT and SUBSCRIBE writes succeed */
#ifdef XI_CONTROL_TOPIC_ENABLED
    will_return_count( xi_mock_layer_tls_prev_push, CONTROL_TLS_PREV_CONTINUE, 3 );
#else
    will_return_count( xi_mock_layer_tls_prev_push, CONTROL_TLS_PREV_CONTINUE, 2 );
#endif

    /* make first PINGREQ fail to write on the socket */
    will_return( xi_mock_layer_tls_prev_push, CONTROL_TLS_PREV_PUSH__WRITE_ERROR );
    /* set the specific error to be returned by the mock IO layer (here tls_prev) */
    will_return( xi_mock_layer_tls_prev_push, XI_STATE_FAILED_WRITING );

    /* mock broker responds with proper PINGRESP on the first PINGREQ */
    will_return( xi_mock_broker_layer_pull, CONTROL_CONTINUE );
    /* make mock broker not to reply on PINGREQ, this is necessary since the
     * mock broker catches the message before IO layer */
    will_return( xi_mock_broker_layer_pull, CONTROL_PULL_PINGREQ_SUPPRESS_RESPONSE );

#ifndef XI_MODULE_THREAD_ENABLED
    expect_value( _xi_itest_mqtt_keepalive__on_connection_state_changed, state, XI_STATE_OK );
    expect_value( _xi_itest_mqtt_keepalive__on_connection_state_changed, state,
                  XI_CONNECTION_RESET_BY_PEER_ERROR );
    expect_value( _xi_itest_mqtt_keepalive__on_connection_state_changed, state, XI_STATE_TIMEOUT );
#endif

    xi_itest_mqtt_keepalive__act( state, 18 );
}
