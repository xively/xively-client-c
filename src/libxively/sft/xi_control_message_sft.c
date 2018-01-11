/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_control_message_sft.h"

#include <xively_error.h>

#include <xi_macros.h>
#include <xi_helpers.h>
#include <xi_sft_revision.h>

xi_control_message_t*
xi_control_message_create_file_info( const char** filenames,
                                     uint16_t count,
                                     uint8_t flag_accept_download_link )
{
    if ( NULL == filenames || NULL == *filenames || 0 == count )
    {
        return NULL;
    }

    xi_state_t state = XI_STATE_OK;
    uint16_t id_file = 0;

    XI_ALLOC( xi_control_message_t, sft_message, state );

    sft_message->common.msgtype = XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO;
    sft_message->common.msgver  = 1;
    sft_message->file_info.flag_accept_download_link = flag_accept_download_link;

    XI_ALLOC_BUFFER_AT( xi_control_message_file_desc_t, sft_message->file_info.list,
                        sizeof( xi_control_message_file_desc_t ) * count, state );

    sft_message->file_info.list_len = count;

    for ( ; id_file < count; ++id_file )
    {
        sft_message->file_info.list[id_file].name = xi_str_dup( *filenames );

        char* revision = NULL;
        state          = xi_sft_revision_get( *filenames, &revision );

        if ( XI_STATE_OK == state && NULL != revision )
        {
            /* passing ownership */
            sft_message->file_info.list[id_file].revision = revision;
        }
        else
        {
            sft_message->file_info.list[id_file].revision =
                xi_str_dup( XI_CONTROL_MESSAGE_SFT_GENERATED_REVISION );
        }

        ++filenames;
    }

    return sft_message;

err_handling:

    xi_control_message_free( &sft_message );

    return sft_message;
}

xi_control_message_t* xi_control_message_create_file_get_chunk( const char* filename,
                                                                const char* revision,
                                                                uint32_t offset,
                                                                uint32_t length )
{
    if ( NULL == filename || NULL == revision )
    {
        return NULL;
    }

    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_control_message_t, sft_message, state );

    sft_message->common.msgtype = XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK;
    sft_message->common.msgver  = 1;

    sft_message->file_get_chunk.name     = xi_str_dup( filename );
    sft_message->file_get_chunk.revision = xi_str_dup( revision );
    sft_message->file_get_chunk.offset   = offset;
    sft_message->file_get_chunk.length   = length;

    return sft_message;

err_handling:

    xi_control_message_free( &sft_message );

    return sft_message;
}

xi_control_message_t* xi_control_message_create_file_status( const char* filename,
                                                             const char* revision,
                                                             uint8_t phase,
                                                             int8_t code )
{
    if ( NULL == filename || NULL == revision )
    {
        return NULL;
    }

    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_control_message_t, sft_message, state );

    sft_message->common.msgtype = XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS;
    sft_message->common.msgver  = 1;

    sft_message->file_status.name     = xi_str_dup( filename );
    sft_message->file_status.revision = xi_str_dup( revision );
    sft_message->file_status.phase    = phase;
    sft_message->file_status.code     = code;

    return sft_message;

err_handling:

    xi_control_message_free( &sft_message );

    return sft_message;
}