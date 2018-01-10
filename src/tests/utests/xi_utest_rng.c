/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"
#include "xi_utest_basic_testcase_frame.h"

#include "xively_error.h"
#include "xi_bsp_rng.h"
#include "xi_macros.h"

#include <stdio.h>
#include <time.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#endif

XI_TT_TESTGROUP_BEGIN( utest_rng )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest_rand, xi_utest_setup_basic, xi_utest_teardown_basic, NULL, {
        const uint32_t r1 = xi_bsp_rng_get();
        const uint32_t r2 = xi_bsp_rng_get();

        /* this fails for every 2^32 time */
        tt_want_int_op( r1, !=, r2 );
    } )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
