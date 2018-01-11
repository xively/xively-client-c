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
#include <xi_control_message.h>
#include <xi_macros.h>

#include <cbor.h>
#include <cn-cbor/cn-cbor.h>

#include <xi_debug.h>


#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#ifdef USE_CBOR_CONTEXT
extern cn_cbor_context* context_cbor;
#endif

void xi_utest_cbor_bin_to_stdout( const uint8_t* data,
                                  uint32_t len,
                                  uint8_t hex_output_type );

/* using the common server CBOR encoder, shared usage with the itests' mock broker */
void xi_cbor_codec_ct_server_encode( const xi_control_message_t* control_message,
                                     uint8_t** out_encoded_allocated_inside,
                                     uint32_t* out_len );

void xi_utest_cbor_ASSERT_control_message_string( const char* str1, const char* str2 )
{
    if ( str1 && str2 )
    {
        tt_want_int_op( 0, ==, strcmp( str1, str2 ) );
    }
    else
    {
        // both have to be NULL here
        tt_want_int_op( str1, ==, str2 );
    }
}

void xi_utest_cbor_ASSERT_control_messages_match( const xi_control_message_t* cm1,
                                                  const xi_control_message_t* cm2 )
{
    tt_ptr_op( NULL, !=, cm1 );
    tt_ptr_op( NULL, !=, cm2 );

    tt_want_int_op( cm1->common.msgtype, ==, cm2->common.msgtype );
    tt_want_int_op( cm1->common.msgver, ==, cm2->common.msgver );

    switch ( cm1->common.msgtype )
    {
        case XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE:
            tt_int_op( cm1->file_update_available.list_len, ==,
                       cm2->file_update_available.list_len );

            uint8_t id_file = 0;
            for ( ; id_file < cm1->file_update_available.list_len; ++id_file )
            {
                xi_utest_cbor_ASSERT_control_message_string(
                    cm1->file_update_available.list[id_file].name,
                    cm2->file_update_available.list[id_file].name );

                xi_utest_cbor_ASSERT_control_message_string(
                    cm1->file_update_available.list[id_file].revision,
                    cm2->file_update_available.list[id_file].revision );

                tt_want_int_op( cm1->file_update_available.list[id_file].file_operation,
                                ==,
                                cm2->file_update_available.list[id_file].file_operation );

                tt_want_int_op( cm1->file_update_available.list[id_file].size_in_bytes,
                                ==,
                                cm2->file_update_available.list[id_file].size_in_bytes );

                tt_int_op( cm1->file_update_available.list[id_file].fingerprint_len, ==,
                           cm2->file_update_available.list[id_file].fingerprint_len );

                tt_want_int_op(
                    0, ==,
                    memcmp( cm1->file_update_available.list[id_file].fingerprint,
                            cm2->file_update_available.list[id_file].fingerprint,
                            cm1->file_update_available.list[id_file].fingerprint_len ) );

                xi_utest_cbor_ASSERT_control_message_string(
                    cm1->file_update_available.list[id_file].download_link,
                    cm2->file_update_available.list[id_file].download_link );

                tt_int_op( cm1->file_update_available.list[id_file]
                               .flag_mqtt_download_also_supported,
                           ==, cm2->file_update_available.list[id_file]
                                   .flag_mqtt_download_also_supported );
            }

            break;

        case XI_CONTROL_MESSAGE_SC__SFT_FILE_CHUNK:

            xi_utest_cbor_ASSERT_control_message_string( cm1->file_chunk.name,
                                                         cm2->file_chunk.name );

            xi_utest_cbor_ASSERT_control_message_string( cm1->file_chunk.revision,
                                                         cm2->file_chunk.revision );

            tt_want_int_op( cm1->file_chunk.offset, ==, cm2->file_chunk.offset );
            tt_want_int_op( cm1->file_chunk.length, ==, cm2->file_chunk.length );
            tt_want_int_op( cm1->file_chunk.status, ==, cm2->file_chunk.status );

            tt_want_int_op( 0, ==, memcmp( cm1->file_chunk.chunk, cm2->file_chunk.chunk,
                                           cm1->file_chunk.length ) );

            break;

        default:;
    }
end:;
}

#endif

XI_TT_TESTGROUP_BEGIN( utest_cbor_codec_ct_decode )


/*****************************************************
 * XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE **
 *****************************************************/

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest_cbor_codec_ct_decode__file_update_available__empty_list,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        const xi_control_message_t file_update_available_in = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 0,
                .list     = NULL}};

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_cbor_codec_ct_server_encode( &file_update_available_in, &encoded,
                                        &encoded_len );

        // xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 0 );
        // xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 1 );

        // ACT
        xi_control_message_t* file_update_available_out =
            xi_cbor_codec_ct_decode( encoded, encoded_len );

        // ASSERT
        xi_utest_cbor_ASSERT_control_messages_match( &file_update_available_in,
                                                     file_update_available_out );

        XI_SAFE_FREE( encoded );
        xi_control_message_free( &file_update_available_out );

    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest_cbor_codec_ct_decode__file_update_available__single_file,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        xi_control_message_file_desc_ext_t single_file_list[1] = {
            {.name            = "first_SFT_test.cfg",
             .revision        = "1",
             .file_operation  = 0,
             .size_in_bytes   = 109489,
             .fingerprint     = ( uint8_t* )"first_SFT_test_artificial_checksum.cfg",
             .fingerprint_len = 39,
             .download_link   = "hello I am the download link",
             .flag_mqtt_download_also_supported = 0}};

        const xi_control_message_t file_update_available_in = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 1,
                .list     = single_file_list}};

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_cbor_codec_ct_server_encode( &file_update_available_in, &encoded,
                                        &encoded_len );

        // xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 0 );
        // xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 1 );

        // ACT
        xi_control_message_t* file_update_available_out =
            xi_cbor_codec_ct_decode( encoded, encoded_len );

        // xi_debug_control_message_dump( &file_update_available_in, "in" );
        // xi_debug_control_message_dump( file_update_available_out, "out" );

        // ASSERT
        xi_utest_cbor_ASSERT_control_messages_match( &file_update_available_in,
                                                     file_update_available_out );

        XI_SAFE_FREE( encoded );
        xi_control_message_free( &file_update_available_out );

    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest_cbor_codec_ct_decode__file_update_available_with_download_link__single_file,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        xi_control_message_file_desc_ext_t single_file_list[1] = {
            {.name            = "first_SFT_test.cfg",
             .revision        = "1",
             .file_operation  = 0,
             .size_in_bytes   = 109489,
             .fingerprint     = ( uint8_t* )"first_SFT_test_artificial_checksum.cfg",
             .fingerprint_len = 39,
             .download_link   = "hello I am the download link",
             .flag_mqtt_download_also_supported = 1}};

        const xi_control_message_t file_update_available_in = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 1,
                .list     = single_file_list}};

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_cbor_codec_ct_server_encode( &file_update_available_in, &encoded,
                                        &encoded_len );

        // xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 0 );
        // xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 1 );

        // ACT
        xi_control_message_t* file_update_available_out =
            xi_cbor_codec_ct_decode( encoded, encoded_len );

        // xi_debug_control_message_dump( &file_update_available_in, "in" );
        // xi_debug_control_message_dump( file_update_available_out, "out" );

        // ASSERT
        xi_utest_cbor_ASSERT_control_messages_match( &file_update_available_in,
                                                     file_update_available_out );

        XI_SAFE_FREE( encoded );
        xi_control_message_free( &file_update_available_out );

    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest_cbor_codec_ct_decode__file_update_available__single_file_null_values,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        xi_control_message_file_desc_ext_t single_file_list[1] = {{.name           = NULL,
                                                                   .revision       = NULL,
                                                                   .file_operation = 0,
                                                                   .size_in_bytes  = 10,
                                                                   .fingerprint    = NULL,
                                                                   .fingerprint_len = 0}};

        const xi_control_message_t file_update_available_in = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 1,
                .list     = single_file_list}};

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_cbor_codec_ct_server_encode( &file_update_available_in, &encoded,
                                        &encoded_len );

        // xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 0 );
        // xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 1 );

        // ACT
        xi_control_message_t* file_update_available_out =
            xi_cbor_codec_ct_decode( encoded, encoded_len );

        // ASSERT
        xi_utest_cbor_ASSERT_control_messages_match( &file_update_available_in,
                                                     file_update_available_out );

        XI_SAFE_FREE( encoded );
        xi_control_message_free( &file_update_available_out );

    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest_cbor_codec_ct_decode__file_update_available__three_files,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        xi_control_message_file_desc_ext_t three_file_list[3] = {
            {.name            = "filename 1",
             .revision        = "revision 11",
             .file_operation  = 11,
             .size_in_bytes   = 111,
             .fingerprint     = ( uint8_t* )"fingerprint 111",
             .fingerprint_len = 15},
            {.name            = "filename 2",
             .revision        = "revision 22",
             .file_operation  = 22,
             .size_in_bytes   = 222,
             .fingerprint     = ( uint8_t* )"fingerprint 222",
             .fingerprint_len = 15},
            {.name            = "filename 3",
             .revision        = "revision 33",
             .file_operation  = 33,
             .size_in_bytes   = 333,
             .fingerprint     = ( uint8_t[] ){0x55, 0x56, 0x0, 0x56, 0x55},
             .fingerprint_len = 5}};

        const xi_control_message_t file_update_available_in = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 3,
                .list     = three_file_list}};

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_cbor_codec_ct_server_encode( &file_update_available_in, &encoded,
                                        &encoded_len );

        // xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 0 );
        // xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 1 );

        // ACT
        xi_control_message_t* file_update_available_out =
            xi_cbor_codec_ct_decode( encoded, encoded_len );

        // ASSERT
        xi_utest_cbor_ASSERT_control_messages_match( &file_update_available_in,
                                                     file_update_available_out );

        XI_SAFE_FREE( encoded );
        xi_control_message_free( &file_update_available_out );

    } )


/*****************************************************
 * XI_CONTROL_MESSAGE_SC__SFT_FILE_CHUNK *************
 *****************************************************/

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest_cbor_codec_ct_decode__file_chunk__basic,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        const xi_control_message_t file_chunk_in = {
            .file_chunk = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_CHUNK, .msgver = 1},
                .name     = "filename for filechunk message",
                .revision = "revision for filechunk message",
                .offset   = 0,
                .length   = 15, /* including terminating zero */
                .status   = 77,
                .chunk    = ( uint8_t* )"my chunk hello"}};

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_cbor_codec_ct_server_encode( &file_chunk_in, &encoded, &encoded_len );

        // xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 0 );
        // xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 1 );

        // ACT
        xi_control_message_t* file_chunk_out =
            xi_cbor_codec_ct_decode( encoded, encoded_len );

        // ASSERT
        xi_utest_cbor_ASSERT_control_messages_match( &file_chunk_in, file_chunk_out );

        XI_SAFE_FREE( encoded );
        xi_control_message_free( &file_chunk_out );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest_cbor_codec_ct_decode__file_chunk__binary_chunk,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        const xi_control_message_t file_chunk_in = {
            .file_chunk = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_CHUNK,
                           .msgver  = -1},
                .name     = "filename for binary chunk !@#Q#$DFVS@#$   \t sdfgsdf",
                .revision = NULL,
                .offset   = 555,
                .length   = 15,
                .status   = 33,
                .chunk    = ( uint8_t[] ){1, 2, 3, 4, 3, 2, 1, 0, 0, 1, 0, 1, 0, 5, 5}}};

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_cbor_codec_ct_server_encode( &file_chunk_in, &encoded, &encoded_len );

        // xi_utest_cbor_bin_to_stdout( encoded, ehncoded_len, 0 );
        // xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 1 );

        // ACT
        xi_control_message_t* file_chunk_out =
            xi_cbor_codec_ct_decode( encoded, encoded_len );

        // xi_debug_control_message_dump( &file_chunk_in, "in" );
        // xi_debug_control_message_dump( file_chunk_out, "out" );

        // ASSERT
        xi_utest_cbor_ASSERT_control_messages_match( &file_chunk_in, file_chunk_out );

        XI_SAFE_FREE( encoded );
        xi_control_message_free( &file_chunk_out );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest_cbor_codec_ct_decode__file_chunk__weirdvalues,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        const xi_control_message_t file_chunk_in = {
            .file_chunk = {.common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_CHUNK,
                                      .msgver  = -1},
                           .name     = NULL,
                           .revision = "",
                           .offset   = -1,
                           .length   = 0,
                           .status   = 0,
                           .chunk    = NULL}};

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_cbor_codec_ct_server_encode( &file_chunk_in, &encoded, &encoded_len );

        // xi_utest_cbor_bin_to_stdout( encoded, ehncoded_len, 0 );
        // xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 1 );

        // ACT
        xi_control_message_t* file_chunk_out =
            xi_cbor_codec_ct_decode( encoded, encoded_len );

        // ASSERT
        xi_utest_cbor_ASSERT_control_messages_match( &file_chunk_in, file_chunk_out );

        XI_SAFE_FREE( encoded );
        xi_control_message_free( &file_chunk_out );
    } )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
