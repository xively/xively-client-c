/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
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
#include "xi_fs_bsp_to_xi_mapping.h"

#include "xi_memory_checks.h"
#include "xi_itest_layerchain_ct_ml_mc.h"
#include "xi_itest_mock_broker_layerchain.h"
#include "xi_itest_mock_broker_sft_logic.h"
#include "xi_control_message_sft_generators.h"
#include <xi_context.h>

#include "xi_bsp_fwu.h"

#include "xi_control_message_sft.h"

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

#define XI_ITEST_SFT__NUMBER_OF_FILE_CHUNKS( mock_broker_size_multiplier )               \
    ( mock_broker_size_multiplier * XI_MOCK_BROKER_SFT__FILE_CHUNK_STEP_SIZE /           \
          XI_SFT_FILE_CHUNK_SIZE +                                                       \
      ( ( XI_MOCK_BROKER_SFT__FILE_CHUNK_STEP_SIZE % XI_SFT_FILE_CHUNK_SIZE ) ? 1        \
                                                                              : 0 ) )

#define expect_file_status_phase_and_code( _phase, _code )                               \
    expect_value( xi_mock_broker_sft_logic_on_file_status,                               \
                  control_message->file_status.phase, _phase );                          \
    expect_value( xi_mock_broker_sft_logic_on_file_status,                               \
                  control_message->file_status.code, _code );

static void
_xi_itest_sft__check_revision_file( const char** filenames, uint16_t files_count )
{
    uint16_t id_file = 0;
    for ( ; id_file < files_count; ++id_file )
    {
        xi_bsp_io_fs_resource_handle_t resource_handle =
            XI_BSP_IO_FS_INVALID_RESOURCE_HANDLE;
        char* filename_revision = xi_str_cat( filenames[id_file], ".xirev" );

        xi_state_t state = xi_fs_bsp_io_fs_2_xi_state( xi_bsp_io_fs_open(
            filename_revision, 0, XI_BSP_IO_FS_OPEN_READ, &resource_handle ) );
        XI_SAFE_FREE( filename_revision );

        assert_int_equal( XI_STATE_OK, state );
        assert_ptr_not_equal( XI_BSP_IO_FS_INVALID_RESOURCE_HANDLE, resource_handle );

        const uint8_t* buffer = NULL;
        size_t buffer_size    = 0;

        state = xi_fs_bsp_io_fs_2_xi_state(
            xi_bsp_io_fs_read( resource_handle, 0, &buffer, &buffer_size ) );

        assert_non_null( buffer );
        assert_int_equal( 71, buffer_size );
        assert_int_equal( XI_STATE_OK, state );

        char* expected_revision = xi_str_dup( XI_CONTROL_MESSAGE_SFT_GENERATED_REVISION );
        /* replay mock broker's last character incrementation */
        ++expected_revision[strlen( expected_revision ) - 1];

        /* client fills up revision if empty, mock broker increments last character by
         * one, that's the expected string */
        assert_memory_equal( expected_revision, buffer, buffer_size );

        XI_SAFE_FREE( expected_revision );

        state = xi_fs_bsp_io_fs_2_xi_state( xi_bsp_io_fs_close( resource_handle ) );

        assert_int_equal( XI_STATE_OK, state );
    }
}

/*********************************************************************************
 * test fixture ******************************************************************
 ********************************************************************************/
typedef struct xi_itest_sft__test_fixture_s
{
    const char* control_topic_name_client_in;
    const char* control_topic_name_client_out;

    uint16_t loop_id__manual_disconnect;

    uint16_t max_loop_count;

} xi_itest_sft__test_fixture_t;


static xi_itest_sft__test_fixture_t* _xi_itest_sft__generate_fixture()
{
    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_itest_sft__test_fixture_t, fixture, state );

    fixture->control_topic_name_client_in  = ( "xi/ctrl/v1/itest_username/cln" );
    fixture->control_topic_name_client_out = ( "xi/ctrl/v1/itest_username/svc" );

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

    *fixture_void = _xi_itest_sft__generate_fixture();

    xi_globals.backoff_status.backoff_lut_i = 0;
    xi_cancel_backoff_event();

    xi_initialize( "xi_itest_sft_account_id", "xi_itest_sft_device_id" );

    XI_CHECK_STATE( xi_create_context_with_custom_layers_and_evtd(
        &xi_context, itest_ct_ml_mc_layer_chain, XI_LAYER_CHAIN_CT_ML_MC,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_CT_ML_MC ), NULL, 1 ) );

    xi_find_handle_for_object( xi_globals.context_handles_vector, xi_context,
                               &xi_context_handle );

    XI_CHECK_STATE( xi_create_context_with_custom_layers_and_evtd(
        &xi_context_mockbroker, itest_mock_broker_codec_layer_chain,
        XI_LAYER_CHAIN_MOCK_BROKER_CODEC,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_MOCK_BROKER_CODEC ), NULL, 0 ) );

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

    xi_itest_sft__test_fixture_t* fixture =
        ( xi_itest_sft__test_fixture_t* )*fixture_void;

    XI_SAFE_FREE( fixture );

    return !xi_memory_limiter_teardown();
}

static void
_xi_itest_sft__on_connection_state_changed( xi_context_handle_t in_context_handle,
                                            void* data,
                                            xi_state_t state )
{
    XI_UNUSED( in_context_handle );
    XI_UNUSED( data );
    XI_UNUSED( state );
}

static void _xi_itest_sft__remove_files( const char** filenames, uint16_t files_count )
{
    uint16_t id_file = 0;
    for ( ; id_file < files_count; ++id_file )
    {
        /* delete files generated during tests (files received from mock broker) */
        xi_bsp_io_fs_remove( filenames[id_file] );

        /* delete files containing revisionstrings */
        char* filename_revision = xi_str_cat( filenames[id_file], ".xirev" );
        xi_bsp_io_fs_remove( filename_revision );
        XI_SAFE_FREE( filename_revision );
    }
}

/*********************************************************************************
 * act ***************************************************************************
 ********************************************************************************/
static void xi_itest_sft__act( void** fixture_void,
                               uint8_t remove_temporary_files_at_the_end,
                               const char** updateable_filenames,
                               uint16_t updateable_files_count,
                               xi_sft_url_handler_callback_t url_handler_callback )
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
    broker_data->layer_output_target =
        xi_itest_find_layer( xi_context, XI_LAYER_TYPE_MQTT_CODEC_SUT );

    XI_PROCESS_INIT_ON_THIS_LAYER(
        &xi_context_mockbroker->layer_chain.top->layer_connection, broker_data,
        XI_STATE_OK );

    xi_evtd_step( xi_globals.evtd_instance, xi_bsp_time_getcurrenttime_seconds() );

    xi_set_updateable_files( xi_context_handle, updateable_filenames,
                             updateable_files_count, url_handler_callback );

    const uint16_t connection_timeout = fixture->max_loop_count;
    const uint16_t keepalive_timeout  = fixture->max_loop_count;

    xi_connect( xi_context_handle, "itest_username", "itest_password", connection_timeout,
                keepalive_timeout, XI_SESSION_CLEAN,
                &_xi_itest_sft__on_connection_state_changed );

    uint16_t loop_counter = 0;
    while ( xi_evtd_dispatcher_continue( xi_globals.evtd_instance ) == 1 &&
            loop_counter < keepalive_timeout )
    {
        xi_evtd_step( xi_globals.evtd_instance,
                      xi_bsp_time_getcurrenttime_seconds() + loop_counter );
        ++loop_counter;

        if ( loop_counter == fixture->loop_id__manual_disconnect )
        {
            xi_shutdown_connection( xi_context_handle );
        }
    }

err_handling:

    if ( 1 == remove_temporary_files_at_the_end )
    {
        _xi_itest_sft__remove_files( updateable_filenames, updateable_files_count );
    }
}

/*********************************************************************************
 * test cases ********************************************************************
 ********************************************************************************/
void xi_itest_sft__client_doesnt_start_SFT_if_no_update_file_is_set( void** fixture_void )
{
    /* SFT process does not start at all in case of not set updateable files.
     * Here the lack of expectations means no expected SFT messages arriving on broker
     * side. */

    /* ACT */
    xi_itest_sft__act( fixture_void, 1, NULL, 0, NULL );
}

void xi_itest_sft__basic_flow__SFT_with_happy_broker__protocol_intact(
    void** fixture_void )
{
    expect_value( xi_mock_broker_sft_logic_on_message, control_message->common.msgtype,
                  XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO );

    /* 1st file */
    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK,
                        XI_ITEST_SFT__NUMBER_OF_FILE_CHUNKS( 1 ) );

    expect_string_count( xi_mock_broker_sft_logic_on_file_get_chunk,
                         control_message->file_get_chunk.name, "file1",
                         XI_ITEST_SFT__NUMBER_OF_FILE_CHUNKS( 1 ) );

    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 2 );
    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
    expect_file_status_phase_and_code( XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_FINISHED,
                                       XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );

    /* 2nd file */
    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK,
                        XI_ITEST_SFT__NUMBER_OF_FILE_CHUNKS( 2 ) );

    expect_string_count( xi_mock_broker_sft_logic_on_file_get_chunk,
                         control_message->file_get_chunk.name, "file2",
                         XI_ITEST_SFT__NUMBER_OF_FILE_CHUNKS( 2 ) );

    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 2 );
    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
    expect_file_status_phase_and_code( XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_FINISHED,
                                       XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );

    /* ACT */
    xi_itest_sft__act( fixture_void, 1, ( const char* [] ){"file1", "file2"}, 2, NULL );
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

    /* ACT */
    xi_itest_sft__act( fixture_void, 1, ( const char* [] ){"file1"}, 1, NULL );

err_handling:;
}

void xi_itest_sft__broker_replies_FUA_on_FILE_GET_CHUNK__client_processes_2nd_FUA(
    void** fixture_void )
{
    xi_state_t state = XI_STATE_OK;

    xi_control_message_t* control_message_reply = NULL;

    /* generate a test case specific mock broker reply on FILE_GET_CHUNK */
    {
        XI_ALLOC_AT( xi_control_message_t, control_message_reply, state );
        XI_ALLOC( xi_control_message_file_desc_ext_t, single_file_desc, state );

        *single_file_desc = ( xi_control_message_file_desc_ext_t ){
            xi_str_dup( "file2" ), xi_str_dup( "rev2" ), 11, 22, NULL, 12, NULL, 0};

        xi_mock_broker_sft_logic_get_fingerprint( single_file_desc->size_in_bytes,
                                                  &single_file_desc->fingerprint,
                                                  &single_file_desc->fingerprint_len );

        control_message_reply->common.msgtype =
            XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE;
        control_message_reply->common.msgver = 1;

        control_message_reply->file_update_available.list_len = 1;
        control_message_reply->file_update_available.list     = single_file_desc;

        /* This will cause mock broker to reply with FILE_UPDATE_AVAILABLE message on
         * a FILE_GET_CHUNK message which is a protocol violation */
        will_return( xi_mock_broker_sft_logic_on_file_get_chunk, control_message_reply );
    }

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
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 2 );

    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
    expect_file_status_phase_and_code( XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_FINISHED,
                                       XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );

    /* ACT */
    xi_itest_sft__act( fixture_void, 1, ( const char* [] ){"file1"}, 1, NULL );

    _xi_itest_sft__remove_files( ( const char* [] ){"file2"}, 1 );

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
        /* dynamic filename genration, trying to avoid invalid characters in filenames */
        file_names_strings[id_file][0] = id_file % 75 + 48;
        file_names_strings[id_file][1] = id_file % 75 + 49;
        file_names_strings[id_file][2] = '-';
        file_names_strings[id_file][3] = id_file / 1000 + '0';
        file_names_strings[id_file][4] = id_file % 1000 / 100 + '0';
        file_names_strings[id_file][5] = id_file % 100 / 10 + '0';
        file_names_strings[id_file][6] = id_file % 10 + '0';
        file_names_strings[id_file][7] = 0; /* end of string */

        file_names_ptrs[id_file] = file_names_strings[id_file];

        /* calculating expected number of FILE_GET_CHUNK messages */
        const uint32_t num_of_file_get_chunks =
            XI_ITEST_SFT__NUMBER_OF_FILE_CHUNKS( ( id_file % 3 + 1 ) );

        expect_value_count(
            xi_mock_broker_sft_logic_on_message, control_message->common.msgtype,
            XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK, num_of_file_get_chunks );

        expect_string_count( xi_mock_broker_sft_logic_on_file_get_chunk,
                             control_message->file_get_chunk.name,
                             file_names_ptrs[id_file], num_of_file_get_chunks );

        expect_value_count( xi_mock_broker_sft_logic_on_message,
                            control_message->common.msgtype,
                            XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 2 );
        expect_file_status_phase_and_code(
            XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
            XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
        expect_file_status_phase_and_code(
            XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_FINISHED,
            XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
    }

    /* ACT */
    xi_itest_sft__act( fixture_void, 1, ( const char** )file_names_ptrs,
                       XI_ITEST_SFT__FILE_NUMBER, NULL );
}

void xi_itest_sft__firmware_bin_received__firmware_test_commit_triggered(
    void** fixture_void )
{
    expect_value( xi_mock_broker_sft_logic_on_message, control_message->common.msgtype,
                  XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO );

    /* 1st file */
    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK,
                        XI_ITEST_SFT__NUMBER_OF_FILE_CHUNKS( 1 ) );

    expect_string_count( xi_mock_broker_sft_logic_on_file_get_chunk,
                         control_message->file_get_chunk.name, "file1",
                         XI_ITEST_SFT__NUMBER_OF_FILE_CHUNKS( 1 ) );

    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 2 );
    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
    expect_file_status_phase_and_code( XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_FINISHED,
                                       XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );


    /* 2nd file */
    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK,
                        XI_ITEST_SFT__NUMBER_OF_FILE_CHUNKS( 2 ) );

    expect_string_count( xi_mock_broker_sft_logic_on_file_get_chunk,
                         control_message->file_get_chunk.name, "firmware.bin",
                         XI_ITEST_SFT__NUMBER_OF_FILE_CHUNKS( 2 ) );

    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 1 );
    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );

    /* 3rd file */
    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK,
                        XI_ITEST_SFT__NUMBER_OF_FILE_CHUNKS( 3 ) );

    expect_string_count( xi_mock_broker_sft_logic_on_file_get_chunk,
                         control_message->file_get_chunk.name, "file3",
                         XI_ITEST_SFT__NUMBER_OF_FILE_CHUNKS( 3 ) );

    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 2 );
    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
    expect_file_status_phase_and_code( XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_FINISHED,
                                       XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );

    /* this is the firmware FILE_STATUS report */
    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 1 );
    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_PROCESSING,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );

    /* ACT */
    xi_itest_sft__act( fixture_void, 1,
                       ( const char* [] ){"file1", "firmware.bin", "file3"}, 3, NULL );
}

void xi_itest_sft__revision_non_volatile_storage__proper_value_stored(
    void** fixture_void )
{
    expect_value( xi_mock_broker_sft_logic_on_message, control_message->common.msgtype,
                  XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO );

    /* 1st file */
    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK,
                        XI_ITEST_SFT__NUMBER_OF_FILE_CHUNKS( 1 ) );

    expect_string_count( xi_mock_broker_sft_logic_on_file_get_chunk,
                         control_message->file_get_chunk.name, "uniquefilename.file1",
                         XI_ITEST_SFT__NUMBER_OF_FILE_CHUNKS( 1 ) );

    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 2 );

    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
    expect_file_status_phase_and_code( XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_FINISHED,
                                       XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );

    /* 2nd file */
    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK,
                        XI_ITEST_SFT__NUMBER_OF_FILE_CHUNKS( 2 ) );

    expect_string_count(
        xi_mock_broker_sft_logic_on_file_get_chunk, control_message->file_get_chunk.name,
        "uniquefilename.firmware.bin", XI_ITEST_SFT__NUMBER_OF_FILE_CHUNKS( 2 ) );

    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 2 );

    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
    expect_file_status_phase_and_code( XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_FINISHED,
                                       XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );

    const char** files_under_update =
        ( const char* [] ){"uniquefilename.file1", "uniquefilename.firmware.bin"};

    /* delete files accidentally existing due to previous run's test case failure */
    _xi_itest_sft__remove_files( files_under_update, 2 );

    /* ACT */
    xi_itest_sft__act( fixture_void, 0, files_under_update, 2, NULL );

    /* check revision file for existing and proper content */
    _xi_itest_sft__check_revision_file( files_under_update, 2 );

    /* file removal only happens here if all expectations are met, otherwise conrols
     * exits before */
    _xi_itest_sft__remove_files( files_under_update, 2 );
}

void xi_itest_sft__checksum_mismatch__update_process_exits( void** fixture_void )
{
    expect_value( xi_mock_broker_sft_logic_on_message, control_message->common.msgtype,
                  XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO );

    will_return( xi_mock_broker_sft_logic_get_fingerprint,
                 xi_str_dup( "mismatching fingerprint" ) );
    will_return( xi_mock_broker_sft_logic_get_fingerprint, 24 );

    /* 1st file */
    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK,
                        XI_ITEST_SFT__NUMBER_OF_FILE_CHUNKS( 1 ) );

    expect_string_count( xi_mock_broker_sft_logic_on_file_get_chunk,
                         control_message->file_get_chunk.name, "file1",
                         XI_ITEST_SFT__NUMBER_OF_FILE_CHUNKS( 1 ) );

    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 1 );

    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__FILE_CHECKSUM_MISMATCH );

    /* package update process exits here, 2nd file is not downloaded at all */

    /* ACT */
    xi_itest_sft__act( fixture_void, 1, ( const char* [] ){"file1", "file2"}, 2, NULL );
}

/**************************************************
 * custom external URL download tests *************
 **************************************************/

typedef enum _xi_itest_sft__url_handler_callback_behaviour_e {
    URL_HANDLER__DOWNLOAD_GETS_REJECTED = 0,
    URL_HANDLER__DOWNLOAD_FAILS         = 1,
    URL_HANDLER__DOWNLOAD_SUCCEEDS      = 2
} _xi_itest_sft__url_handler_callback_behaviour_t;

static uint8_t _xi_itest_sft__url_handler_callback(
    const char* url,
    const char* filename,
    uint8_t* checksum,
    uint16_t checksum_len,
    uint8_t flag_mqtt_download_available,
    xi_sft_on_file_downloaded_callback_t* fn_on_file_downloaded,
    void* callback_data )
{
    XI_UNUSED( url );
    XI_UNUSED( checksum );
    XI_UNUSED( checksum_len );
    XI_UNUSED( flag_mqtt_download_available );

    uint8_t return_value = mock_type( uint8_t );

    if ( URL_HANDLER__DOWNLOAD_GETS_REJECTED != return_value )
    {
        ( *fn_on_file_downloaded )( callback_data, filename,
                                    ( URL_HANDLER__DOWNLOAD_FAILS == return_value ) ? 0
                                                                                    : 1 );
    }

    return ( URL_HANDLER__DOWNLOAD_GETS_REJECTED == return_value ) ? 0 : 1;
}

void xi_itest_sft__custom_URL_download__single_file( void** fixture_void )
{
    expect_value( xi_mock_broker_sft_logic_on_message, control_message->common.msgtype,
                  XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO );

    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 3 );

    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADING,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
    expect_file_status_phase_and_code( XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_FINISHED,
                                       XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );

    will_return( _xi_itest_sft__url_handler_callback, URL_HANDLER__DOWNLOAD_SUCCEEDS );

    /* ACT */
    xi_itest_sft__act( fixture_void, 1, ( const char* [] ){"custom_url.dl"}, 1,
                       _xi_itest_sft__url_handler_callback );
}

void xi_itest_sft__custom_URL_download__single_file_firmware( void** fixture_void )
{
    expect_value( xi_mock_broker_sft_logic_on_message, control_message->common.msgtype,
                  XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO );

    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 3 );

    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADING,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_PROCESSING,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );

    will_return( _xi_itest_sft__url_handler_callback, URL_HANDLER__DOWNLOAD_SUCCEEDS );

    /* ACT */
    xi_itest_sft__act( fixture_void, 1, ( const char* [] ){"firmware.bin"}, 1,
                       _xi_itest_sft__url_handler_callback );
}

void xi_itest_sft__custom_URL_download__two_files_oneURL_oneMQTT( void** fixture_void )
{
    expect_value( xi_mock_broker_sft_logic_on_message, control_message->common.msgtype,
                  XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO );

    /* 1nd file: URL download */
    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 3 );

    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADING,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
    expect_file_status_phase_and_code( XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_FINISHED,
                                       XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );

    /* 2nd file: fallback to MQTT download */
    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 1 );
    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADING,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__URLDL_REJECTED );

    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK,
                        XI_ITEST_SFT__NUMBER_OF_FILE_CHUNKS( 2 ) );

    expect_string_count( xi_mock_broker_sft_logic_on_file_get_chunk,
                         control_message->file_get_chunk.name, "custom_url.dl.2",
                         XI_ITEST_SFT__NUMBER_OF_FILE_CHUNKS( 2 ) );

    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 2 );

    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
    expect_file_status_phase_and_code( XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_FINISHED,
                                       XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );

    will_return( _xi_itest_sft__url_handler_callback, URL_HANDLER__DOWNLOAD_SUCCEEDS );
    will_return( _xi_itest_sft__url_handler_callback,
                 URL_HANDLER__DOWNLOAD_GETS_REJECTED );

    will_return_always( xi_mock_broker_sft_logic_generate_reply_happy_FUA, 1 );

    /* ACT */
    xi_itest_sft__act( fixture_void, 1,
                       ( const char* [] ){"custom_url.dl.1", "custom_url.dl.2"}, 2,
                       _xi_itest_sft__url_handler_callback );
}

void xi_itest_sft__custom_URL_download__three_files_URLFAIL_fallback_URLREJECTED_fallback_URLSUCCEEDS(
    void** fixture_void )
{
    expect_value( xi_mock_broker_sft_logic_on_message, control_message->common.msgtype,
                  XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO );

    /* 1st file: URL download */
    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 2 );

    /* 1st file URL download starts, but fails */
    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADING,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__URLDL_FAILED );

    /* 1st file fallback to MQTT download */
    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK,
                        XI_ITEST_SFT__NUMBER_OF_FILE_CHUNKS( 1 ) );

    expect_string_count( xi_mock_broker_sft_logic_on_file_get_chunk,
                         control_message->file_get_chunk.name, "custom_url.dl.fails",
                         XI_ITEST_SFT__NUMBER_OF_FILE_CHUNKS( 1 ) );

    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 2 );

    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
    expect_file_status_phase_and_code( XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_FINISHED,
                                       XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );


    /* 2nd file: fallback to MQTT download */
    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 1 );
    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADING,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__URLDL_REJECTED );

    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK,
                        XI_ITEST_SFT__NUMBER_OF_FILE_CHUNKS( 2 ) );

    expect_string_count( xi_mock_broker_sft_logic_on_file_get_chunk,
                         control_message->file_get_chunk.name, "custom_url.dl.rejected",
                         XI_ITEST_SFT__NUMBER_OF_FILE_CHUNKS( 2 ) );

    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 2 );

    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
    expect_file_status_phase_and_code( XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_FINISHED,
                                       XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );


    /* 3rd file: successul URL download */
    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 3 );

    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADING,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
    expect_file_status_phase_and_code( XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_FINISHED,
                                       XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );

    will_return( _xi_itest_sft__url_handler_callback, URL_HANDLER__DOWNLOAD_FAILS );
    will_return( _xi_itest_sft__url_handler_callback,
                 URL_HANDLER__DOWNLOAD_GETS_REJECTED );
    will_return( _xi_itest_sft__url_handler_callback, URL_HANDLER__DOWNLOAD_SUCCEEDS );

    will_return_always( xi_mock_broker_sft_logic_generate_reply_happy_FUA, 1 );

    /* ACT */
    xi_itest_sft__act( fixture_void, 1,
                       ( const char* [] ){"custom_url.dl.fails", "custom_url.dl.rejected",
                                          "custom_url.dl.succeeds"},
                       3, _xi_itest_sft__url_handler_callback );
}

void xi_itest_sft__custom_URL_download__two_files__URLREJECTED__no_fallback_available(
    void** fixture_void )
{
    expect_value( xi_mock_broker_sft_logic_on_message, control_message->common.msgtype,
                  XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO );

    /* 1st file: URL download */
    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 1 );

    /* 1st file URL download rejected */
    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADING,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__URLDL_REJECTED );

    will_return( _xi_itest_sft__url_handler_callback,
                 URL_HANDLER__DOWNLOAD_GETS_REJECTED );

    will_return( xi_mock_broker_sft_logic_generate_reply_happy_FUA, 0 );

    /* ACT */
    xi_itest_sft__act( fixture_void, 1, ( const char* [] ){"custom_url.dl.rejected",
                                                           "package.download.stops.here"},
                       2, _xi_itest_sft__url_handler_callback );
}

void xi_itest_sft__custom_URL_download__two_files__URLFAILS__no_fallback_available(
    void** fixture_void )
{
    expect_value( xi_mock_broker_sft_logic_on_message, control_message->common.msgtype,
                  XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO );

    /* 1st file: URL download */
    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 2 );

    /* 1st file URL download rejected */
    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADING,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__URLDL_FAILED );

    will_return( _xi_itest_sft__url_handler_callback, URL_HANDLER__DOWNLOAD_FAILS );

    will_return( xi_mock_broker_sft_logic_generate_reply_happy_FUA, 0 );

    /* ACT */
    xi_itest_sft__act( fixture_void, 1, ( const char* [] ){"custom_url.dl.fails",
                                                           "package.download.stops.here"},
                       2, _xi_itest_sft__url_handler_callback );
}

/***************************************************************
 * testing checksum passed the URL handler callback ************
 ***************************************************************/

static uint8_t _xi_itest_sft__url_handler_callback_for_checksum_validation(
    const char* url,
    const char* filename,
    uint8_t* checksum,
    uint16_t checksum_len,
    uint8_t flag_mqtt_download_available,
    xi_sft_on_file_downloaded_callback_t* fn_on_file_downloaded,
    void* callback_data )
{
    XI_UNUSED( url );
    XI_UNUSED( filename );
    XI_UNUSED( flag_mqtt_download_available );
    XI_UNUSED( fn_on_file_downloaded );
    XI_UNUSED( callback_data );

    assert_ptr_not_equal( NULL, checksum );
    assert_int_not_equal( 0, checksum_len );

    /* doing nothing else "just" validating that the checksums are equal: the one checksum
     * received as this function's argument and the checksum sent by the broker (if the
     * client sent some different byte array) */
    {
        /* this is the same byte chunk what broker send as the content of the first file
         * in the update package */
        uint8_t* randomlike_bytes =
            xi_control_message_sft_get_reproducible_randomlike_bytes(
                0, XI_MOCK_BROKER_SFT__FILE_CHUNK_STEP_SIZE );

        void* local_checksum_context = NULL;
        xi_bsp_fwu_checksum_init( &local_checksum_context );

        xi_bsp_fwu_checksum_update( local_checksum_context, randomlike_bytes,
                                    XI_MOCK_BROKER_SFT__FILE_CHUNK_STEP_SIZE );

        uint8_t* local_checksum_buffer     = NULL;
        uint16_t local_checksum_buffer_len = 0;
        xi_bsp_fwu_checksum_final( &local_checksum_context, &local_checksum_buffer,
                                   &local_checksum_buffer_len );

        /* check if the URL handler callback receives the same checksum which is generated
         * based on the content of the first file in the update package */
        assert_int_equal( checksum_len, local_checksum_buffer_len );
        assert_memory_equal( checksum, local_checksum_buffer, checksum_len );

        XI_SAFE_FREE( randomlike_bytes );
    }

    return 1;
}

void xi_itest_sft__custom_URL_download_checksum_validation__single_file(
    void** fixture_void )
{
    expect_value( xi_mock_broker_sft_logic_on_message, control_message->common.msgtype,
                  XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO );

    expect_value_count( xi_mock_broker_sft_logic_on_message,
                        control_message->common.msgtype,
                        XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS, 1 );

    expect_file_status_phase_and_code(
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADING,
        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );

    /* ACT */
    xi_itest_sft__act( fixture_void, 1,
                       ( const char* [] ){"custom_url_checksum_validation.dl"}, 1,
                       _xi_itest_sft__url_handler_callback_for_checksum_validation );
}
