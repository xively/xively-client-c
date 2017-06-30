/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_itest_mock_broker_layer_sft.h"
#include <stdio.h>

#include <xi_control_message.h>
#include <xi_cbor_codec_ct_server.h>
#include "xi_itest_helpers.h"
#include "xi_helpers.h"

xi_control_message_t*
xi_mock_broker_sft_logic_on_file_info( const xi_control_message_t* control_message )
{
    if ( NULL == control_message )
    {
        return NULL;
    }

    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_control_message_t, control_message_reply, state );

    control_message_reply->common.msgtype = XI_CONTROL_MESSAGE_SC_FILE_UPDATE_AVAILABLE;
    control_message_reply->common.msgver  = 1;

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

        control_message_reply->file_update_available.list[id_file].revision =
            xi_str_cat( control_message->file_info.list[id_file].revision, " - update" );

        control_message_reply->file_update_available.list[id_file].file_operation = 0;
        control_message_reply->file_update_available.list[id_file].size_in_bytes =
            7777 * ( 1 + id_file );

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
xi_mock_broker_sft_logic_on_file_get_chunk( const xi_control_message_t* control_message )
{
    if ( NULL == control_message )
    {
        return NULL;
    }

    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_control_message_t, control_message_reply, state );

    control_message_reply->common.msgtype = XI_CONTROL_MESSAGE_SC_FILE_CHUNK;
    control_message_reply->common.msgver  = 1;

    return control_message_reply;

err_handling:

    xi_control_message_free( &control_message_reply );

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
        const xi_control_message_t* ) = {0};

    mock_broker_logic_by_msgtype[XI_CONTROL_MESSAGE_CS_FILE_INFO] =
        &xi_mock_broker_sft_logic_on_file_info;
    mock_broker_logic_by_msgtype[XI_CONTROL_MESSAGE_CS_FILE_GET_CHUNK] =
        &xi_mock_broker_sft_logic_on_file_get_chunk;
    mock_broker_logic_by_msgtype[XI_CONTROL_MESSAGE_CS_FILE_STATUS] = NULL;

    /* applying mock broker logic */
    if ( NULL != mock_broker_logic_by_msgtype[control_message->common.msgtype] )
    {
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

    // - channel each msg type into separate functions (3 messages) to help failure
    //   control from integration test case
    // - apply mock broker logic: happy broker OR controlled failure -->
    //   xi_control_message_t
    // - encode and send to client
}
