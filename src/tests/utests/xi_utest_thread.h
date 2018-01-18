/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"

#include "xively.h"
#include "xi_critical_section.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#endif // XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

XI_TT_TESTGROUP_BEGIN( utest_thread )

#include "xi_utest_platform_thread.h"

XI_TT_TESTCASE( utest__xi_init_critical_section_valid_input__create_critical_section, {
    struct xi_critical_section_s* cs = 0;
    xi_state_t state                 = xi_init_critical_section( &cs );

    tt_want_int_op( XI_STATE_OK, ==, state );
    tt_want_ptr_op( NULL, !=, cs );

    xi_destroy_critical_section( &cs );
} )

XI_TT_TESTCASE(
    utest__xi_destroy_critical_section_valid_input__critical_section_freed_nullyfied, {
        struct xi_critical_section_s* cs = 0;
        xi_init_critical_section( &cs );

        xi_destroy_critical_section( &cs );

        tt_want_ptr_op( NULL, ==, cs );
    } )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif // XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
