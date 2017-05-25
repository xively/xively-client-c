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


#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

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
    cn_cbor* cb_map = cn_cbor_map_create( &err );

    cn_cbor_map_put( cb_map, cn_cbor_string_create( "msgtype", &err ),
                     cn_cbor_int_create( control_message->common.msgtype, &err ), &err );

    cn_cbor_map_put( cb_map, cn_cbor_string_create( "msgver", &err ),
                     cn_cbor_int_create( control_message->common.msgver, &err ), &err );

    switch ( control_message->common.msgtype )
    {
        /* the followings are encoded by the broker and decoded by the client */
        case XI_CONTROL_MESSAGE_BD_FILE_UPDATE_AVAILABLE:

            if ( 0 < control_message->file_update_available.list_len )
            {
                cn_cbor* files = cn_cbor_array_create( &err );

                uint16_t id_file = 0;
                for ( ; id_file < control_message->file_update_available.list_len;
                      ++id_file )
                {
                    cn_cbor* file = cn_cbor_map_create( &err );

                    xi_cbor_put_name_and_revision(
                        file, control_message->file_update_available.list[id_file].name,
                        control_message->file_update_available.list[id_file].revision,
                        &err );

                    cn_cbor_map_put(
                        file, cn_cbor_string_create( "O", &err ),
                        cn_cbor_int_create(
                            control_message->file_update_available.list[id_file]
                                .file_operation,
                            &err ),
                        &err );

                    cn_cbor_map_put(
                        file, cn_cbor_string_create( "S", &err ),
                        cn_cbor_int_create(
                            control_message->file_update_available.list[id_file]
                                .size_in_bytes,
                            &err ),
                        &err );

                    cn_cbor_map_put(
                        file, cn_cbor_string_create( "F", &err ),
                        cn_cbor_string_create(
                            control_message->file_update_available.list[id_file]
                                .fingerprint,
                            &err ),
                        &err );

                    cn_cbor_array_append( files, file, &err );
                }

                cn_cbor_map_put( cb_map, cn_cbor_string_create( "list", &err ), files,
                                 &err );
            }

            break;

        case XI_CONTROL_MESSAGE_BD_FILE_CHUNK:

            break;

        case XI_CONTROL_MESSAGE_DB_FILE_INFO:
        case XI_CONTROL_MESSAGE_DB_FILE_GET_CHUNK:
        case XI_CONTROL_MESSAGE_DB_FILE_STATUS:
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

#endif

XI_TT_TESTGROUP_BEGIN( utest_cbor_codec_ct_decode )


/*****************************************************
 * XI_CONTROL_MESSAGE_BD_FILE_UPDATE_AVAILABLE *******
 *****************************************************/

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest_cbor_codec_ct_decode__file_update_available__empty_list,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {

        const xi_control_message_t file_update_available_in = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_BD_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 0,
                .list     = NULL}};

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_utest_cbor_codec_ct_encode( &file_update_available_in, &encoded,
                                       &encoded_len );

        xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 0 );
        // xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 1 );

        xi_control_message_t* file_update_available_out =
            xi_cbor_codec_ct_decode( encoded, encoded_len );

        tt_int_op( 0, !=, file_update_available_out );
        tt_want_int_op( file_update_available_in.common.msgtype, ==,
                        file_update_available_out->common.msgtype );
        tt_want_int_op( file_update_available_in.common.msgver, ==,
                        file_update_available_out->common.msgver );
        tt_want_int_op( file_update_available_in.file_update_available.list_len, ==,
                        file_update_available_out->file_update_available.list_len );
        tt_want_int_op( file_update_available_in.file_update_available.list, ==,
                        file_update_available_out->file_update_available.list );

    end:
        XI_SAFE_FREE( encoded );
        XI_SAFE_FREE( file_update_available_out );

    } )

XI_TT_TESTCASE_WITH_SETUP(
    xi_utest_cbor_codec_ct_decode__file_update_available__single_file,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_control_message_file_desc_ext_t single_file_list[1] = {
            {.name           = "file to update",
             .revision       = "new revision",
             .file_operation = 99,
             .size_in_bytes  = 123,
             .fingerprint    = "my fingerprint 888"}};

        const xi_control_message_t file_update_available_in = {
            .file_update_available = {
                .common = {.msgtype = XI_CONTROL_MESSAGE_BD_FILE_UPDATE_AVAILABLE,
                           .msgver  = 1},
                .list_len = 1,
                .list     = single_file_list}};

        uint8_t* encoded     = NULL;
        uint32_t encoded_len = 0;

        xi_utest_cbor_codec_ct_encode( &file_update_available_in, &encoded,
                                       &encoded_len );

        xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 0 );
        // xi_utest_cbor_bin_to_stdout( encoded, encoded_len, 1 );

        xi_control_message_t* file_update_available_out =
            xi_cbor_codec_ct_decode( encoded, encoded_len );

        tt_int_op( 0, !=, file_update_available_out );
        tt_want_int_op( file_update_available_in.common.msgtype, ==,
                        file_update_available_out->common.msgtype );
        tt_want_int_op( file_update_available_in.common.msgver, ==,
                        file_update_available_out->common.msgver );
        tt_want_int_op( file_update_available_in.file_update_available.list_len, ==,
                        file_update_available_out->file_update_available.list_len );

    end:
        XI_SAFE_FREE( encoded );
        xi_control_message_free( &file_update_available_out );

    } )


XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
