/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"
#include "xi_utest_basic_testcase_frame.h"

#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

#include <xi_bsp_crypt.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

void print_hash( unsigned char( hash )[32] )
{
    ( void )hash;

    int idx;
    for ( idx = 0; idx < 32; idx++ )
        printf( "%02x", hash[idx] );
    printf( "\n" );
}

#endif

XI_TT_TESTGROUP_BEGIN( utest_sha256 )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest_sha256_basic, xi_utest_setup_basic, xi_utest_teardown_basic, NULL, {

        void* sha = NULL;

        xi_bsp_crypt_sha256_init( &sha );

        const uint8_t data[2] = {0xab, 0xcd};

        xi_bsp_crypt_sha256_update( sha, data, 2 );

        uint8_t out[32];
        xi_bsp_crypt_sha256_final( sha, out );

        uint8_t desired_hash[32] = {0x12, 0x3d, 0x4c, 0x7e, 0xf2, 0xd1, 0x60, 0x0a,
                                    0x1b, 0x3a, 0x0f, 0x6a, 0xdd, 0xc6, 0x0a, 0x10,
                                    0xf0, 0x5a, 0x34, 0x95, 0xc9, 0x40, 0x9f, 0x2e,
                                    0xcb, 0xf4, 0xcc, 0x09, 0x5d, 0x00, 0x0a, 0x6b};

        // print_hash( out );
        // print_hash( desired_hash );

        const int result_memcmp = memcmp( out, desired_hash, 32 );

        tt_want_int_op( 0, ==, result_memcmp );
    } )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
