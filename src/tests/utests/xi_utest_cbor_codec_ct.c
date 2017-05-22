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
#include <time.h>
#include <stdint.h>
#include <string.h>

#include <xi_cbor_codec_ct.h>
#include <xi_macros.h>


#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

void bin_to_stdout( const uint8_t* data, uint32_t len )
{
    uint32_t id_byte = 0;
    printf( "\nencoded len: %d\n", len );
    for ( ; id_byte < len; ++id_byte )
    {
        if ( 0 == id_byte % 8 )
        {
            printf( "\n" );
        }

        printf( "0x%.2hhx, ", *data++ );
        // printf( "%.2hhx ", *data++ );
    }
    printf( "\n\n" );
}

#endif

XI_TT_TESTGROUP_BEGIN( utest_cbor_codec_ct )

#if 0
XI_TT_TESTCASE_WITH_SETUP(
    xi_utest_cbor_codec_ct__file_info__empty,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_control_message_t file_info_empty = {
            .file_info = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_DB_FILE_INFO, .msgver = 1},
                .list_len = 0,
                .list     = NULL}};

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_cbor_codec_ct_encode( &file_info_empty, &encoded, &encoded_len );

        // bin_to_stdout( encoded, encoded_len );

        const uint8_t expected_cbor[] = {0xa2, 0x67, 0x6d, 0x73, 0x67, 0x74,
                                         0x79, 0x70, 0x65, 0x00, 0x66, 0x6d,
                                         0x73, 0x67, 0x76, 0x65, 0x72, 0x01};

        tt_want_int_op( 0, ==, memcmp( expected_cbor, encoded, encoded_len ) );

        XI_SAFE_FREE( encoded );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest_cbor_codec_ct__file_info__single_file,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_control_message_file_desc_t file_desc = {.name     = "filename.bin",
                                                    .revision = "rev1"};

        xi_control_message_t file_info_single_file = {
            .file_info = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_DB_FILE_INFO, .msgver = 1},
                .list_len = 1,
                .list     = &file_desc}};

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_cbor_codec_ct_encode( &file_info_single_file, &encoded, &encoded_len );

        // bin_to_stdout( encoded, encoded_len );

        const uint8_t expected_cbor[] = {
            0xa3, 0x67, 0x6d, 0x73, 0x67, 0x74, 0x79, 0x70, 0x65, 0x00, 0x66, 0x6d,
            0x73, 0x67, 0x76, 0x65, 0x72, 0x01, 0x64, 0x6c, 0x69, 0x73, 0x74, 0x81,
            0xa2, 0x61, 0x4e, 0x6c, 0x66, 0x69, 0x6c, 0x65, 0x6e, 0x61, 0x6d, 0x65,
            0x2e, 0x62, 0x69, 0x6e, 0x61, 0x52, 0x64, 0x72, 0x65, 0x76, 0x31};

        tt_want_int_op( 0, ==, memcmp( expected_cbor, encoded, encoded_len ) );

        XI_SAFE_FREE( encoded );

    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest_cbor_codec_ct__file_info__three_files,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_control_message_file_desc_t three_file_desc[3] = {
            {.name = "filename.bin", .revision = "rev4"},
            {.name = "cukroscsibecombcsont", .revision = "rev2"},
            {.name = "txt.hello.bello.txt", .revision = "long_revision_name with space"}};

        xi_control_message_t file_info_single_file = {
            .file_info = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_DB_FILE_INFO, .msgver = 55},
                .list_len = 3,
                .list     = three_file_desc}};

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_cbor_codec_ct_encode( &file_info_single_file, &encoded, &encoded_len );

        // bin_to_stdout( encoded, encoded_len );

        const uint8_t expected_cbor[] = {
            0xa3, 0x67, 0x6d, 0x73, 0x67, 0x74, 0x79, 0x70, 0x65, 0x00, 0x66, 0x6d, 0x73,
            0x67, 0x76, 0x65, 0x72, 0x18, 0x37, 0x64, 0x6c, 0x69, 0x73, 0x74, 0x83, 0xa2,
            0x61, 0x4e, 0x6c, 0x66, 0x69, 0x6c, 0x65, 0x6e, 0x61, 0x6d, 0x65, 0x2e, 0x62,
            0x69, 0x6e, 0x61, 0x52, 0x64, 0x72, 0x65, 0x76, 0x34, 0xa2, 0x61, 0x4e, 0x74,
            0x63, 0x75, 0x6b, 0x72, 0x6f, 0x73, 0x63, 0x73, 0x69, 0x62, 0x65, 0x63, 0x6f,
            0x6d, 0x62, 0x63, 0x73, 0x6f, 0x6e, 0x74, 0x61, 0x52, 0x64, 0x72, 0x65, 0x76,
            0x32, 0xa2, 0x61, 0x4e, 0x73, 0x74, 0x78, 0x74, 0x2e, 0x68, 0x65, 0x6c, 0x6c,
            0x6f, 0x2e, 0x62, 0x65, 0x6c, 0x6c, 0x6f, 0x2e, 0x74, 0x78, 0x74, 0x61, 0x52,
            0x78, 0x1d, 0x6c, 0x6f, 0x6e, 0x67, 0x5f, 0x72, 0x65, 0x76, 0x69, 0x73, 0x69,
            0x6f, 0x6e, 0x5f, 0x6e, 0x61, 0x6d, 0x65, 0x20, 0x77, 0x69, 0x74, 0x68, 0x20,
            0x73, 0x70, 0x61, 0x63, 0x65};

        tt_want_int_op( 0, ==, memcmp( expected_cbor, encoded, encoded_len ) );

        XI_SAFE_FREE( encoded );

    } )

#endif

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
