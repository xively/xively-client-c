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

#include <xi_control_message.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#endif

XI_TT_TESTGROUP_BEGIN( utest_control_message )


/*****************************************************
 * FILE_INFO *****************************************
 *****************************************************/

XI_TT_TESTCASE_WITH_SETUP( xi_utest__invalid_input__null_output_expected,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               xi_control_message_t* message_file_info =
                                   xi_control_message_create_file_info( NULL, NULL, 0 );

                               tt_want_ptr_op( 0, ==, message_file_info );
                           } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__invalid_input_revision_is_not_enough__null_output_expected,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        const char* single_revision = "revision";

        xi_control_message_t* message_file_info =
            xi_control_message_create_file_info( NULL, &single_revision, 1 );

        tt_want_ptr_op( 0, ==, message_file_info );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__single_filename_provided_but_zero_count__null_output_expected,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        const char* single_filename = "filename";

        xi_control_message_t* message_file_info =
            xi_control_message_create_file_info( &single_filename, NULL, 0 );

        tt_want_ptr_op( 0, ==, message_file_info );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__single_filename_provided__valid_output_message_expected,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        const char* single_filename = "filename";

        xi_control_message_t* message_file_info =
            xi_control_message_create_file_info( &single_filename, NULL, 1 );

        tt_ptr_op( 0, !=, message_file_info );
        tt_want_int_op( XI_CONTROL_MESSAGE_CS_FILE_INFO, ==,
                        message_file_info->common.msgtype );
        tt_want_int_op( 1, ==, message_file_info->common.msgver );


        tt_want_int_op( 1, ==, message_file_info->file_info.list_len );
        tt_ptr_op( NULL, !=, message_file_info->file_info.list );

        tt_ptr_op( NULL, !=, message_file_info->file_info.list->name );
        tt_want_int_op(
            0, ==, strcmp( single_filename, message_file_info->file_info.list->name ) );
        tt_want_ptr_op( NULL, ==, message_file_info->file_info.list->revision );

        xi_control_message_free( &message_file_info );

    end:;
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__single_filename_and_revision_provided__valid_output_message_expected,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        const char* single_filename = "filename";
        const char* single_revision = "revision";

        xi_control_message_t* message_file_info =
            xi_control_message_create_file_info( &single_filename, &single_revision, 1 );

        tt_ptr_op( 0, !=, message_file_info );
        tt_want_int_op( XI_CONTROL_MESSAGE_CS_FILE_INFO, ==,
                        message_file_info->common.msgtype );
        tt_want_int_op( 1, ==, message_file_info->common.msgver );


        tt_want_int_op( 1, ==, message_file_info->file_info.list_len );
        tt_ptr_op( NULL, !=, message_file_info->file_info.list );

        tt_ptr_op( NULL, !=, message_file_info->file_info.list->name );
        tt_want_int_op(
            0, ==, strcmp( single_filename, message_file_info->file_info.list->name ) );

        tt_ptr_op( NULL, !=, message_file_info->file_info.list->revision );
        tt_want_int_op( 0, ==, strcmp( single_revision,
                                       message_file_info->file_info.list->revision ) );

        xi_control_message_free( &message_file_info );

    end:;
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__three_filenames_and_revisions_provided__valid_output_message_expected,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        const char* filenames[]   = {"filename1", "filename2", "filename3"};
        const char* revisions[]   = {"revision1", "revision2", "revision3"};
        const uint16_t file_count = sizeof( filenames ) / sizeof( char* );

        xi_control_message_t* message_file_info =
            xi_control_message_create_file_info( filenames, revisions, file_count );

        tt_ptr_op( 0, !=, message_file_info );

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
                0, ==, strcmp( revisions[id_file],
                               message_file_info->file_info.list[id_file].revision ) );
        }

        xi_control_message_free( &message_file_info );

    end:;
    } )


XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
