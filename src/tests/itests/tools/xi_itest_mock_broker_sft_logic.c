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

xi_control_message_t*
xi_mock_broker_sft_logic_on_file_info( xi_control_message_t* control_message )
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
            7777 * ( id_file % 3 + 1 );

        const char* fingerprint =
            xi_str_cat( "@#$@#$@$xxx^ - test fingerprint - ",
                        control_message_reply->file_update_available.list[id_file].name );
        control_message_reply->file_update_available.list[id_file].fingerprint =
            ( uint8_t* )fingerprint;
        control_message_reply->file_update_available.list[id_file].fingerprint_len =
            strlen( fingerprint );
    }

    return control_message_reply;

err_handling:

    xi_control_message_free( &control_message_reply );

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

    uint8_t* file_chunk_out_artificial = NULL;

    xi_control_message_t* control_message_reply = mock_type( xi_control_message_t* );

    if ( NULL == control_message_reply )
    {
        xi_state_t state = XI_STATE_OK;

        enum XI_MOCK_BROKER_SFT_FILE_CHUNK_STATUS
        {
            MOCK_BROKER_SFT_FILE_CHUNK__REQUEST_IS_CORRECT                       = 0,
            MOCK_BROKER_SFT_FILE_CHUNK__END_OF_FILE_CHUNK_MIGHT_BE_SHORTER       = 1,
            MOCK_BROKER_SFT_FILE_CHUNK__OFFSET_IS_GREATER_THAN_FILE_LENGTH       = 2,
            MOCK_BROKER_SFT_FILE_CHUNK__REQUESTED_LENGTH_IS_GREATER_THAN_MAXIMUM = 3,
            MOCK_BROKER_SFT_FILE_CHUNK__FILE_UNAVAILABLE                         = 4,
        };

        XI_ALLOC_AT( xi_control_message_t, control_message_reply, state );
        XI_ALLOC_BUFFER_AT( uint8_t, file_chunk_out_artificial,
                            control_message->file_get_chunk.length, state );

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
    }

    return control_message_reply;

err_handling:

    xi_control_message_free( &control_message_reply );
    XI_SAFE_FREE( file_chunk_out_artificial );

    return NULL;
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
