/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
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

#ifdef XI_SECURE_FILE_TRANSFER_ENABLED

uint8_t* xi_mock_broker_sft_logic_get_reproducible_randomlike_bytes( uint32_t offset,
                                                                     uint32_t length )
{
    xi_state_t state = XI_STATE_OK;

    XI_ALLOC_BUFFER( uint8_t, reproducible_randomlike_bytes, length, state );

    uint8_t* it_byte = reproducible_randomlike_bytes;

    uint32_t id_byte = offset;
    for ( ; id_byte < offset + length; ++id_byte, ++it_byte )
    {
        /* the simpliest "random" bytes */
        *it_byte = id_byte;
    }

    return reproducible_randomlike_bytes;

err_handling:

    XI_SAFE_FREE( reproducible_randomlike_bytes );

    return NULL;
}

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
        xi_mock_broker_sft_logic_get_reproducible_randomlike_bytes( 0, size_in_bytes );

    void* checksum_context = NULL;

    xi_bsp_fwu_checksum_init( &checksum_context );

    xi_bsp_fwu_checksum_update( checksum_context, artificial_file_content,
                                size_in_bytes );

    uint8_t* checksum = NULL;

    xi_bsp_fwu_checksum_final( checksum_context, &checksum, fingerprint_len_out );

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
            control_message->file_info.list[id_file]
                .revision[strlen( control_message->file_info.list[id_file].revision ) -
                          1]++;
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

xi_control_message_t* xi_mock_broker_sft_logic_generate_reply_FILE_CHUNK(
    xi_control_message_t* control_message )
{
    if ( NULL == control_message )
    {
        return NULL;
    }

    uint8_t* file_chunk_out_artificial = NULL;
    xi_state_t state                   = XI_STATE_OK;

    enum XI_MOCK_BROKER_SFT_FILE_CHUNK_STATUS
    {
        MOCK_BROKER_SFT_FILE_CHUNK__REQUEST_IS_CORRECT                       = 0,
        MOCK_BROKER_SFT_FILE_CHUNK__END_OF_FILE_CHUNK_MIGHT_BE_SHORTER       = 1,
        MOCK_BROKER_SFT_FILE_CHUNK__OFFSET_IS_GREATER_THAN_FILE_LENGTH       = 2,
        MOCK_BROKER_SFT_FILE_CHUNK__REQUESTED_LENGTH_IS_GREATER_THAN_MAXIMUM = 3,
        MOCK_BROKER_SFT_FILE_CHUNK__FILE_UNAVAILABLE                         = 4,
    };

    XI_ALLOC( xi_control_message_t, control_message_reply, state );

    file_chunk_out_artificial =
        xi_mock_broker_sft_logic_get_reproducible_randomlike_bytes(
            control_message->file_get_chunk.offset,
            control_message->file_get_chunk.length );

    control_message_reply->file_chunk = ( struct file_chunk_s ){
        .common = {.msgtype = XI_CONTROL_MESSAGE_SC__SFT_FILE_CHUNK, .msgver = 1},
        .name     = control_message->file_get_chunk.name,
        .revision = control_message->file_get_chunk.revision,
        .offset   = control_message->file_get_chunk.offset,
        .length   = control_message->file_get_chunk.length,
        .status   = MOCK_BROKER_SFT_FILE_CHUNK__REQUEST_IS_CORRECT,
        .chunk    = file_chunk_out_artificial};


    /* prevent deallocation of name and revision since these are reused in reply msg
     */
    control_message->file_get_chunk.name     = NULL;
    control_message->file_get_chunk.revision = NULL;

    return control_message_reply;

err_handling:

    xi_control_message_free( &control_message_reply );
    XI_SAFE_FREE( file_chunk_out_artificial );

    return NULL;
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
            xi_mock_broker_sft_logic_generate_reply_FILE_CHUNK( control_message );
    }

    return control_message_reply;
}

xi_control_message_t*
xi_mock_broker_sft_logic_on_file_status( xi_control_message_t* control_message )
{
    XI_UNUSED( control_message );

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
            // todo_atigyi: make more elegant ownership passing
            cbor_reply_message_data_desc->memory_type = XI_MEMORY_TYPE_MANAGED;
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
