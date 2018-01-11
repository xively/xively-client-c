/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

/*
 * Note:  These tests make no demands on platform specific code and are
 * therefore useful for initial testing on a cross-compiled embedded
 * target for the purpose of demonstrate usable linkage to the xibxively library
 * and the functionality of the ported tinytest framework.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tinytest.h"
#include "tinytest_macros.h"

#include "xi_tt_testcase_management.h"
#include "xi_helpers.h"


XI_TT_TESTGROUP_BEGIN( utest_helper_functions )

XI_TT_TESTCASE( utest_helper_bitFilter_boundry_conditions, {
    tt_assert( xi_highest_bit_filter( 0x8000 ) == 0x8000 );
    tt_assert( xi_highest_bit_filter( 0xC000 ) == 0x8000 );
    tt_assert( xi_highest_bit_filter( 0x0800 ) == 0x0800 );
    tt_assert( xi_highest_bit_filter( 0x0C00 ) == 0x0800 );
    tt_assert( xi_highest_bit_filter( 0x0002 ) == 0x0002 );
    tt_assert( xi_highest_bit_filter( 0x0003 ) == 0x0002 );
    tt_assert( xi_highest_bit_filter( 0x8001 ) == 0x8000 );
    tt_assert( xi_highest_bit_filter( 0x0180 ) == 0x0100 );
end:;
} )

XI_TT_TESTCASE( utest_helper_str_copy_untiln, {
    char buf1[80] = "abcdefghijklmnopqrstuvwxyz0123456789";
    char buf2[80];
    int n;
    memset( buf2, 0x0, sizeof( buf2 ) );
    n = xi_str_copy_untiln( buf2, sizeof( buf2 ), buf1, 'k' );
    printf( "n = %d\n", n );
    tt_assert( n == 10 );
    tt_str_op( buf2, ==, "abcdefghij" );
end:;
} )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
