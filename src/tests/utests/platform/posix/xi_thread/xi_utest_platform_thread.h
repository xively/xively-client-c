/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_critical_section_def.h"

XI_TT_TESTCASE(
    utest__posix__xi_create_critical_section__valid_input__critical_section_state_eq_zero,
    {
        struct xi_critical_section_s* cs = 0;
        xi_init_critical_section( &cs );

        tt_want_int_op( cs->cs_state, ==, 0 );

        xi_destroy_critical_section( &cs );
    } )

XI_TT_TESTCASE(
    utest__posix__xi_lock_critical_section__valid_input__critical_section_state_eq_one, {
        struct xi_critical_section_s* cs = 0;
        xi_init_critical_section( &cs );

        xi_lock_critical_section( cs );

        tt_want_int_op( cs->cs_state, ==, 1 );

        xi_unlock_critical_section( cs );
        xi_destroy_critical_section( &cs );
    } )

XI_TT_TESTCASE(
    utest__posix__xi_lock_critical_section__valid_input__critical_section_state_eq_zero, {
        struct xi_critical_section_s* cs = 0;
        xi_init_critical_section( &cs );

        xi_lock_critical_section( cs );
        xi_unlock_critical_section( cs );

        tt_want_int_op( cs->cs_state, ==, 0 );

        xi_destroy_critical_section( &cs );
    } )
