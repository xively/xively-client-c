/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_control_message.h>
#include <xi_macros.h>

#include <xi_debug.h>

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
                XI_SAFE_FREE( ( *control_message )
                                  ->file_update_available.list[id_file]
                                  .download_link );
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
                                    const char* debug_custom_label )
{
    XI_UNUSED( debug_custom_label );
    xi_debug_printf( "+++ xi_control_message_t: +++++++++++++++++++++++++ [%s] \n",
                     debug_custom_label );

    if ( NULL == control_message )
    {
        xi_debug_printf( "+++ control message pointer is NULL\n" );
        goto err_handling;
    }

    xi_debug_printf( "+++ msgtype: %d, msgver: %d\n", control_message->common.msgtype,
                     control_message->common.msgver );

    switch ( control_message->common.msgtype )
    {
        case XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO:
        {
            xi_debug_printf( "+++ SFT FILE_INFO, list_len %d\n",
                             control_message->file_info.list_len );
            xi_debug_printf( "++++ #  [name], [revision]\n" );

            uint16_t id_file = 0;
            for ( ; id_file < control_message->file_info.list_len; ++id_file )
            {
                xi_debug_printf( "++++ #%d [%s], [%s]\n", id_file + 1,
                                 control_message->file_info.list[id_file].name,
                                 control_message->file_info.list[id_file].revision );
            }

            xi_debug_printf( "+++ flag_accept_download_link: %x\n",
                             control_message->file_info.flag_accept_download_link );
        }
        break;

        case XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE:
        {
            xi_debug_printf( "+++ SFT FILE_UPDATE_AVAILABLE, list_len %d\n",
                             control_message->file_update_available.list_len );
            xi_debug_printf(
                "+++ #  [name], [revision], [file operation], [size in bytes], "
                "[fingerprint], [link], [MQTT download supported]\n" );

            uint16_t id_file = 0;
            for ( ; id_file < control_message->file_update_available.list_len; ++id_file )
            {
                xi_debug_printf(
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
                    xi_debug_printf( "%c",
                                     control_message->file_update_available.list[id_file]
                                         .fingerprint[id_byte] );
                }

                xi_debug_printf( "]:[%d]",
                                 control_message->file_update_available.list[id_file]
                                     .fingerprint_len );

                xi_debug_printf(
                    ", [%s], [%x]\n",
                    control_message->file_update_available.list[id_file].download_link,
                    control_message->file_update_available.list[id_file]
                        .flag_mqtt_download_also_supported );
            }
        }
        break;

        case XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK:
            xi_debug_printf( "+++ SFT FILE_GET_CHUNK\n" );
            xi_debug_printf( "+++ name: [%s], revision: [%s]\n",
                             control_message->file_get_chunk.name,
                             control_message->file_get_chunk.revision );
            xi_debug_printf( "+++ offset: %d, length: %d\n",
                             control_message->file_get_chunk.offset,
                             control_message->file_get_chunk.length );
            break;

        case XI_CONTROL_MESSAGE_SC__SFT_FILE_CHUNK:
            xi_debug_printf( "+++ SFT FILE_CHUNK\n" );
            xi_debug_printf( "+++ name: [%s], revision: [%s]\n",
                             control_message->file_chunk.name,
                             control_message->file_chunk.revision );
            xi_debug_printf( "+++ offset: %d, length: %d\n",
                             control_message->file_chunk.offset,
                             control_message->file_chunk.length );
            xi_debug_printf( "+++ status: %d, chunk ptr: %p\n",
                             control_message->file_chunk.status,
                             control_message->file_chunk.chunk );
            break;
        case XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS:
            xi_debug_printf( "+++ SFT FILE_STATUS\n" );
            xi_debug_printf( "+++ name: [%s], revision: [%s]\n",
                             control_message->file_status.name,
                             control_message->file_status.revision );
            xi_debug_printf( "+++ phase: %d, code: %d\n",
                             control_message->file_status.phase,
                             control_message->file_status.code );
            break;
        case XI_CONTROL_MESSAGE_COUNT:;
    }

err_handling:

    xi_debug_printf( "+++++++++++++++++++++++++++++++++++++++++++++++++++ [%s]\n",
                     debug_custom_label );
}
#endif
