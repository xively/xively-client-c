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

void xi_cbor_put_name_and_revision( cn_cbor* cb_map,
                                    const char* name,
                                    const char* revision,
                                    cn_cbor_errback* errp );

void xi_utest_cbor_codec_ct_encode( const xi_control_message_t* control_message,
                                    uint8_t** out_encoded_allocated_inside,
                                    uint32_t* out_len )
{
    cn_cbor_errback err;
    cn_cbor* cb_map = cn_cbor_map_create( CBOR_CONTEXT_PARAM_COMMA & err );

    cn_cbor_map_put(
        cb_map, cn_cbor_string_create( "msgtype" CBOR_CONTEXT_PARAM, &err ),
        cn_cbor_int_create( control_message->common.msgtype CBOR_CONTEXT_PARAM, &err ),
        &err );

    cn_cbor_map_put(
        cb_map, cn_cbor_string_create( "msgver" CBOR_CONTEXT_PARAM, &err ),
        cn_cbor_int_create( control_message->common.msgver CBOR_CONTEXT_PARAM, &err ),
        &err );

    switch ( control_message->common.msgtype )
    {
        /* the followings are encoded by the broker and decoded by the client */
        case XI_CONTROL_MESSAGE_SC_FILE_UPDATE_AVAILABLE:

            if ( 0 < control_message->file_update_available.list_len )
            {
                cn_cbor* files = cn_cbor_array_create( CBOR_CONTEXT_PARAM_COMMA & err );

                uint16_t id_file = 0;
                for ( ; id_file < control_message->file_update_available.list_len;
                      ++id_file )
                {
                    cn_cbor* file = cn_cbor_map_create( CBOR_CONTEXT_PARAM_COMMA & err );

                    xi_cbor_put_name_and_revision(
                        file, control_message->file_update_available.list[id_file].name,
                        control_message->file_update_available.list[id_file].revision,
                        &err );

                    cn_cbor_map_put(
                        file, cn_cbor_string_create( "O" CBOR_CONTEXT_PARAM, &err ),
                        cn_cbor_int_create(
                            control_message->file_update_available.list[id_file]
                                .file_operation CBOR_CONTEXT_PARAM,
                            &err ),
                        &err );

                    cn_cbor_map_put(
                        file, cn_cbor_string_create( "S" CBOR_CONTEXT_PARAM, &err ),
                        cn_cbor_int_create(
                            control_message->file_update_available.list[id_file]
                                .size_in_bytes CBOR_CONTEXT_PARAM,
                            &err ),
                        &err );

                    cn_cbor_map_put(
                        file, cn_cbor_string_create( "F" CBOR_CONTEXT_PARAM, &err ),
                        cn_cbor_string_create(
                            control_message->file_update_available.list[id_file]
                                .fingerprint CBOR_CONTEXT_PARAM,
                            &err ),
                        &err );

                    cn_cbor_array_append( files, file, &err );
                }

                cn_cbor_map_put( cb_map,
                                 cn_cbor_string_create( "list" CBOR_CONTEXT_PARAM, &err ),
                                 files, &err );
            }

            break;

        case XI_CONTROL_MESSAGE_SC_FILE_CHUNK:

            xi_cbor_put_name_and_revision( cb_map, control_message->file_chunk.name,
                                           control_message->file_chunk.revision, &err );

            cn_cbor_map_put(
                cb_map, cn_cbor_string_create( "O" CBOR_CONTEXT_PARAM, &err ),
                cn_cbor_int_create( control_message->file_chunk.offset CBOR_CONTEXT_PARAM,
                                    &err ),
                &err );

            cn_cbor_map_put(
                cb_map, cn_cbor_string_create( "L" CBOR_CONTEXT_PARAM, &err ),
                cn_cbor_int_create( control_message->file_chunk.length CBOR_CONTEXT_PARAM,
                                    &err ),
                &err );

            break;

        case XI_CONTROL_MESSAGE_CS_FILE_INFO:
        case XI_CONTROL_MESSAGE_CS_FILE_GET_CHUNK:
        case XI_CONTROL_MESSAGE_CS_FILE_STATUS:
        default:
            cn_cbor_free( cb_map CBOR_CONTEXT_PARAM );
            return;
    }

    unsigned char encoded[512];
    *out_len = cn_cbor_encoder_write( encoded, 0, sizeof( encoded ), cb_map );

    cn_cbor_free( cb_map CBOR_CONTEXT_PARAM );

    xi_state_t state = XI_STATE_OK;
    XI_ALLOC_BUFFER_AT( uint8_t, *out_encoded_allocated_inside, *out_len, state );

    memcpy( *out_encoded_allocated_inside, encoded, *out_len );

err_handling:;
}

void xi_utest_cbor_ASSERT_control_message_string( const char* str1, const char* str2 )
{
    if ( NULL == str1 )
    {
        tt_want_int_op( str1, ==, str2 );
    }
    else
    {
        tt_want_int_op( 0, ==, strcmp( str1, str2 ) );
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
        case XI_CONTROL_MESSAGE_SC_FILE_UPDATE_AVAILABLE:
            tt_int_op( cm1->file_update_available.list_len, ==,
                       cm2->file_update_available.list_len );

            uint8_t id_file = 0;
            for ( ; id_file < cm1->file_update_available.list_len; ++id_file )
            {
                tt_want_int_op( 0, ==,
                                strcmp( cm1->file_update_available.list[id_file].name,
                                        cm2->file_update_available.list[id_file].name ) );

                tt_want_int_op(
                    0, ==, strcmp( cm1->file_update_available.list[id_file].revision,
                                   cm2->file_update_available.list[id_file].revision ) );

                tt_want_int_op( cm1->file_update_available.list[id_file].file_operation,
                                ==,
                                cm2->file_update_available.list[id_file].file_operation );

                tt_want_int_op( cm1->file_update_available.list[id_file].size_in_bytes,
                                ==,
                                cm2->file_update_available.list[id_file].size_in_bytes );

                tt_want_int_op(
                    0, ==,
                    strcmp( cm1->file_update_available.list[id_file].fingerprint,
                            cm2->file_update_available.list[id_file].fingerprint ) );

                /*xi_debug_printf( "name: [%s]\n",
                                 cm2->file_update_available.list[id_file].name );
                xi_debug_printf( "name: [%s]\n",
                                 cm2->file_update_available.list[id_file].revision );
                xi_debug_printf( "name: [%s]\n",
                                 cm2->file_update_available.list[id_file].fingerprint );*/
            }

            break;

        case XI_CONTROL_MESSAGE_SC_FILE_CHUNK:

            xi_utest_cbor_ASSERT_control_message_string( cm1->file_chunk.name,
                                                         cm2->file_chunk.name );

            xi_utest_cbor_ASSERT_control_message_string( cm1->file_chunk.revision,
                                                         cm2->file_chunk.revision );

            tt_want_int_op( cm1->file_chunk.offset, ==, cm2->file_chunk.offset );
            tt_want_int_op( cm1->file_chunk.length, ==, cm2->file_chunk.length );

            break;

        default:;
    }
end:;
}

#endif

XI_TT_TESTGROUP_BEGIN( utest_cbor_codec_ct_decode )


/*****************************************************
 * XI_CONTROL_MESSAGE_SC_FILE_UPDATE_AVAILABLE *******
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
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 0,
                .list     = NULL}};

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_utest_cbor_codec_ct_encode( &file_update_available_in, &encoded,
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
            {.name           = "name of file to update",
             .revision       = "new revision",
             .file_operation = 99,
             .size_in_bytes  = 123,
             .fingerprint    = "my fingerprint 888"}};

        const xi_control_message_t file_update_available_in = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 1,
                .list     = single_file_list}};

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_utest_cbor_codec_ct_encode( &file_update_available_in, &encoded,
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
            {.name           = "filename 1",
             .revision       = "revision 11",
             .file_operation = 11,
             .size_in_bytes  = 111,
             .fingerprint    = "fingerprint 111"},
            {.name           = "filename 2",
             .revision       = "revision 22",
             .file_operation = 22,
             .size_in_bytes  = 222,
             .fingerprint    = "fingerprint 222"},
            {.name           = "filename 3",
             .revision       = "revision 33",
             .file_operation = 33,
             .size_in_bytes  = 333,
             .fingerprint    = "fingerprint 333"}};

        const xi_control_message_t file_update_available_in = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 3,
                .list     = three_file_list}};

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_utest_cbor_codec_ct_encode( &file_update_available_in, &encoded,
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
 * XI_CONTROL_MESSAGE_SC_FILE_CHUNK ******************
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
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC_FILE_CHUNK, .msgver = 1},
                .name     = "filename for filechunk message",
                .revision = "revision for filechunk message",
                .offset   = 0,
                .length   = 1024}};

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_utest_cbor_codec_ct_encode( &file_chunk_in, &encoded, &encoded_len );

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
    xi_utest_cbor_codec_ct_decode__file_chunk__weirdvalues,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // ARRANGE
        const xi_control_message_t file_chunk_in = {
            .file_chunk = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_SC_FILE_CHUNK, .msgver = -1},
                .name     = NULL,
                .revision = "",
                .offset   = -1,
                .length   = 0}};

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_utest_cbor_codec_ct_encode( &file_chunk_in, &encoded, &encoded_len );

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


XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
