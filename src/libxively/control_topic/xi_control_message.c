/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_control_message.h>
#include <xi_macros.h>

#include <stdio.h>

void xi_control_message_free( xi_control_message_t** control_message )
{
    if ( NULL == control_message || NULL == *control_message )
    {
        return;
    }

    switch ( ( *control_message )->common.msgtype )
    {
        case XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO:
        {
            uint16_t id_file = 0;
            for ( ; id_file < ( *control_message )->file_info.list_len; ++id_file )
            {
                /* free xi_control_message_file_desc_t */
                XI_SAFE_FREE( ( *control_message )->file_info.list[id_file].name );
                XI_SAFE_FREE( ( *control_message )->file_info.list[id_file].revision );
            }

            XI_SAFE_FREE( ( *control_message )->file_info.list );
        }

        break;

        case XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE:
        {
            uint16_t id_file = 0;
            for ( ; id_file < ( *control_message )->file_update_available.list_len;
                  ++id_file )
            {
                /* free xi_control_message_file_desc_ext_t */
                XI_SAFE_FREE(
                    ( *control_message )->file_update_available.list[id_file].name );
                XI_SAFE_FREE(
                    ( *control_message )->file_update_available.list[id_file].revision );
                XI_SAFE_FREE( ( *control_message )
                                  ->file_update_available.list[id_file]
                                  .fingerprint );
            }

            XI_SAFE_FREE( ( *control_message )->file_update_available.list );
        }

        break;

        case XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK:

            XI_SAFE_FREE( ( *control_message )->file_get_chunk.name );
            XI_SAFE_FREE( ( *control_message )->file_get_chunk.revision );

            break;

        case XI_CONTROL_MESSAGE_SC__SFT_FILE_CHUNK:

            XI_SAFE_FREE( ( *control_message )->file_chunk.name );
            XI_SAFE_FREE( ( *control_message )->file_chunk.revision );
            XI_SAFE_FREE( ( *control_message )->file_chunk.chunk );

            break;

        case XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS:

            XI_SAFE_FREE( ( *control_message )->file_status.name );
            XI_SAFE_FREE( ( *control_message )->file_status.revision );

            break;

        case XI_CONTROL_MESSAGE_COUNT:;
    }

    XI_SAFE_FREE( *control_message );
}

#if XI_DEBUG_OUTPUT
void xi_debug_control_message_dump( const xi_control_message_t* control_message,
                                    const char* custom_label )
{
    printf( "+++ xi_control_message_t: +++++++++++++++++++++++++ [%s] \n", custom_label );

    if ( NULL == control_message )
    {
        printf( "+++ control message pointer is NULL\n" );
        return;
    }

    printf( "+++ msgtype: %d, msgver: %d\n", control_message->common.msgtype,
            control_message->common.msgver );

    switch ( control_message->common.msgtype )
    {
        case XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO:
        {
            printf( "+++ SFT FILE_INFO, list_len %d\n",
                    control_message->file_info.list_len );
            printf( "+++ #  [name], [revision]\n" );

            uint16_t id_file = 0;
            for ( ; id_file < control_message->file_info.list_len; ++id_file )
            {
                printf( "+++ #%d [%s], [%s]\n", id_file + 1,
                        control_message->file_info.list[id_file].name,
                        control_message->file_info.list[id_file].revision );
            }
        }
        break;

        case XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE:
        {
            printf( "+++ SFT FILE_UPDATE_AVAILABLE, list_len %d\n",
                    control_message->file_update_available.list_len );
            printf( "+++ #  [name], [revision], [file operation], [size in bytes], "
                    "[fingerprint]\n" );

            uint16_t id_file = 0;
            for ( ; id_file < control_message->file_update_available.list_len; ++id_file )
            {
                printf(
                    "+++ #%d [%s], [%s], [%d], [%d], [", id_file + 1,
                    control_message->file_update_available.list[id_file].name,
                    control_message->file_update_available.list[id_file].revision,
                    control_message->file_update_available.list[id_file].file_operation,
                    control_message->file_update_available.list[id_file].size_in_bytes );

                uint16_t id_byte = 0;
                for ( ; id_byte < control_message->file_update_available.list[id_file]
                                      .fingerprint_len;
                      ++id_byte )
                {
                    printf( "%c", control_message->file_update_available.list[id_file]
                                      .fingerprint[id_byte] );
                }

                printf( "]:[%d]\n", control_message->file_update_available.list[id_file]
                                        .fingerprint_len );
            }
        }
        break;

        case XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK:
            printf( "+++ SFT FILE_GET_CHUNK\n" );
            printf( "+++ name: [%s], revision: [%s]\n",
                    control_message->file_get_chunk.name,
                    control_message->file_get_chunk.revision );
            printf( "+++ offset: %d, length: %d\n",
                    control_message->file_get_chunk.offset,
                    control_message->file_get_chunk.length );
            break;

        case XI_CONTROL_MESSAGE_SC__SFT_FILE_CHUNK:
            printf( "+++ SFT FILE_CHUNK\n" );
            printf( "+++ name: [%s], revision: [%s]\n", control_message->file_chunk.name,
                    control_message->file_chunk.revision );
            printf( "+++ offset: %d, length: %d\n", control_message->file_chunk.offset,
                    control_message->file_chunk.length );
            printf( "+++ status: %d, chunk ptr: %p\n", control_message->file_chunk.status,
                    control_message->file_chunk.chunk );
            break;
        case XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS:
            printf( "+++ SFT FILE_STATUS\n" );
            printf( "+++ name: [%s], revision: [%s]\n", control_message->file_status.name,
                    control_message->file_status.revision );
            printf( "+++ phase: %d, code: %d\n", control_message->file_status.phase,
                    control_message->file_status.code );
            break;
        case XI_CONTROL_MESSAGE_COUNT:;
    }

    printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++ [%s]\n", custom_label );
}
#endif
