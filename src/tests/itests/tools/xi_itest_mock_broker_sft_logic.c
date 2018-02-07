/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_itest_mock_broker_sft_logic.h"
#include <stdio.h>

#include <xi_control_message.h>
#include <xi_cbor_codec_ct_server.h>
#include "xi_itest_helpers.h"
#include "xi_helpers.h"
#include <xi_bsp_fwu.h>

#include <xi_control_message_sft_generators.h>

#ifdef XI_SECURE_FILE_TRANSFER_ENABLED


void xi_mock_broker_sft_logic_get_fingerprint( uint32_t size_in_bytes,
                                               uint8_t** fingerprint_out,
                                               uint16_t* fingerprint_len_out )
{
    /* try to fill up fingerprint from a test-case-set one */
    *fingerprint_out     = mock_type( uint8_t* );
    *fingerprint_len_out = mock_type( uint16_t );

    if ( NULL != *fingerprint_out && 0 != *fingerprint_len_out )
    {
        return;
    }

    uint8_t* artificial_file_content =
        xi_control_message_sft_get_reproducible_randomlike_bytes( 0, size_in_bytes );

    void* checksum_context = NULL;

    xi_bsp_fwu_checksum_init( &checksum_context );

    xi_bsp_fwu_checksum_update( checksum_context, artificial_file_content,
                                size_in_bytes );

    uint8_t* checksum = NULL;

    xi_bsp_fwu_checksum_final( &checksum_context, &checksum, fingerprint_len_out );

    xi_state_t state = XI_STATE_OK;

    XI_ALLOC_BUFFER_AT( uint8_t, *fingerprint_out, *fingerprint_len_out, state );
    memcpy( *fingerprint_out, checksum, *fingerprint_len_out );

    XI_SAFE_FREE( artificial_file_content );

    return;

err_handling:

    XI_SAFE_FREE( artificial_file_content );
    XI_SAFE_FREE( *fingerprint_out );
}

xi_control_message_t* xi_mock_broker_sft_logic_generate_reply_happy_FUA(
    const xi_control_message_t* control_message )
{
    if ( NULL == control_message )
    {
        return NULL;
    }

    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_control_message_t, control_message_reply, state );

    control_message_reply->common.msgtype =
        XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE;
    control_message_reply->common.msgver = 1;

    XI_ALLOC_BUFFER_AT( xi_control_message_file_desc_ext_t,
                        control_message_reply->file_update_available.list,
                        sizeof( xi_control_message_file_desc_ext_t ) *
                            control_message->file_info.list_len,
                        state );

    control_message_reply->file_update_available.list_len =
        control_message->file_info.list_len;

    uint16_t id_file = 0;
    for ( ; id_file < control_message->file_info.list_len; ++id_file )
    {
        control_message_reply->file_update_available.list[id_file].name =
            control_message->file_info.list[id_file].name;
        /* prevent deallocation of reused name */
        control_message->file_info.list[id_file].name = NULL;

        /* altering last character, just to return different revision */
        if ( NULL != control_message->file_info.list[id_file].revision )
        {
            ++control_message->file_info.list[id_file]
                  .revision[strlen( control_message->file_info.list[id_file].revision ) -
                            1];
        }

        control_message_reply->file_update_available.list[id_file].revision =
            xi_str_dup( control_message->file_info.list[id_file].revision
                            ? control_message->file_info.list[id_file].revision
                            : "mock broker generated revision 0" );

        control_message_reply->file_update_available.list[id_file].file_operation = 0;
        control_message_reply->file_update_available.list[id_file].size_in_bytes =
            XI_MOCK_BROKER_SFT__FILE_CHUNK_STEP_SIZE * ( id_file % 3 + 1 );

        xi_mock_broker_sft_logic_get_fingerprint(
            control_message_reply->file_update_available.list[id_file].size_in_bytes,
            &control_message_reply->file_update_available.list[id_file].fingerprint,
            &control_message_reply->file_update_available.list[id_file].fingerprint_len );

        control_message_reply->file_update_available.list[id_file].download_link =
            control_message->file_info.flag_accept_download_link
                ? xi_str_cat(
                      "url for custom download of file: ",
                      control_message_reply->file_update_available.list[id_file].name )
                : NULL;

        control_message_reply->file_update_available.list[id_file]
            .flag_mqtt_download_also_supported = mock_type( uint8_t );
    }

    return control_message_reply;

err_handling:

    xi_control_message_free( &control_message_reply );

    return NULL;
}

xi_control_message_t*
xi_mock_broker_sft_logic_on_file_info( xi_control_message_t* control_message )
{
    if ( NULL == control_message )
    {
        return NULL;
    }

    /* generating a "happy" FILE_UPDATE_AVAILABLE reply containing all requested files */
    xi_control_message_t* control_message_reply =
        xi_mock_broker_sft_logic_generate_reply_happy_FUA( control_message );

    return control_message_reply;
}

xi_control_message_t*
xi_mock_broker_sft_logic_on_file_get_chunk( xi_control_message_t* control_message )
{
    if ( NULL == control_message )
    {
        return NULL;
    }

    check_expected( control_message->file_get_chunk.name );

    /* try to fill up reply message set before in the test case */
    xi_control_message_t* control_message_reply = mock_type( xi_control_message_t* );

    /* in case test case didn't define reply message, then generate a happy one :) */
    if ( NULL == control_message_reply )
    {
        control_message_reply =
            xi_control_message_sft_generate_reply_FILE_CHUNK( control_message );
    }

    return control_message_reply;
}

xi_control_message_t*
xi_mock_broker_sft_logic_on_file_status( xi_control_message_t* control_message )
{
    if ( NULL == control_message )
    {
        return NULL;
    }

    check_expected( control_message->file_status.phase );
    check_expected( control_message->file_status.code );

    return NULL;
}

xi_data_desc_t*
xi_mock_broker_sft_logic_on_message( const xi_data_desc_t* control_message_encoded )
{
    xi_data_desc_t* cbor_reply_message_data_desc = NULL;

    if ( NULL == control_message_encoded )
    {
        return cbor_reply_message_data_desc;
    }

    /* CBOR decode */
    xi_control_message_t* control_message = xi_cbor_codec_ct_server_decode(
        control_message_encoded->data_ptr, control_message_encoded->length );

    xi_debug_control_message_dump( control_message, "mock broker --- INCOMING" );

    if ( NULL == control_message )
    {
        return cbor_reply_message_data_desc;
    }

    check_expected( control_message->common.msgtype );

    xi_control_message_t* ( *mock_broker_logic_by_msgtype[XI_CONTROL_MESSAGE_COUNT] )(
        xi_control_message_t* ) = {0};

    mock_broker_logic_by_msgtype[XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO] =
        &xi_mock_broker_sft_logic_on_file_info;
    mock_broker_logic_by_msgtype[XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK] =
        &xi_mock_broker_sft_logic_on_file_get_chunk;
    mock_broker_logic_by_msgtype[XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS] =
        &xi_mock_broker_sft_logic_on_file_status;

    if ( NULL != mock_broker_logic_by_msgtype[control_message->common.msgtype] )
    {
        /* applying mock broker logic */
        xi_control_message_t* reply_message =
            ( mock_broker_logic_by_msgtype[control_message->common.msgtype] )(
                control_message );

        xi_debug_control_message_dump( reply_message, "mock broker --- OUTGOING" );

        uint8_t* cbor_reply_message     = NULL;
        uint32_t cbor_reply_message_len = 0;

        /* CBOR encode */
        xi_cbor_codec_ct_server_encode( reply_message, &cbor_reply_message,
                                        &cbor_reply_message_len );


        if ( NULL != cbor_reply_message )
        {
            cbor_reply_message_data_desc = xi_make_desc_from_buffer_share(
                cbor_reply_message, cbor_reply_message_len );
        }
        else
        {
            xi_debug_logger( "WARNING: CBOR encoding failed" );
        }


        xi_control_message_free( &reply_message );
    }
    else
    {
        xi_debug_format( "[WARNING] no message handler found for msgtype: %d",
                         control_message->common.msgtype );
    }

    xi_control_message_free( &control_message );

    return cbor_reply_message_data_desc;
}

#else

xi_data_desc_t*
xi_mock_broker_sft_logic_on_message( const xi_data_desc_t* control_message_encoded )
{
    XI_UNUSED( control_message_encoded );

    return NULL;
}

#endif
