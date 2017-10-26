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

#include <xi_sft_logic.h>
#include <xi_macros.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#endif

XI_TT_TESTGROUP_BEGIN( utest_sft_logic )


XI_TT_TESTCASE_WITH_SETUP( xi_utest__make_context__invalid_paramter,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               const xi_state_t state =
                                   xi_sft_make_context( NULL, NULL, 0, NULL, NULL );

                               tt_want_ptr_op( XI_INVALID_PARAMETER, ==, state );
                           } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest__make_context__minimal_config__no_leak_after_deallocation,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_sft_context_t* sft_context = NULL;

        xi_state_t state = xi_sft_make_context( &sft_context, NULL, 0, NULL, NULL );

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

                               xi_sft_make_context( &sft_context, NULL, 0, NULL, NULL );

                               xi_sft_on_connected( sft_context );

                               xi_sft_free_context( &sft_context );
                           } )

XI_TT_TESTCASE_WITH_SETUP( xi_utest__minimal_config__call_on_connection_failed__no_crash,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               xi_sft_context_t* sft_context = NULL;

                               xi_sft_make_context( &sft_context, NULL, 0, NULL, NULL );

                               xi_sft_on_connection_failed( sft_context );

                               xi_sft_free_context( &sft_context );
                           } )

XI_TT_TESTCASE_WITH_SETUP( xi_utest__minimal_config__call_on_message_with_NULL__no_crash,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               xi_sft_context_t* sft_context = NULL;

                               xi_sft_make_context( &sft_context, NULL, 0, NULL, NULL );

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

        xi_sft_make_context( &sft_context, NULL, 0, NULL, NULL );

        xi_state_t state = XI_STATE_OK;

        XI_ALLOC( xi_control_message_t, message_uninitialized, state );

        xi_sft_on_message( sft_context, message_uninitialized );

    err_handling:

        xi_sft_free_context( &sft_context );
    } )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif