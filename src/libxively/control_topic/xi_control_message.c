/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_control_message.h>
#include <stddef.h>
#include <xi_macros.h>

void xi_control_message_free( xi_control_message_t** control_message )
{
    if ( NULL == control_message || NULL == *control_message )
    {
        return;
    }

    switch ( ( *control_message )->common.msgtype )
    {
        case XI_CONTROL_MESSAGE_DB_FILE_INFO:

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

            break;

        case XI_CONTROL_MESSAGE_DB_FILE_STATUS:

            break;
    }

    XI_SAFE_FREE( *control_message );
}
