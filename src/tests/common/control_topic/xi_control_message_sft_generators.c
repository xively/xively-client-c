/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_control_message_sft_generators.h>

#include <stddef.h>
#include <xively_error.h>
#include <xi_macros.h>

uint8_t* xi_control_message_sft_get_reproducible_randomlike_bytes( uint32_t offset,
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

xi_control_message_t*
xi_control_message_sft_generate_reply_FILE_CHUNK( xi_control_message_t* control_message )
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

    file_chunk_out_artificial = xi_control_message_sft_get_reproducible_randomlike_bytes(
        control_message->file_get_chunk.offset, control_message->file_get_chunk.length );

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
