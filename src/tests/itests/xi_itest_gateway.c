/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_itest_gateway.h>
#include "xi_itest_helpers.h"

#include "xi_memory_checks.h"
#include "xi_globals.h"
#include "xi_backoff_status_api.h"
#include "xi_itest_layerchain_ct_ml_mc.h"
#include "xi_itest_mock_broker_layerchain.h"
#include "xi_handle.h"
#include <xi_context.h>
#include <xively_gateway.h>

extern xi_context_t* xi_context;
extern xi_context_t* xi_context_mockbroker;

/*********************************************************************************
 * test fixture ******************************************************************
 ********************************************************************************/
typedef struct xi_itest_gateway__test_fixture_s
{
    uint16_t loop_id__manual_connect_ed;
    uint16_t loop_id__manual_disconnect_ed;
    uint16_t loop_id__manual_disconnect;
    uint16_t loop_id__manual_disconnect_mockbroker;
    uint16_t max_loop_count;

    xi_context_t* xi_context_sut;
    xi_context_handle_t xi_context_handle;

    xi_context_t* xi_context_mockbroker;
    xi_context_t* xi_context_mockbroker_edge_device_broker;
} xi_itest_gateway__test_fixture_t;


static xi_itest_gateway__test_fixture_t* _xi_itest_gateway__generate_fixture()
{
    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_itest_gateway__test_fixture_t, fixture, state );


    fixture->loop_id__manual_connect_ed            = 10;
    fixture->loop_id__manual_disconnect_ed         = 50;
    fixture->loop_id__manual_disconnect            = 60;
    fixture->loop_id__manual_disconnect_mockbroker = 70;
    fixture->max_loop_count                        = 80;

    return fixture;

err_handling:
    fail();

    return NULL;
}

/*********************************************************************************
 * setup / teardown **************************************************************
 ********************************************************************************/
int xi_itest_gateway_setup( void** fixture_void )
{
    xi_memory_limiter_tearup();

    xi_itest_gateway__test_fixture_t* fixture = _xi_itest_gateway__generate_fixture();

    *fixture_void = fixture;

    xi_globals.backoff_status.backoff_lut_i = 0;
    xi_cancel_backoff_event();

    xi_initialize( "xi_itest_gateway_account_id" );

    xi_state_t state = xi_create_context_with_custom_layers_and_evtd(
        &fixture->xi_context_sut, itest_ct_ml_mc_layer_chain, XI_LAYER_CHAIN_CT_ML_MC,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_CT_ML_MC ), NULL, 1 );

    xi_context = fixture->xi_context_sut;

    XI_CHECK_STATE( state );

    xi_find_handle_for_object( xi_globals.context_handles_vector, fixture->xi_context_sut,
                               &fixture->xi_context_handle );

    state = xi_create_context_with_custom_layers_and_evtd(
        &fixture->xi_context_mockbroker, itest_mock_broker_codec_layer_chain,
        XI_LAYER_CHAIN_MOCK_BROKER_CODEC,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_MOCK_BROKER_CODEC ), NULL, 0 );

    xi_context_mockbroker = fixture->xi_context_mockbroker;

    XI_CHECK_STATE( state );

    state = xi_create_context_with_custom_layers_and_evtd(
        &fixture->xi_context_mockbroker_edge_device_broker,
        itest_mock_broker_codec_layer_chain, XI_LAYER_CHAIN_MOCK_BROKER_CODEC,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_MOCK_BROKER_CODEC ), NULL, 0 );

    XI_CHECK_STATE( state );

    return 0;

err_handling:
    fail();

    return 1;
}

int xi_itest_gateway_teardown( void** fixture_void )
{
    xi_itest_gateway__test_fixture_t* fixture =
        ( xi_itest_gateway__test_fixture_t* )*fixture_void;

    xi_delete_context_with_custom_layers(
        &fixture->xi_context_sut, itest_ct_ml_mc_layer_chain,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_CT_ML_MC ) );

    xi_delete_context_with_custom_layers(
        &fixture->xi_context_mockbroker, itest_mock_broker_codec_layer_chain,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_MOCK_BROKER_CODEC ) );

    xi_delete_context_with_custom_layers(
        &fixture->xi_context_mockbroker_edge_device_broker,
        itest_mock_broker_codec_layer_chain,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_MOCK_BROKER_CODEC ) );

    xi_shutdown();

    XI_SAFE_FREE( fixture );

    return !xi_memory_limiter_teardown();
}

/*********************************************************************************
 * callbacks *********************************************************************
 ********************************************************************************/
static void
_xi_itest_gateway__on_connection_state_changed( xi_context_handle_t in_context_handle,
                                                void* data,
                                                xi_state_t state )
{
    printf( "--- %s ---\n", __FUNCTION__ );

    XI_UNUSED( in_context_handle );
    XI_UNUSED( data );
    XI_UNUSED( state );
}

static void _xi_ed_connect_callback( xi_context_handle_t in_context_handle,
                                     void* data,
                                     xi_state_t state )
{
    printf( "--- %s ---\n", __FUNCTION__ );

    XI_UNUSED( in_context_handle );
    XI_UNUSED( data );
    XI_UNUSED( state );
}

/*********************************************************************************
 * act ***************************************************************************
 ********************************************************************************/
static void _xi_itest_gateway__act( void** fixture_void )
{
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

    xi_state_t state                   = XI_STATE_OK;
    xi_mock_broker_data_t* broker_data = NULL;

    xi_itest_gateway__test_fixture_t* fixture =
        ( xi_itest_gateway__test_fixture_t* )*fixture_void;

    {
        /* initialize mock broker layer chains */
        XI_ALLOC( xi_mock_broker_data_t, broker_data, state );
        broker_data->layer_tunneled_message_target =
            xi_itest_find_layer( fixture->xi_context_mockbroker_edge_device_broker,
                                 XI_LAYER_TYPE_MOCKBROKER_BOTTOM );
        broker_data->layer_output_target =
            xi_itest_find_layer( fixture->xi_context_sut, XI_LAYER_TYPE_MQTT_CODEC_SUT );

        XI_PROCESS_INIT_ON_THIS_LAYER(
            &fixture->xi_context_mockbroker->layer_chain.top->layer_connection,
            broker_data, XI_STATE_OK );

        XI_ALLOC_AT( xi_mock_broker_data_t, broker_data, state );
        broker_data->layer_output_target = xi_itest_find_layer(
            fixture->xi_context_mockbroker, XI_LAYER_TYPE_MOCKBROKER_MQTT_CODEC );

        XI_PROCESS_INIT_ON_THIS_LAYER( &fixture->xi_context_mockbroker_edge_device_broker
                                            ->layer_chain.top->layer_connection,
                                       broker_data, XI_STATE_OK );

        xi_evtd_step( xi_globals.evtd_instance, xi_bsp_time_getcurrenttime_seconds() );
    }

    const uint16_t connection_timeout = fixture->max_loop_count;
    const uint16_t keepalive_timeout  = fixture->max_loop_count;

    xi_connect( fixture->xi_context_handle, "itest_username", "itest_password",
                connection_timeout, keepalive_timeout, XI_SESSION_CLEAN,
                _xi_itest_gateway__on_connection_state_changed );


    uint16_t loop_counter = 0;
    while ( xi_evtd_dispatcher_continue( xi_globals.evtd_instance ) == 1 &&
            loop_counter < keepalive_timeout )
    {
        xi_evtd_step( xi_globals.evtd_instance,
                      xi_bsp_time_getcurrenttime_seconds() + loop_counter );
        ++loop_counter;


        ( 0 == loop_counter % 1 ) ? printf( "." ) : printf( "" );
        fflush( stdout );

        if ( loop_counter == fixture->loop_id__manual_connect_ed )
        {
            xi_gw_edge_device_connect( fixture->xi_context_handle,
                                       "edge application device id",
                                       _xi_ed_connect_callback );
        }

        if ( loop_counter == fixture->loop_id__manual_disconnect_ed )
        {
            /* tunneled edge device disconnect */
            state = xi_gw_edge_device_disconnect( fixture->xi_context_handle,
                                                  "edge application device id" );

            printf( "--- xi_disconnect_ed, state: %d\n", state );
        }

        if ( loop_counter == fixture->loop_id__manual_disconnect )
        {
            /* main client disconnect */
            xi_shutdown_connection( fixture->xi_context_handle );
        }

        if ( loop_counter == fixture->loop_id__manual_disconnect_mockbroker )
        {
#if 1
            XI_PROCESS_CLOSE_ON_THIS_LAYER(
                &fixture->xi_context_mockbroker_edge_device_broker->layer_chain.top
                     ->layer_connection,
                NULL, XI_STATE_OK );
#endif

#if 0
            XI_PROCESS_CLOSE_ON_THIS_LAYER(
                &fixture->xi_context_mockbroker->layer_chain.top->layer_connection, NULL,
                XI_STATE_OK );
#endif
        }
    }

    /* edge device context deletion */
    xi_gw_edge_device_remove( fixture->xi_context_handle, "edge application device id" );

err_handling:

    XI_SAFE_FREE( broker_data );
}

/*********************************************************************************
 * test cases ********************************************************************
 ********************************************************************************/
void xi_itest_gateway__first( void** fixture_void )
{
    /*xi_itest_gateway__test_fixture_t* fixture =
        ( xi_itest_gateway__test_fixture_t* )*fixture_void;*/

    _xi_itest_gateway__act( fixture_void );
}
