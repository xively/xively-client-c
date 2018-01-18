/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"

#include "xi_thread_workerthread.h"

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#endif // XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

XI_TT_TESTGROUP_BEGIN( utest_thread_workerthread )

#include "xi_utest_platform_thread_workerthread.h"

XI_TT_TESTCASE(
    utest__xi_workerthread_create_and_destroy_instance__no_input__not_null_output, {
        struct xi_workerthread_s* new_workerthread_instance =
            xi_workerthread_create_instance( NULL );

        tt_ptr_op( NULL, !=, new_workerthread_instance );

        xi_workerthread_destroy_instance( &new_workerthread_instance );

        tt_ptr_op( NULL, ==, new_workerthread_instance );

    end:
        return;
    } )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif // XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
