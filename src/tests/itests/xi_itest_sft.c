/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_itest_sft.h>
#include "xi_itest_helpers.h"
#include "xi_backoff_status_api.h"
#include "xi_bsp_io_fs.h"

#include "xi_debug.h"
#include "xi_globals.h"
#include "xi_handle.h"
#include "xi_helpers.h"

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

    uint16_t loop_id__manual_disconnect;

    uint16_t max_loop_count;

} xi_itest_sft__test_fixture_t;


xi_itest_sft__test_fixture_t* xi_itest_sft__generate_fixture()
{
    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_itest_sft__test_fixture_t, fixture, state );

    fixture->control_topic_name_client_in  = ( "xi/ctrl/v1/xi_itest_sft_device_id/cln" );
    fixture->control_topic_name_client_out = ( "xi/ctrl/v1/xi_itest_sft_device_id/svc" );

    fixture->loop_id__manual_disconnect = 0xFFFF - 5;
    fixture->max_loop_count             = 0xFFFF;

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

void xi_itest_sft__remove_files( const char** filenames, uint16_t files_count )
{
    uint16_t id_file = 0;
    for ( ; id_file < files_count; ++id_file )
    {
        xi_bsp_io_fs_remove( filenames[id_file] );
    }
}

/*********************************************************************************
 * act ***************************************************************************
 ********************************************************************************/
static void xi_itest_sft__act( void** fixture_void,
                               char do_disconnect_flag,
                               const char** updateable_filenames,
                               uint16_t updateable_files_count )
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
                             updateable_files_count );

    const uint16_t connection_timeout = fixture->max_loop_count;
    const uint16_t keepalive_timeout  = fixture->max_loop_count;
    xi_connect( xi_context_handle, "itest_username", "itest_password", connection_timeout,
                keepalive_timeout, XI_SESSION_CLEAN,
                &xi_itest_sft__on_connection_state_changed );

    uint16_t loop_counter = 0;
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

err_handling:

    xi_itest_sft__remove_files( updateable_filenames, updateable_files_count );
}

/*********************************************************************************
 * test cases ********************************************************************
 ********************************************************************************/
void xi_itest_sft__client_doesnt_start_SFT_if_no_update_file_is_set( void** fixture_void )
{
    /* SFT process does not start at all in case of not set updateable files.
     * Here the lack of expectations means no expected SFT messages arriving on broker
     * side. */

    // ACT
    xi_itest_sft__act( fixture_void, 1, NULL, 0 );
}

void xi_itest_sft__basic_flow__SFT_with_happy_broker__protocol_intact(
    void** fixture_void )
{
    expect_value( xi_mock_broker_sft_logic_on_message, control_message->common.msgtype,
                  XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO );

    /* 1st file */
    expect_value_count(
        xi_mock_broker_sft_logic_on_message, control_message->common.msgtype,
        XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK, 7777 / XI_SFT_FILE_CHUNK_SIZE + 1 );

    expect_string_count( xi_mock_broker_sft_logic_on_file_get_chunk,
                         control_message->file_get_chunk.name, "file1",
                         7777 / XI_SFT_FILE_CHUNK_SIZE + 1 );

    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 3 );

    /* 2nd file */
    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK,
                        2 * 7777 / XI_SFT_FILE_CHUNK_SIZE + 1 );

    expect_string_count( xi_mock_broker_sft_logic_on_file_get_chunk,
                         control_message->file_get_chunk.name, "file2",
                         2 * 7777 / XI_SFT_FILE_CHUNK_SIZE + 1 );

    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 3 );

    // ACT
    xi_itest_sft__act( fixture_void, 1, ( const char* [] ){"file1", "file2"}, 2 );
}

void xi_itest_sft__broker_replies_FILE_INFO_on_FILE_GET_CHUNK__client_does_not_crash_or_leak(
    void** fixture_void )
{
    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_control_message_t, control_message_reply, state );

    control_message_reply->common.msgtype = XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO;

    will_return( xi_mock_broker_sft_logic_on_file_get_chunk, control_message_reply );

    expect_value( xi_mock_broker_sft_logic_on_message, control_message->common.msgtype,
                  XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO );

    /* 1st file */
    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK, 1 );

    expect_string_count( xi_mock_broker_sft_logic_on_file_get_chunk,
                         control_message->file_get_chunk.name, "file1", 1 );

    // ACT
    xi_itest_sft__act( fixture_void, 1, ( const char* [] ){"file1"}, 1 );

err_handling:;
}

void xi_itest_sft__broker_replies_FUA_on_FILE_GET_CHUNK__client_processes_2nd_FUA(
    void** fixture_void )
{
    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_control_message_t, control_message_reply, state );
    XI_ALLOC( xi_control_message_file_desc_ext_t, single_file_desc, state );

    *single_file_desc =
        ( xi_control_message_file_desc_ext_t ){xi_str_dup( "file2" ),
                                               xi_str_dup( "rev2" ),
                                               11,
                                               22,
                                               ( uint8_t* )xi_str_dup( "fingerprint1" ),
                                               12};

    control_message_reply->common.msgtype =
        XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE;
    control_message_reply->common.msgver = 1;

    control_message_reply->file_update_available.list_len = 1;
    control_message_reply->file_update_available.list     = single_file_desc;

    /* This will cause mock broker to reply with FILE_UPDATE_AVAILABLE message on
     * a FILE_GET_CHUNK message which is a protocol violation */
    will_return( xi_mock_broker_sft_logic_on_file_get_chunk, control_message_reply );

    expect_value( xi_mock_broker_sft_logic_on_message, control_message->common.msgtype,
                  XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO );

    /* 1st file */
    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK, 1 );

    expect_string_count( xi_mock_broker_sft_logic_on_file_get_chunk,
                         control_message->file_get_chunk.name, "file1", 1 );

    /* new file */
    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK, 1 );

    expect_string_count( xi_mock_broker_sft_logic_on_file_get_chunk,
                         control_message->file_get_chunk.name, "file2", 1 );

    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 3 );

    // ACT
    xi_itest_sft__act( fixture_void, 1, ( const char* [] ){"file1"}, 1 );

    xi_itest_sft__remove_files( ( const char* [] ){"file2"}, 1 );

err_handling:;
}

#define XI_ITEST_SFT__FILE_NUMBER 111

void xi_itest_sft__manymany_updateable_files( void** fixture_void )
{
    expect_value( xi_mock_broker_sft_logic_on_message, control_message->common.msgtype,
                  XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO );

    char file_names_strings[XI_ITEST_SFT__FILE_NUMBER][8] = {{0}};
    char* file_names_ptrs[XI_ITEST_SFT__FILE_NUMBER]      = {0};

    uint16_t id_file = 0;
    for ( ; id_file < XI_ITEST_SFT__FILE_NUMBER; ++id_file )
    {
        /* dynamic genration of filenames */
        file_names_strings[id_file][0] = id_file % 95 + 33;
        file_names_strings[id_file][1] = id_file % 95 + 34;
        file_names_strings[id_file][2] = '-';
        file_names_strings[id_file][3] = id_file / 1000 + '0';
        file_names_strings[id_file][4] = id_file % 1000 / 100 + '0';
        file_names_strings[id_file][5] = id_file % 100 / 10 + '0';
        file_names_strings[id_file][6] = id_file % 10 + '0';
        file_names_strings[id_file][7] = 0; /* end of string */

        file_names_ptrs[id_file] = file_names_strings[id_file];

        /* calculating expected number of FILE_GET_CHUNK messages */
        const uint32_t num_of_file_get_chunks =
            7777 * ( id_file % 3 + 1 ) / XI_SFT_FILE_CHUNK_SIZE + 1;

        expect_value_count(
            xi_mock_broker_sft_logic_on_message, control_message->common.msgtype,
            XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK, num_of_file_get_chunks );

        expect_string_count( xi_mock_broker_sft_logic_on_file_get_chunk,
                             control_message->file_get_chunk.name,
                             file_names_ptrs[id_file], num_of_file_get_chunks );

        expect_value_count( xi_mock_broker_sft_logic_on_message,
                            control_message->common.msgtype,
                            XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 3 );
    }

    // ACT
    xi_itest_sft__act( fixture_void, 1, ( const char** )file_names_ptrs,
                       XI_ITEST_SFT__FILE_NUMBER );
}
