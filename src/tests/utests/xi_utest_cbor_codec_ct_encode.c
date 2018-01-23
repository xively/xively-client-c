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
#include <time.h>
#include <stdint.h>
#include <string.h>

#include <xi_cbor_codec_ct.h>
#include <xi_control_message_sft.h>
#include <xi_macros.h>


#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

void xi_utest_cbor_bin_to_stdout( const uint8_t* data,
                                  uint32_t len,
                                  uint8_t hex_output_type )
{
    uint32_t id_byte = 0;
    printf( "\nencoded len: %d\n", len );
    for ( ; id_byte < len; ++id_byte )
    {
        if ( 0 == id_byte % 8 )
        {
            printf( "\n" );
        }

        hex_output_type ? printf( "0x%.2hhx, ", *data++ ) : printf( "%.2hhx ", *data++ );
    }
    printf( "\n\n" );
}

#endif

XI_TT_TESTGROUP_BEGIN( utest_cbor_codec_ct_encode )


/*****************************************************
 * FILE_INFO *****************************************
 *****************************************************/

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest_cbor_codec_ct_encode__file_info__empty,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        const xi_control_message_t file_info_empty = {
            .file_info = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO, .msgver = 1},
                .list_len = 0,
                .list     = NULL}};

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_cbor_codec_ct_encode( &file_info_empty, &encoded, &encoded_len );

        // xi_utest_cbor_bin_to_stdout( encoded, encoded_len );

        const uint8_t expected_cbor[] = {0xa2, 0x67, 0x6d, 0x73, 0x67, 0x74,
                                         0x79, 0x70, 0x65, 0x00, 0x66, 0x6d,
                                         0x73, 0x67, 0x76, 0x65, 0x72, 0x01};

        tt_want_int_op( 0, ==, memcmp( expected_cbor, encoded, encoded_len ) );
        tt_want_int_op( sizeof( expected_cbor ), ==, encoded_len );

        XI_SAFE_FREE( encoded );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest_cbor_codec_ct_encode__file_info__invalid_name_and_revision,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_control_message_file_desc_t file_desc = {.name = NULL, .revision = NULL};

        const xi_control_message_t file_info_single_file = {
            .file_info = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO, .msgver = 1},
                .list_len                  = 1,
                .list                      = &file_desc,
                .flag_accept_download_link = 1}};

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_cbor_codec_ct_encode( &file_info_single_file, &encoded, &encoded_len );

        // xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 1 );

        /* {"msgtype":0,"msgver":1,"list":[{}]} */
        const uint8_t expected_cbor[] = {0xa4, 0x67, 0x6d, 0x73, 0x67, 0x74, 0x79,
                                         0x70, 0x65, 0x00, 0x66, 0x6d, 0x73, 0x67,
                                         0x76, 0x65, 0x72, 0x01, 0x64, 0x6c, 0x69,
                                         0x73, 0x74, 0x81, 0xa0, 0x61, 0x4c, 0xf5};

        tt_want_int_op( 0, ==, memcmp( expected_cbor, encoded, encoded_len ) );
        tt_want_int_op( sizeof( expected_cbor ), ==, encoded_len );

        XI_SAFE_FREE( encoded );

    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest_cbor_codec_ct_encode__file_info__single_file,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_control_message_file_desc_t file_desc = {.name     = "filename.bin",
                                                    .revision = "rev1"};

        const xi_control_message_t file_info_single_file = {
            .file_info = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO, .msgver = 1},
                .list_len                  = 1,
                .list                      = &file_desc,
                .flag_accept_download_link = 0}};

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_cbor_codec_ct_encode( &file_info_single_file, &encoded, &encoded_len );

        // xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 1 );

        const uint8_t expected_cbor[] = {
            0xa4, 0x67, 0x6d, 0x73, 0x67, 0x74, 0x79, 0x70, 0x65, 0x00, 0x66, 0x6d, 0x73,
            0x67, 0x76, 0x65, 0x72, 0x01, 0x64, 0x6c, 0x69, 0x73, 0x74, 0x81, 0xa2, 0x61,
            0x4e, 0x6c, 0x66, 0x69, 0x6c, 0x65, 0x6e, 0x61, 0x6d, 0x65, 0x2e, 0x62, 0x69,
            0x6e, 0x61, 0x52, 0x64, 0x72, 0x65, 0x76, 0x31, 0x61, 0x4c, 0xf4};

        tt_want_int_op( 0, ==, memcmp( expected_cbor, encoded, encoded_len ) );
        tt_want_int_op( sizeof( expected_cbor ), ==, encoded_len );

        XI_SAFE_FREE( encoded );

    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest_cbor_codec_ct_encode__file_info__three_files,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_control_message_file_desc_t three_file_desc[3] = {
            {.name = "filename.bin", .revision = "rev4"},
            {.name = "cukroscsibecombcsont", .revision = "rev2"},
            {.name = "txt.hello.bello.txt", .revision = "long_revision_name with space"}};

        const xi_control_message_t file_info_three_files = {
            .file_info = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO, .msgver = 55},
                .list_len                  = 3,
                .list                      = three_file_desc,
                .flag_accept_download_link = 1}};

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_cbor_codec_ct_encode( &file_info_three_files, &encoded, &encoded_len );

        // xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 1 );

        /*
         * To decode this use tinycbor (https://github.com/01org/tinycbor) cbordump
         * executable on a binary file containing the bytes below. This will emit a human
         * readable json you can run eye-o-scopic analysis against ;).
         *
         * {
         *  "msgtype":0,
         *  "msgver":55,
         *  "list":[
         *      {"N":"filename.bin","R":"rev4"},
         *      {"N":"cukroscsibecombcsont","R":"rev2"},
         *      {"N":"txt.hello.bello.txt","R":"long_revision_name with space"}
         *      ]
         * }
         */
        const uint8_t expected_cbor[] = {
            0xa4, 0x67, 0x6d, 0x73, 0x67, 0x74, 0x79, 0x70, 0x65, 0x00, 0x66, 0x6d, 0x73,
            0x67, 0x76, 0x65, 0x72, 0x18, 0x37, 0x64, 0x6c, 0x69, 0x73, 0x74, 0x83, 0xa2,
            0x61, 0x4e, 0x6c, 0x66, 0x69, 0x6c, 0x65, 0x6e, 0x61, 0x6d, 0x65, 0x2e, 0x62,
            0x69, 0x6e, 0x61, 0x52, 0x64, 0x72, 0x65, 0x76, 0x34, 0xa2, 0x61, 0x4e, 0x74,
            0x63, 0x75, 0x6b, 0x72, 0x6f, 0x73, 0x63, 0x73, 0x69, 0x62, 0x65, 0x63, 0x6f,
            0x6d, 0x62, 0x63, 0x73, 0x6f, 0x6e, 0x74, 0x61, 0x52, 0x64, 0x72, 0x65, 0x76,
            0x32, 0xa2, 0x61, 0x4e, 0x73, 0x74, 0x78, 0x74, 0x2e, 0x68, 0x65, 0x6c, 0x6c,
            0x6f, 0x2e, 0x62, 0x65, 0x6c, 0x6c, 0x6f, 0x2e, 0x74, 0x78, 0x74, 0x61, 0x52,
            0x78, 0x1d, 0x6c, 0x6f, 0x6e, 0x67, 0x5f, 0x72, 0x65, 0x76, 0x69, 0x73, 0x69,
            0x6f, 0x6e, 0x5f, 0x6e, 0x61, 0x6d, 0x65, 0x20, 0x77, 0x69, 0x74, 0x68, 0x20,
            0x73, 0x70, 0x61, 0x63, 0x65, 0x61, 0x4c, 0xf5};

        tt_want_int_op( 0, ==, memcmp( expected_cbor, encoded, encoded_len ) );
        tt_want_int_op( sizeof( expected_cbor ), ==, encoded_len );

        XI_SAFE_FREE( encoded );

    } )


/*****************************************************
 * FILE_GET_CHUNK ************************************
 *****************************************************/

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest_cbor_codec_ct_encode__file_get_chunk__basic,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        const xi_control_message_t file_get_chunk = {
            .file_get_chunk = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK,
                           .msgver  = 1},
                .name     = "givemethischunkboy",
                .revision = "theperfect revision please 123",
                .offset   = 11,
                .length   = 888}};

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_cbor_codec_ct_encode( &file_get_chunk, &encoded, &encoded_len );

        // xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 0 );
        // xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 1 );

        /* {"msgtype":2,"msgver":1,"N":"givemethischunkboy","R":"theperfect revision
         * please 123","O":11,"L":888} */
        const uint8_t expected_cbor[] = {
            0xa6, 0x67, 0x6d, 0x73, 0x67, 0x74, 0x79, 0x70, 0x65, 0x02, 0x66, 0x6d,
            0x73, 0x67, 0x76, 0x65, 0x72, 0x01, 0x61, 0x4e, 0x72, 0x67, 0x69, 0x76,
            0x65, 0x6d, 0x65, 0x74, 0x68, 0x69, 0x73, 0x63, 0x68, 0x75, 0x6e, 0x6b,
            0x62, 0x6f, 0x79, 0x61, 0x52, 0x78, 0x1e, 0x74, 0x68, 0x65, 0x70, 0x65,
            0x72, 0x66, 0x65, 0x63, 0x74, 0x20, 0x72, 0x65, 0x76, 0x69, 0x73, 0x69,
            0x6f, 0x6e, 0x20, 0x70, 0x6c, 0x65, 0x61, 0x73, 0x65, 0x20, 0x31, 0x32,
            0x33, 0x61, 0x4f, 0x0b, 0x61, 0x4c, 0x19, 0x03, 0x78};

        tt_want_int_op( 0, ==, memcmp( expected_cbor, encoded, encoded_len ) );
        tt_want_int_op( sizeof( expected_cbor ), ==, encoded_len );

        XI_SAFE_FREE( encoded );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest_cbor_codec_ct_encode__file_get_chunk__zero_length_and_offset,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        const xi_control_message_t file_get_chunk = {
            .file_get_chunk = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK,
                           .msgver  = 123},
                .name     = "zero",
                .revision = "zero 999",
                .offset   = 0,
                .length   = 0}};

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_cbor_codec_ct_encode( &file_get_chunk, &encoded, &encoded_len );

        // xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 0 );
        // xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 1 );

        /* {"msgtype":2,"msgver":123,"N":"zero","R":"zero 999","O":0,"L":0} */
        const uint8_t expected_cbor[] = {
            0xa6, 0x67, 0x6d, 0x73, 0x67, 0x74, 0x79, 0x70, 0x65, 0x02, 0x66,
            0x6d, 0x73, 0x67, 0x76, 0x65, 0x72, 0x18, 0x7b, 0x61, 0x4e, 0x64,
            0x7a, 0x65, 0x72, 0x6f, 0x61, 0x52, 0x68, 0x7a, 0x65, 0x72, 0x6f,
            0x20, 0x39, 0x39, 0x39, 0x61, 0x4f, 0x00, 0x61, 0x4c, 0x00};

        tt_want_int_op( 0, ==, memcmp( expected_cbor, encoded, encoded_len ) );
        tt_want_int_op( sizeof( expected_cbor ), ==, encoded_len );

        XI_SAFE_FREE( encoded );
    } )


/*****************************************************
 * FILE_STATUS ***************************************
 *****************************************************/

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest_cbor_codec_ct_encode__file_status__basic,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_control_message_t* sft_message = xi_control_message_create_file_status(
            "name for FILE_STATUS message", "revision for FILE_STATUS message", 77, 99 );

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_cbor_codec_ct_encode( sft_message, &encoded, &encoded_len );

        const uint8_t expected_cbor[] = {
            0xa6, 0x67, 0x6d, 0x73, 0x67, 0x74, 0x79, 0x70, 0x65, 0x04, 0x66, 0x6d,
            0x73, 0x67, 0x76, 0x65, 0x72, 0x01, 0x61, 0x4e, 0x78, 0x1c, 0x6e, 0x61,
            0x6d, 0x65, 0x20, 0x66, 0x6f, 0x72, 0x20, 0x46, 0x49, 0x4c, 0x45, 0x5f,
            0x53, 0x54, 0x41, 0x54, 0x55, 0x53, 0x20, 0x6d, 0x65, 0x73, 0x73, 0x61,
            0x67, 0x65, 0x61, 0x52, 0x78, 0x20, 0x72, 0x65, 0x76, 0x69, 0x73, 0x69,
            0x6f, 0x6e, 0x20, 0x66, 0x6f, 0x72, 0x20, 0x46, 0x49, 0x4c, 0x45, 0x5f,
            0x53, 0x54, 0x41, 0x54, 0x55, 0x53, 0x20, 0x6d, 0x65, 0x73, 0x73, 0x61,
            0x67, 0x65, 0x61, 0x50, 0x18, 0x4d, 0x61, 0x53, 0x18, 0x63};

        tt_want_int_op( 0, ==, memcmp( expected_cbor, encoded, encoded_len ) );
        tt_want_int_op( sizeof( expected_cbor ), ==, encoded_len );

        xi_control_message_free( &sft_message );
        XI_SAFE_FREE( encoded );
    } )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
