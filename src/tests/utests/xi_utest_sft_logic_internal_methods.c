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

#include <xi_sft_logic_internal_methods.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif

XI_TT_TESTGROUP_BEGIN( utest_sft_logic_internal_methods )

/*********************************************
 * _xi_sft_send_file_status ******************
 *********************************************/
XI_TT_TESTCASE_WITH_SETUP( xi_utest__send_file_status__null_parameters__no_crash,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           { _xi_sft_send_file_status( NULL, NULL, 0, 0 ); } )

XI_TT_TESTCASE_WITH_SETUP( xi_utest__send_file_status__context_with_null_fields__no_crash,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               xi_sft_context_t sft_context = {
                                   .fn_send_message = ( fn_send_control_message_t )1};

                               _xi_sft_send_file_status( &sft_context, NULL, 0, 0 );
                           } )

/*********************************************
 * _xi_sft_send_file_get_chunk ***************
 *********************************************/
XI_TT_TESTCASE_WITH_SETUP( xi_utest__send_file_get_chunk__null_parameters__no_crash,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           { _xi_sft_send_file_get_chunk( NULL, 0, 0 ); } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__send_file_get_chunk__context_with_null_fields__no_crash,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_sft_context_t sft_context = {.fn_send_message = ( fn_send_control_message_t )1,
                                        .update_current_file = NULL};

        _xi_sft_send_file_get_chunk( &sft_context, 0, 0 );
    } )


/*********************************************
 * _xi_sft_select_next_resource_to_download **
 *********************************************/
XI_TT_TESTCASE_WITH_SETUP( xi_utest__select_next_resource_to_download__null_context,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               const xi_state_t state =
                                   _xi_sft_select_next_resource_to_download( NULL );

                               tt_want_int_op( XI_INVALID_PARAMETER, ==, state );
                           } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__select_next_resource_to_download__context_with_FWU_NULL,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        int32_t download_order[] = {0, 1, 2};
        xi_sft_context_t sft_context;
        sft_context.updateable_files_download_order = download_order;
        sft_context.update_current_file             = NULL;
        sft_context.update_message_fua              = NULL;

        const xi_state_t state = _xi_sft_select_next_resource_to_download( &sft_context );

        tt_want_int_op( XI_STATE_OK, ==, state );
        tt_want_ptr_op( NULL, ==, sft_context.update_current_file );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__select_next_resource_to_download__context_with_FWU_list_NULL,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_control_message_t file_update_available = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 3,
                .list     = NULL}};

        int32_t download_order[] = {0, 1, 2};
        xi_sft_context_t sft_context;
        sft_context.updateable_files_download_order = download_order;
        sft_context.update_current_file             = NULL;
        sft_context.update_message_fua              = &file_update_available;

        xi_state_t state = _xi_sft_select_next_resource_to_download( &sft_context );
        state            = _xi_sft_select_next_resource_to_download( &sft_context );

        tt_want_int_op( XI_STATE_OK, ==, state );
        tt_want_ptr_op( NULL, ==, sft_context.update_current_file );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__select_next_resource_to_download__picks_first_element,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_control_message_file_desc_ext_t three_file_list[3];
        memset( three_file_list, 0, 3 * sizeof( xi_control_message_file_desc_ext_t ) );

        xi_control_message_t file_update_available = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 3,
                .list     = three_file_list}};

        int32_t download_order[] = {0, 1, 2};
        xi_sft_context_t sft_context;
        sft_context.updateable_files_download_order = download_order;
        sft_context.update_current_file             = NULL;
        sft_context.update_message_fua              = &file_update_available;

        xi_state_t state = _xi_sft_select_next_resource_to_download( &sft_context );
        tt_want_int_op( XI_STATE_OK, ==, state );
        tt_want_ptr_op( sft_context.update_current_file, ==, &three_file_list[0] );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__select_next_resource_to_download__negative_index_in_download_order,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        xi_control_message_file_desc_ext_t three_file_list[3];
        memset( three_file_list, 0, 3 * sizeof( xi_control_message_file_desc_ext_t ) );

        xi_control_message_t file_update_available = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 3,
                .list     = three_file_list}};

        int32_t download_order[] = {-1, 1, 2};
        xi_sft_context_t sft_context;
        sft_context.updateable_files_download_order = download_order;
        sft_context.update_current_file             = NULL;
        sft_context.update_message_fua              = &file_update_available;

        xi_state_t state = _xi_sft_select_next_resource_to_download( &sft_context );
        tt_want_int_op( XI_STATE_OK, ==, state );
        tt_want_ptr_op( sft_context.update_current_file, ==, &three_file_list[1] );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__select_next_resource_to_download__picks_last_index_in_download_order,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        xi_control_message_file_desc_ext_t three_file_list[3];
        memset( three_file_list, 0, 3 * sizeof( xi_control_message_file_desc_ext_t ) );

        xi_control_message_t file_update_available = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 3,
                .list     = three_file_list}};

        int32_t download_order[] = {-1, -1, 2};
        xi_sft_context_t sft_context;
        sft_context.updateable_files_download_order = download_order;
        sft_context.update_current_file             = NULL;
        sft_context.update_message_fua              = &file_update_available;

        xi_state_t state = _xi_sft_select_next_resource_to_download( &sft_context );
        tt_want_int_op( XI_STATE_OK, ==, state );
        tt_want_ptr_op( sft_context.update_current_file, ==, &three_file_list[2] );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__select_next_resource_to_download__all_negative_index,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        xi_control_message_file_desc_ext_t three_file_list[3];
        memset( three_file_list, 0, 3 * sizeof( xi_control_message_file_desc_ext_t ) );

        xi_control_message_t file_update_available = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 3,
                .list     = three_file_list}};

        int32_t download_order[] = {-1, -1, -2};
        xi_sft_context_t sft_context;
        sft_context.updateable_files_download_order = download_order;
        sft_context.update_current_file             = NULL;
        sft_context.update_message_fua              = &file_update_available;

        xi_state_t state = _xi_sft_select_next_resource_to_download( &sft_context );
        tt_want_int_op( XI_STATE_OK, ==, state );
        tt_want_ptr_op( NULL, ==, sft_context.update_current_file );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__select_next_resource_to_download__duplicate_index_ok,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        xi_control_message_file_desc_ext_t three_file_list[3];
        memset( three_file_list, 0, 3 * sizeof( xi_control_message_file_desc_ext_t ) );

        xi_control_message_t file_update_available = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 3,
                .list     = three_file_list}};

        int32_t download_order[] = {1, 1, 1};
        xi_sft_context_t sft_context;
        sft_context.updateable_files_download_order = download_order;
        sft_context.update_current_file             = NULL;
        sft_context.update_message_fua              = &file_update_available;

        xi_state_t state = _xi_sft_select_next_resource_to_download( &sft_context );
        tt_want_int_op( XI_STATE_OK, ==, state );
        tt_want_ptr_op( sft_context.update_current_file, ==, &three_file_list[1] );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__select_next_resource_to_download__null_download_order,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        xi_control_message_file_desc_ext_t three_file_list[3];
        memset( three_file_list, 0, 3 * sizeof( xi_control_message_file_desc_ext_t ) );

        xi_control_message_t file_update_available = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 3,
                .list     = three_file_list}};

        xi_sft_context_t sft_context;
        sft_context.updateable_files_download_order = NULL;
        sft_context.update_current_file             = NULL;
        sft_context.update_message_fua              = &file_update_available;

        xi_state_t state = _xi_sft_select_next_resource_to_download( &sft_context );
        tt_want_int_op( XI_INTERNAL_ERROR, ==, state );
        tt_want_ptr_op( sft_context.update_current_file, ==, NULL );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__select_next_resource_to_download__large_number_of_files,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        xi_control_message_file_desc_ext_t large_file_list[128];
        memset( large_file_list, 0, 128 * sizeof( xi_control_message_file_desc_ext_t ) );

        xi_control_message_t file_update_available = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 3,
                .list     = large_file_list}};

        int32_t download_order[128];
        uint16_t i = 0;

        for ( ; i < 128; ++i )
        {
            download_order[i] = i;
        }

        xi_sft_context_t sft_context;
        sft_context.updateable_files_download_order = download_order;
        sft_context.update_current_file             = NULL;
        sft_context.update_message_fua              = &file_update_available;


        xi_state_t state = _xi_sft_select_next_resource_to_download( &sft_context );
        tt_want_int_op( XI_STATE_OK, ==, state );
        tt_want_ptr_op( large_file_list, ==, sft_context.update_current_file );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__select_next_resource_to_download__large_number_of_files_last_index_is_last_file,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        xi_control_message_file_desc_ext_t large_file_list[128];
        memset( large_file_list, 0, 128 * sizeof( xi_control_message_file_desc_ext_t ) );

        xi_control_message_t file_update_available = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 128,
                .list     = large_file_list}};

        int32_t download_order[128];
        uint16_t i = 0;

        for ( ; i < 127; ++i )
        {
            download_order[i] = -1;
        }
        download_order[127] = 127;

        xi_sft_context_t sft_context;
        sft_context.updateable_files_download_order = download_order;
        sft_context.update_current_file             = NULL;
        sft_context.update_message_fua              = &file_update_available;

        xi_state_t state = _xi_sft_select_next_resource_to_download( &sft_context );
        tt_want_int_op( XI_STATE_OK, ==, state );
        tt_want_ptr_op( sft_context.update_current_file, ==, &large_file_list[127] );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__select_next_resource_to_download__large_number_of_files_last_index_is_first_file,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        xi_control_message_file_desc_ext_t large_file_list[128];
        memset( large_file_list, 0, 128 * sizeof( xi_control_message_file_desc_ext_t ) );

        xi_control_message_t file_update_available = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 128,
                .list     = large_file_list}};

        int32_t download_order[128];
        uint16_t i = 0;

        for ( ; i < 127; ++i )
        {
            download_order[i] = -1;
        }

        download_order[127] = 0;

        xi_sft_context_t sft_context;
        sft_context.updateable_files_download_order = download_order;
        sft_context.update_current_file             = NULL;
        sft_context.update_message_fua              = &file_update_available;

        xi_state_t state = _xi_sft_select_next_resource_to_download( &sft_context );
        tt_want_int_op( XI_STATE_OK, ==, state );
        tt_want_ptr_op( sft_context.update_current_file, ==, &large_file_list[0] );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__select_next_resource_to_download__large_number_of_files__deafult_order__check_full_list,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        xi_control_message_file_desc_ext_t large_file_list[128];
        memset( large_file_list, 0, 128 * sizeof( xi_control_message_file_desc_ext_t ) );

        xi_control_message_t file_update_available = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 128,
                .list     = large_file_list}};

        int32_t download_order[128];
        uint16_t i = 0;

        for ( ; i < 127; ++i )
        {
            download_order[i] = i;
        }

        xi_sft_context_t sft_context;
        sft_context.updateable_files_download_order = download_order;
        sft_context.update_current_file             = NULL;
        sft_context.update_message_fua              = &file_update_available;

        for ( i = 0; i < 127; ++i )
        {
            xi_state_t state = _xi_sft_select_next_resource_to_download( &sft_context );
            tt_want_int_op( XI_STATE_OK, ==, state );
            tt_want_ptr_op( sft_context.update_current_file, ==, &large_file_list[i] );
        }
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__select_next_resource_to_download__large_number_of_files__check_full_list__reverse_order,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        xi_control_message_file_desc_ext_t large_file_list[128];
        memset( large_file_list, 0, 128 * sizeof( xi_control_message_file_desc_ext_t ) );

        xi_control_message_t file_update_available = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 128,
                .list     = large_file_list}};

        int32_t download_order[128];
        uint16_t i = 0;

        for ( ; i < 127; ++i )
        {
            download_order[i] = 127 - i;
        }

        xi_sft_context_t sft_context;
        sft_context.updateable_files_download_order = download_order;
        sft_context.update_current_file             = NULL;
        sft_context.update_message_fua              = &file_update_available;

        for ( i = 0; i < 127; ++i )
        {
            xi_state_t state = _xi_sft_select_next_resource_to_download( &sft_context );
            tt_want_int_op( XI_STATE_OK, ==, state );
            tt_want_ptr_op( sft_context.update_current_file, ==,
                            &large_file_list[127 - i] );
        }
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__select_next_resource_to_download__large_number_of_files__check_full_list__center_out,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        xi_control_message_file_desc_ext_t large_file_list[128];
        memset( large_file_list, 0, 128 * sizeof( xi_control_message_file_desc_ext_t ) );

        xi_control_message_t file_update_available = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 128,
                .list     = large_file_list}};

        int32_t download_order[128];
        uint16_t i = 0;

        for ( ; i < 127; ++i )
        {
            // download_order[i] = (i < 64) ? 126 - 2 * i : 255 - 2 * i;
            download_order[i] = ( i < 64 ) ? 126 - 2 * i : 2 * i - 127;
        }

        xi_sft_context_t sft_context;
        sft_context.updateable_files_download_order = download_order;
        sft_context.update_current_file             = NULL;
        sft_context.update_message_fua              = &file_update_available;

        for ( i = 0; i < 127; ++i )
        {
            uint16_t expected_index = ( i < 64 ) ? 126 - 2 * i : 2 * i - 127;
            xi_state_t state = _xi_sft_select_next_resource_to_download( &sft_context );
            tt_want_int_op( XI_STATE_OK, ==, state );
            tt_want_ptr_op( sft_context.update_current_file, ==,
                            &large_file_list[expected_index] );
        }
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__select_next_resource_to_download__large_number_of_files__odds_evens__check_full_list,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        xi_control_message_file_desc_ext_t large_file_list[128];
        memset( large_file_list, 0, 128 * sizeof( xi_control_message_file_desc_ext_t ) );

        xi_control_message_t file_update_available = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 128,
                .list     = large_file_list}};

        int32_t download_order[128];
        uint16_t i = 0;

        for ( ; i < 127; ++i )
        {
            download_order[i] = ( i < 64 ) ? 126 - 2 * i : 255 - 2 * i;
        }

        xi_sft_context_t sft_context;
        sft_context.updateable_files_download_order = download_order;
        sft_context.update_current_file             = NULL;
        sft_context.update_message_fua              = &file_update_available;

        for ( i = 0; i < 127; ++i )
        {
            uint16_t expected_index = ( i < 64 ) ? 126 - 2 * i : 255 - 2 * i;
            xi_state_t state = _xi_sft_select_next_resource_to_download( &sft_context );
            tt_want_int_op( XI_STATE_OK, ==, state );
            tt_want_ptr_op( sft_context.update_current_file, ==,
                            &large_file_list[expected_index] );
        }
    } )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
