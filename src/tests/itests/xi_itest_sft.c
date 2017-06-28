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

#include "xi_control_message.h"

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
    const char* control_topic_name_client_in;
    const char* control_topic_name_client_out;

    xi_mock_broker_data_t* broker_data;

    uint8_t loop_id__manual_disconnect;

    uint8_t max_loop_count;

} xi_itest_sft__test_fixture_t;


xi_itest_sft__test_fixture_t* xi_itest_sft__generate_fixture()
{
    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_itest_sft__test_fixture_t, fixture, state );

    fixture->control_topic_name_client_in  = ( "xi/ctrl/v1/xi_itest_sft_device_id/cln" );
    fixture->control_topic_name_client_out = ( "xi/ctrl/v1/xi_itest_sft_device_id/svc" );

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
static void xi_itest_sft__act( void** fixture_void,
                               char do_disconnect_flag,
                               const char* updateable_filenames[],
                               uint16_t updateable_file_count )
{
    xi_state_t state = XI_STATE_OK;

    const xi_itest_sft__test_fixture_t* const fixture =
        ( xi_itest_sft__test_fixture_t* )*fixture_void;

    XI_ALLOC( xi_mock_broker_data_t, broker_data, state );
    broker_data->control_topic_name_broker_in  = fixture->control_topic_name_client_out;
    broker_data->control_topic_name_broker_out = fixture->control_topic_name_client_in;

    XI_PROCESS_INIT_ON_THIS_LAYER(
        &xi_context_mockbroker->layer_chain.top->layer_connection, broker_data,
        XI_STATE_OK );

    xi_evtd_step( xi_globals.evtd_instance, xi_bsp_time_getcurrenttime_seconds() );

    xi_set_updateable_files( xi_context_handle, updateable_filenames,
                             updateable_file_count );

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

err_handling:;
}

/*********************************************************************************
 * test cases ********************************************************************
 ********************************************************************************/
void xi_itest_sft__basic_flow__SFT_protocol_intact( void** fixture_void )
{
    const xi_itest_sft__test_fixture_t* const fixture =
        ( xi_itest_sft__test_fixture_t* )*fixture_void;

    will_return_always( xi_mock_broker_layer__check_expected__LEVEL0,
                        CONTROL_SKIP_CHECK_EXPECTED );

    will_return_always( xi_mock_layer_tls_prev__check_expected__LEVEL0,
                        CONTROL_SKIP_CHECK_EXPECTED );

    /* MQTT connect */
    expect_value( xi_mock_broker_layer_pull, recvd_msg_type, XI_MQTT_TYPE_CONNECT );

    /* control topic subscription */
    expect_value( xi_mock_broker_layer_pull, recvd_msg_type, XI_MQTT_TYPE_SUBSCRIBE );
    expect_string( xi_mock_broker_layer_pull, subscribe_topic_name,
                   fixture->control_topic_name_client_in );

    /* SFT FILE_INFO */
    expect_value( xi_mock_broker_layer_pull, recvd_msg_type, XI_MQTT_TYPE_PUBLISH );
    expect_string( xi_mock_broker_layer_pull, publish_topic_name,
                   fixture->control_topic_name_client_out );

    expect_value( xi_mock_broker_layer_pull, recvd_msg_type, XI_MQTT_TYPE_DISCONNECT );

    expect_value( xi_mock_broker_sft_logic_on_message, control_message->common.msgtype,
                  XI_CONTROL_MESSAGE_CS_FILE_INFO );

    // ACT
    xi_itest_sft__act( fixture_void, 1, ( const char* [] ){"file1", "file2"}, 2 );
}
