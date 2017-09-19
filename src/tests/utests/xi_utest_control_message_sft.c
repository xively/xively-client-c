/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
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

#include <xi_control_message_sft.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#endif

XI_TT_TESTGROUP_BEGIN( utest_control_message_sft )


/*****************************************************
 * FILE_INFO *****************************************
 *****************************************************/

XI_TT_TESTCASE_WITH_SETUP( xi_utest__FILE_INFO__invalid_input__null_output_expected,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               xi_control_message_t* message_file_info =
                                   xi_control_message_create_file_info( NULL, 0 );

                               tt_want_ptr_op( NULL, ==, message_file_info );
                           } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__FILE_INFO__single_filename_provided_but_zero_count__null_output_expected,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        const char* single_filename = "filename";

        xi_control_message_t* message_file_info =
            xi_control_message_create_file_info( &single_filename, 0 );

        tt_want_ptr_op( NULL, ==, message_file_info );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__FILE_INFO__single_filename_provided__valid_output_message_expected,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        const char* single_filename = "filename";

        xi_control_message_t* message_file_info =
            xi_control_message_create_file_info( &single_filename, 1 );

        tt_ptr_op( NULL, !=, message_file_info );
        tt_want_int_op( XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO, ==,
                        message_file_info->common.msgtype );
        tt_want_int_op( 1, ==, message_file_info->common.msgver );


        tt_want_int_op( 1, ==, message_file_info->file_info.list_len );
        tt_ptr_op( NULL, !=, message_file_info->file_info.list );

        tt_ptr_op( NULL, !=, message_file_info->file_info.list->name );
        tt_want_int_op(
            0, ==, strcmp( single_filename, message_file_info->file_info.list->name ) );

        tt_want_ptr_op( NULL, !=, message_file_info->file_info.list->revision );

        xi_control_message_free( &message_file_info );

    end:;
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__FILE_INFO__single_filename_and_revision_provided__valid_output_message_expected,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        const char* single_filename = "filename";

        xi_control_message_t* message_file_info =
            xi_control_message_create_file_info( &single_filename, 1 );

        tt_ptr_op( NULL, !=, message_file_info );
        tt_want_int_op( XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO, ==,
                        message_file_info->common.msgtype );
        tt_want_int_op( 1, ==, message_file_info->common.msgver );


        tt_want_int_op( 1, ==, message_file_info->file_info.list_len );
        tt_ptr_op( NULL, !=, message_file_info->file_info.list );

        tt_ptr_op( NULL, !=, message_file_info->file_info.list->name );
        tt_want_int_op(
            0, ==, strcmp( single_filename, message_file_info->file_info.list->name ) );

        tt_ptr_op( NULL, !=, message_file_info->file_info.list->revision );
        tt_want_int_op( 0, ==, strcmp( XI_CONTROL_MESSAGE_SFT_GENERATED_REVISION,
                                       message_file_info->file_info.list->revision ) );

        xi_control_message_free( &message_file_info );

    end:;
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__FILE_INFO__three_filenames_and_revisions_provided__valid_output_message_expected,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        const char* filenames[]   = {"filename1", "filename2", "filename3"};
        const uint16_t file_count = sizeof( filenames ) / sizeof( char* );

        xi_control_message_t* message_file_info =
            xi_control_message_create_file_info( filenames, file_count );

        tt_ptr_op( NULL, !=, message_file_info );

        tt_want_int_op( file_count, ==, message_file_info->file_info.list_len );
        tt_ptr_op( NULL, !=, message_file_info->file_info.list );

        uint16_t id_file = 0;
        for ( ; id_file < message_file_info->file_info.list_len; ++id_file )
        {
            tt_ptr_op( NULL, !=, message_file_info->file_info.list[id_file].name );
            tt_want_int_op( 0, ==,
                            strcmp( filenames[id_file],
                                    message_file_info->file_info.list[id_file].name ) );

            tt_ptr_op( NULL, !=, message_file_info->file_info.list[id_file].revision );
            tt_want_int_op(
                0, ==, strcmp( XI_CONTROL_MESSAGE_SFT_GENERATED_REVISION,
                               message_file_info->file_info.list[id_file].revision ) );
        }

        xi_control_message_free( &message_file_info );

    end:;
    } )


/*****************************************************
 * FILE_GET_CHUNK ************************************
 *****************************************************/

XI_TT_TESTCASE_WITH_SETUP( xi_utest__FILE_GET_CHUNK__null_filename__message_not_created,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               xi_control_message_t* message_file_get_chunk =
                                   xi_control_message_create_file_get_chunk(
                                       NULL, "filerevision", 0, 128 );

                               tt_want_ptr_op( NULL, ==, message_file_get_chunk );
                           } )

XI_TT_TESTCASE_WITH_SETUP( xi_utest__FILE_GET_CHUNK__null_revision__message_not_created,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               xi_control_message_t* message_file_get_chunk =
                                   xi_control_message_create_file_get_chunk(
                                       "filename", NULL, 0, 128 );

                               tt_want_ptr_op( NULL, ==, message_file_get_chunk );
                           } )

XI_TT_TESTCASE_WITH_SETUP( xi_utest__FILE_GET_CHUNK__basic,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               xi_control_message_t* message_file_get_chunk =
                                   xi_control_message_create_file_get_chunk(
                                       "filename", "filerevision", 0, 128 );

                               tt_want_ptr_op( NULL, !=, message_file_get_chunk );

                               xi_control_message_free( &message_file_get_chunk );
                           } )

/*****************************************************
 * FILE_STATUS ***************************************
 *****************************************************/

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__FILE_STATUS__basic, xi_utest_setup_basic, xi_utest_teardown_basic, NULL, {
        xi_control_message_t* message_file_status = xi_control_message_create_file_status(
            "name of file", "revision of file",
            XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_PROCESSING,
            XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );

        tt_want_ptr_op( NULL, !=, message_file_status );

        xi_control_message_free( &message_file_status );
    } )

XI_TT_TESTCASE_WITH_SETUP( xi_utest__FILE_STATUS__null_filename__no_message_created,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               xi_control_message_t* message_file_status =
                                   xi_control_message_create_file_status(
                                       NULL, "revision",
                                       XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_FINISHED,
                                       XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );

                               tt_want_ptr_op( NULL, ==, message_file_status );
                           } )

XI_TT_TESTCASE_WITH_SETUP( xi_utest__FILE_STATUS__null_revision__no_message_created,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               xi_control_message_t* message_file_status =
                                   xi_control_message_create_file_status( "name of file",
                                                                          NULL, 11, 22 );

                               tt_want_ptr_op( NULL, ==, message_file_status );
                           } )


/*****************************************************
 * FILE_UPDATE_AVAILABLE get_next_file_desc_ext ******
 *****************************************************/

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__get_next_file_desc_ext__get_1sts_successor,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        xi_control_message_file_desc_ext_t three_file_list[3] = {
            {.name            = "filename 1",
             .revision        = "revision 11",
             .file_operation  = 11,
             .size_in_bytes   = 111,
             .fingerprint     = ( uint8_t* )"fingerprint 111",
             .fingerprint_len = 15},
            {.name            = "filename 2",
             .revision        = "revision 22",
             .file_operation  = 22,
             .size_in_bytes   = 222,
             .fingerprint     = ( uint8_t* )"fingerprint 222",
             .fingerprint_len = 15},
            {.name            = "filename 3",
             .revision        = "revision 33",
             .file_operation  = 33,
             .size_in_bytes   = 333,
             .fingerprint     = ( uint8_t[] ){0x55, 0x56, 0x0, 0x56, 0x55},
             .fingerprint_len = 5}};

        const xi_control_message_t file_update_available = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 3,
                .list     = three_file_list}};

        const xi_control_message_file_desc_ext_t* file_desc =
            xi_control_message_file_update_available_get_next_file_desc_ext(
                &file_update_available.file_update_available, "filename 1" );

        tt_want_str_op( "filename 2", ==, file_desc->name );
        tt_want_ptr_op( &three_file_list[1], ==, file_desc );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__get_next_file_desc_ext__get_2nds_successor,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        xi_control_message_file_desc_ext_t three_file_list[3] = {
            {.name            = "filename 1",
             .revision        = "revision 11",
             .file_operation  = 11,
             .size_in_bytes   = 111,
             .fingerprint     = ( uint8_t* )"fingerprint 111",
             .fingerprint_len = 15},
            {.name            = "filename 2",
             .revision        = "revision 22",
             .file_operation  = 22,
             .size_in_bytes   = 222,
             .fingerprint     = ( uint8_t* )"fingerprint 222",
             .fingerprint_len = 15},
            {.name            = "filename 3",
             .revision        = "revision 33",
             .file_operation  = 33,
             .size_in_bytes   = 333,
             .fingerprint     = ( uint8_t[] ){0x55, 0x56, 0x0, 0x56, 0x55},
             .fingerprint_len = 5}};

        const xi_control_message_t file_update_available = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 3,
                .list     = three_file_list}};

        const xi_control_message_file_desc_ext_t* file_desc =
            xi_control_message_file_update_available_get_next_file_desc_ext(
                &file_update_available.file_update_available, "filename 2" );

        tt_want_str_op( "filename 3", ==, file_desc->name );
        tt_want_ptr_op( &three_file_list[2], ==, file_desc );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__get_next_file_desc_ext__get_lasts_successor,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        xi_control_message_file_desc_ext_t three_file_list[3] = {
            {.name            = "filename 1",
             .revision        = "revision 11",
             .file_operation  = 11,
             .size_in_bytes   = 111,
             .fingerprint     = ( uint8_t* )"fingerprint 111",
             .fingerprint_len = 15},
            {.name            = "filename 2",
             .revision        = "revision 22",
             .file_operation  = 22,
             .size_in_bytes   = 222,
             .fingerprint     = ( uint8_t* )"fingerprint 222",
             .fingerprint_len = 15},
            {.name            = "filename 3",
             .revision        = "revision 33",
             .file_operation  = 33,
             .size_in_bytes   = 333,
             .fingerprint     = ( uint8_t[] ){0x55, 0x56, 0x0, 0x56, 0x55},
             .fingerprint_len = 5}};

        const xi_control_message_t file_update_available = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 3,
                .list     = three_file_list}};

        const xi_control_message_file_desc_ext_t* file_desc =
            xi_control_message_file_update_available_get_next_file_desc_ext(
                &file_update_available.file_update_available, "filename 3" );

        tt_want_ptr_op( NULL, ==, file_desc );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__get_next_file_desc_ext__get_on_null_name,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        xi_control_message_file_desc_ext_t three_file_list[3] = {
            {.name            = "filename 1",
             .revision        = "revision 11",
             .file_operation  = 11,
             .size_in_bytes   = 111,
             .fingerprint     = ( uint8_t* )"fingerprint 111",
             .fingerprint_len = 15},
            {.name            = "filename 2",
             .revision        = "revision 22",
             .file_operation  = 22,
             .size_in_bytes   = 222,
             .fingerprint     = ( uint8_t* )"fingerprint 222",
             .fingerprint_len = 15},
            {.name            = "filename 3",
             .revision        = "revision 33",
             .file_operation  = 33,
             .size_in_bytes   = 333,
             .fingerprint     = ( uint8_t[] ){0x55, 0x56, 0x0, 0x56, 0x55},
             .fingerprint_len = 5}};

        const xi_control_message_t file_update_available = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 3,
                .list     = three_file_list}};

        const xi_control_message_file_desc_ext_t* file_desc =
            xi_control_message_file_update_available_get_next_file_desc_ext(
                &file_update_available.file_update_available, NULL );

        tt_want_ptr_op( NULL, ==, file_desc );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__get_next_file_desc_ext__get_a_non_existing_filename,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        xi_control_message_file_desc_ext_t three_file_list[3] = {
            {.name            = "filename 1",
             .revision        = "revision 11",
             .file_operation  = 11,
             .size_in_bytes   = 111,
             .fingerprint     = ( uint8_t* )"fingerprint 111",
             .fingerprint_len = 15},
            {.name            = "filename 2",
             .revision        = "revision 22",
             .file_operation  = 22,
             .size_in_bytes   = 222,
             .fingerprint     = ( uint8_t* )"fingerprint 222",
             .fingerprint_len = 15},
            {.name            = "filename 3",
             .revision        = "revision 33",
             .file_operation  = 33,
             .size_in_bytes   = 333,
             .fingerprint     = ( uint8_t[] ){0x55, 0x56, 0x0, 0x56, 0x55},
             .fingerprint_len = 5}};

        const xi_control_message_t file_update_available = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 3,
                .list     = three_file_list}};

        const xi_control_message_file_desc_ext_t* file_desc =
            xi_control_message_file_update_available_get_next_file_desc_ext(
                &file_update_available.file_update_available, "non existing filename" );

        tt_want_ptr_op( NULL, ==, file_desc );
    } )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
