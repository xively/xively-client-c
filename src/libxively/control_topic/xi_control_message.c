/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_control_message.h>
#include <stddef.h>
#include <xi_macros.h>

#include <stdio.h>

char* xi_control_message_strdup( const char* string_to_duplicate )
{
    if ( NULL == string_to_duplicate )
    {
        return NULL;
    }

    char* target = NULL;

    xi_state_t state = XI_STATE_OK;

    XI_ALLOC_BUFFER_AT( char, target, strlen( string_to_duplicate ) + 1, state );

    memcpy( target, string_to_duplicate, strlen( string_to_duplicate ) + 1 );

err_handling:
    return target;
}

xi_control_message_t* xi_control_message_create_file_info( const char** filenames,
                                                           const char** revisions,
                                                           uint16_t count )
{
    ( void )revisions;

    if ( NULL == filenames || NULL == *filenames || 0 == count )
    {
        return NULL;
    }

    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_control_message_t, file_info, state );

    file_info->file_info.common.msgtype = XI_CONTROL_MESSAGE_DB_FILE_INFO;
    file_info->file_info.common.msgver  = 1;

    XI_ALLOC_BUFFER_AT( xi_control_message_file_desc_t, file_info->file_info.list,
                        sizeof( xi_control_message_file_desc_t ) * count, state );

    file_info->file_info.list_len = count;

    uint16_t id_file = 0;
    for ( ; id_file < count; ++id_file )
    {
        file_info->file_info.list[id_file].name = xi_control_message_strdup( *filenames );
        ++filenames;

        if ( NULL != revisions )
        {
            file_info->file_info.list[id_file].revision =
                xi_control_message_strdup( *revisions );
            ++revisions;
        }
    }

err_handling:

    return file_info;
}

void xi_control_message_free( xi_control_message_t** control_message )
{
    if ( NULL == control_message || NULL == *control_message )
    {
        return;
    }

    switch ( ( *control_message )->common.msgtype )
    {
        case XI_CONTROL_MESSAGE_DB_FILE_INFO:
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

        case XI_CONTROL_MESSAGE_BD_FILE_UPDATE_AVAILABLE:
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

        case XI_CONTROL_MESSAGE_DB_FILE_GET_CHUNK:

            break;

        case XI_CONTROL_MESSAGE_BD_FILE_CHUNK:

            XI_SAFE_FREE( ( *control_message )->file_chunk.name );
            XI_SAFE_FREE( ( *control_message )->file_chunk.revision );

            break;

        case XI_CONTROL_MESSAGE_DB_FILE_STATUS:

            break;
    }

    XI_SAFE_FREE( *control_message );
}
