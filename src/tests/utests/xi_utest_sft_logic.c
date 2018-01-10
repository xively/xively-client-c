/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"
#include "xi_utest_basic_testcase_frame.h"

#include <stdio.h>
#include <string.h>

#include <xi_sft_logic.h>
#include <xi_macros.h>
#include <xi_helpers.h>
#include <xi_control_message_sft.h>
#include <xi_control_message_sft_generators.h>
#include <xi_bsp_io_fs.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

xi_control_message_t* xi_utest_sft_logic__generate_basic_FUA( const char* filename,
                                                              const char* revision,
                                                              const char* fingerprint )
{
    xi_control_message_t* message_FUA = NULL;

    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_control_message_file_desc_ext_t, one_file_list, state );

    // ARRANGE
    one_file_list->name            = xi_str_dup( filename );
    one_file_list->revision        = xi_str_dup( revision );
    one_file_list->file_operation  = 11;
    one_file_list->size_in_bytes   = 111;
    one_file_list->fingerprint     = ( uint8_t* )xi_str_dup( fingerprint );
    one_file_list->fingerprint_len = 15;

    XI_ALLOC_AT( xi_control_message_t, message_FUA, state );

    message_FUA->file_update_available.common.msgtype =
        XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE;
    message_FUA->file_update_available.common.msgver = 1;
    message_FUA->file_update_available.list_len      = 1;
    message_FUA->file_update_available.list          = one_file_list;

err_handling:

    return message_FUA;
}

#endif

XI_TT_TESTGROUP_BEGIN( utest_sft_logic )


XI_TT_TESTCASE_WITH_SETUP( xi_utest__make_context__invalid_paramter,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               const xi_state_t state =
                                   xi_sft_make_context( NULL, NULL, 0, NULL, NULL, NULL );

                               tt_want_ptr_op( XI_INVALID_PARAMETER, ==, state );
                           } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__make_context__minimal_config__no_leak_after_deallocation,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_sft_context_t* sft_context = NULL;

        xi_state_t state = xi_sft_make_context( &sft_context, NULL, 0, NULL, NULL, NULL );

        tt_want_ptr_op( XI_STATE_OK, ==, state );
        tt_want_ptr_op( NULL, !=, sft_context );

        state = xi_sft_free_context( &sft_context );

        tt_want_ptr_op( XI_STATE_OK, ==, state );
        tt_want_ptr_op( NULL, ==, sft_context );
    } )

XI_TT_TESTCASE_WITH_SETUP( xi_utest__minimal_config__call_on_connected__no_crash,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               xi_sft_context_t* sft_context = NULL;

                               xi_sft_make_context( &sft_context, NULL, 0, NULL, NULL,
                                                    NULL );

                               xi_sft_on_connected( sft_context );

                               xi_sft_free_context( &sft_context );
                           } )

XI_TT_TESTCASE_WITH_SETUP( xi_utest__minimal_config__call_on_connection_failed__no_crash,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               xi_sft_context_t* sft_context = NULL;

                               xi_sft_make_context( &sft_context, NULL, 0, NULL, NULL,
                                                    NULL );

                               xi_sft_on_connection_failed( sft_context );

                               xi_sft_free_context( &sft_context );
                           } )

XI_TT_TESTCASE_WITH_SETUP( xi_utest__minimal_config__call_on_message_with_NULL__no_crash,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               xi_sft_context_t* sft_context = NULL;

                               xi_sft_make_context( &sft_context, NULL, 0, NULL, NULL,
                                                    NULL );

                               xi_sft_on_message( sft_context, NULL );

                               xi_sft_free_context( &sft_context );
                           } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__minimal_config__call_on_message_with_uninitialized_message__no_crash,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_sft_context_t* sft_context = NULL;

        xi_sft_make_context( &sft_context, NULL, 0, NULL, NULL, NULL );

        xi_state_t state = XI_STATE_OK;

        XI_ALLOC( xi_control_message_t, message_uninitialized, state );

        xi_sft_on_message( sft_context, message_uninitialized );

    err_handling:

        xi_sft_free_context( &sft_context );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__minimal_config__call_on_message_with_FILE_CHUNK__no_crash,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_sft_context_t* sft_context = NULL;

        xi_sft_make_context( &sft_context, NULL, 0, NULL, NULL, NULL );

        xi_control_message_t* message_FILE_GET_CHUNK =
            xi_control_message_create_file_get_chunk( "filename", "revision", 11, 22 );

        tt_ptr_op( NULL, !=, message_FILE_GET_CHUNK );

        xi_control_message_t* message_FILE_CHUNK =
            xi_control_message_sft_generate_reply_FILE_CHUNK( message_FILE_GET_CHUNK );

        tt_ptr_op( NULL, !=, message_FILE_CHUNK );

        xi_sft_on_message( sft_context, message_FILE_CHUNK );

    end:
        xi_control_message_free( &message_FILE_GET_CHUNK );
        xi_sft_free_context( &sft_context );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__minimal_config__call_on_message_with_FILE_UPDATE_AVAILABLE__no_crash,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_sft_context_t* sft_context = NULL;

        xi_sft_make_context( &sft_context, NULL, 0, NULL, NULL, NULL );

        xi_control_message_t* message_FUA = xi_utest_sft_logic__generate_basic_FUA(
            "filename", "revision", "fingerprint" );

        tt_ptr_op( NULL, !=, message_FUA );

        xi_sft_on_message( sft_context, message_FUA );

    end:

        xi_sft_free_context( &sft_context );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__minimal_config__call_on_message_with_FILE_UPDATE_AVAILABLE_and_FILE_CHUNK__no_crash,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_sft_context_t* sft_context = NULL;

        xi_sft_make_context( &sft_context, NULL, 0, NULL, NULL, NULL );

        xi_control_message_t* message_FUA = xi_utest_sft_logic__generate_basic_FUA(
            __FUNCTION__, "revision", "fingerprint" );

        xi_sft_on_message( sft_context, message_FUA );

        xi_control_message_t* message_FILE_GET_CHUNK =
            xi_control_message_create_file_get_chunk( __FUNCTION__, "revision", 0, 17 );

        xi_control_message_t* message_FILE_CHUNK =
            xi_control_message_sft_generate_reply_FILE_CHUNK( message_FILE_GET_CHUNK );

        xi_sft_on_message( sft_context, message_FILE_CHUNK );

        tt_ptr_op( NULL, !=, sft_context->checksum_context );

        xi_bsp_io_fs_remove( __FUNCTION__ );
    end:

        xi_control_message_free( &message_FILE_GET_CHUNK );
        xi_sft_free_context( &sft_context );
    } )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
