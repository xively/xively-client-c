/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_utest_basic_testcase_frame.h"
#include "xively.h"
#include "tinytest_macros.h"
#include "xi_memory_checks.h"
#include "xi_macros.h"
#include <stdio.h>

void* xi_utest_setup_basic( const struct testcase_t* testcase )
{
    XI_UNUSED( testcase );

    xi_memory_limiter_tearup();

    xi_initialize( "utest-account-id" );

    return ( intptr_t* )1;
}

int xi_utest_teardown_basic( const struct testcase_t* testcase, void* fixture )
{
    XI_UNUSED( testcase );
    XI_UNUSED( fixture );

    xi_shutdown();

    // 1 - OK, 0 - NOT OK
    return xi_memory_limiter_teardown();
}
