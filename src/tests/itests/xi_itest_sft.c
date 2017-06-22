/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_itest_sft.h>
#include "xi_itest_helpers.h"
#include "xi_backoff_status_api.h"

#include "xi_debug.h"
#include "xi_globals.h"
#include "xi_handle.h"

#include "xi_memory_checks.h"
#include "xi_itest_layerchain_ct_ml_mc.h"
#include "xi_itest_mock_broker_layerchain.h"

/*
 * xi_itest_layerchain_ct_ml_mc.h
 *  |
 *  L->   CT - ML - MC - MB - TLSPREV
 *                       |
 *                       MC
 *                       |
 *                       MBSecondary
 *
 *                       A
 *                       |
 *                       |
 *                  xi_itest_mock_broker_layerchain.h
 */


/* Depends on the xi_itest_tls_error.c */
extern xi_context_t* xi_context;
extern xi_context_handle_t xi_context_handle;
extern xi_context_t* xi_context_mockbroker;
/* end of dependency */

/*********************************************************************************
 * test fixture ******************************************************************
 ********************************************************************************/
typedef struct xi_itest_sft__test_fixture_s
{
    const char* control_topic_name;

    uint8_t loop_id__manual_disconnect;

    uint8_t max_loop_count;

} xi_itest_sft__test_fixture_t;


xi_itest_sft__test_fixture_t* xi_itest_sft__generate_fixture()
{
    xi_state_t xi_state = XI_STATE_OK;

    XI_ALLOC( xi_itest_sft__test_fixture_t, fixture, xi_state );

    fixture->control_topic_name = ( "xi/ctrl/v1/xi_itest_sft_device_id/cln" );

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
int xi_itest_sft_setup( void** fixture_void )
{
    /* clear the external dependencies */
    xi_context            = NULL;
    xi_context_handle     = XI_INVALID_CONTEXT_HANDLE;
    xi_context_mockbroker = NULL;

    xi_memory_limiter_tearup();

    *fixture_void = xi_itest_sft__generate_fixture();

    xi_globals.backoff_status.backoff_lut_i = 0;
    xi_cancel_backoff_event();

    xi_initialize( "xi_itest_sft_account_id", "xi_itest_sft_device_id" );

    // xi_initialize_add_updateable_file( ( const char* [] ){"file1", "file2", "file3"}, 3
    // );
    // xi_initialize_add_updateable_file( ( const char* [] ){"file1"}, 1 );

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

int xi_itest_sft_teardown( void** fixture_void )
{
    xi_delete_context_with_custom_layers(
        &xi_context, itest_ct_ml_mc_layer_chain,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_CT_ML_MC ) );

    xi_delete_context_with_custom_layers(
        &xi_context_mockbroker, itest_mock_broker_codec_layer_chain,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_MOCK_BROKER_CODEC ) );

    xi_shutdown();

    XI_UNUSED( fixture_void );
    xi_itest_sft__test_fixture_t* fixture =
        ( xi_itest_sft__test_fixture_t* )*fixture_void;

    XI_SAFE_FREE( fixture );

    return !xi_memory_limiter_teardown();
}

void xi_itest_sft__on_connection_state_changed( xi_context_handle_t in_context_handle,
                                                void* data,
                                                xi_state_t state )
{
    XI_UNUSED( in_context_handle );
    XI_UNUSED( data );
    XI_UNUSED( state );
}

/*********************************************************************************
 * act ***************************************************************************
 ********************************************************************************/
static void xi_itest_sft__act( void** fixture_void, char do_disconnect_flag )
{
    XI_UNUSED( fixture_void );
    XI_UNUSED( do_disconnect_flag );

    XI_PROCESS_INIT_ON_THIS_LAYER(
        &xi_context_mockbroker->layer_chain.top->layer_connection, NULL, XI_STATE_OK );

    xi_evtd_step( xi_globals.evtd_instance, xi_bsp_time_getcurrenttime_seconds() );

    const xi_itest_sft__test_fixture_t* const fixture =
        ( xi_itest_sft__test_fixture_t* )*fixture_void;

    const uint16_t connection_timeout = 20;
    const uint16_t keepalive_timeout  = fixture->max_loop_count;
    xi_connect( xi_context_handle, "itest_username", "itest_password", connection_timeout,
                keepalive_timeout, XI_SESSION_CLEAN,
                &xi_itest_sft__on_connection_state_changed );

    uint8_t loop_counter = 0;
    while ( xi_evtd_dispatcher_continue( xi_globals.evtd_instance ) == 1 &&
            loop_counter < keepalive_timeout )
    {
        xi_evtd_step( xi_globals.evtd_instance,
                      xi_bsp_time_getcurrenttime_seconds() + loop_counter );
        ++loop_counter;

        if ( do_disconnect_flag && loop_counter == fixture->loop_id__manual_disconnect )
        {
            xi_shutdown_connection( xi_context_handle );
        }
    }
}


void xi_itest_sft__basic_flow__SFT_protocol_intact( void** fixture_void )
{
    const xi_itest_sft__test_fixture_t* const fixture =
        ( xi_itest_sft__test_fixture_t* )*fixture_void;

    will_return_always( xi_mock_broker_layer__check_expected__LEVEL0,
                        CONTROL_SKIP_CHECK_EXPECTED );

    will_return_always( xi_mock_layer_tls_prev__check_expected__LEVEL0,
                        CONTROL_SKIP_CHECK_EXPECTED );

    expect_value( xi_mock_broker_layer_pull, recvd_msg_type, XI_MQTT_TYPE_CONNECT );
    expect_value( xi_mock_broker_layer_pull, recvd_msg_type, XI_MQTT_TYPE_SUBSCRIBE );
    expect_string( xi_mock_broker_layer_pull, subscribe_topic_name,
                   fixture->control_topic_name );
    expect_value( xi_mock_broker_layer_pull, recvd_msg_type, XI_MQTT_TYPE_DISCONNECT );

    // ACT
    xi_itest_sft__act( fixture_void, 1 );
}
