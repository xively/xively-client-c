/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_sft_logic.h>
#include <stddef.h>
#include <xi_macros.h>

xi_state_t xi_sft_make_context( xi_sft_context_t** context,
                                const char** updateable_files,
                                uint16_t updateable_files_count,
                                fn_send_control_message fn_send_message,
                                void* user_data )
{
    if ( NULL == context )
    {
        return XI_INVALID_PARAMETER;
    }

    if ( NULL != *context )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t state = XI_STATE_OK;

    XI_ALLOC_AT( xi_sft_context_t, *context, state );

    ( *context )->fn_send_message        = fn_send_message;
    ( *context )->send_message_user_data = user_data;
    ( *context )->updateable_files       = updateable_files;
    ( *context )->updateable_files_count = updateable_files_count;

    return state;

err_handling:

    XI_SAFE_FREE( *context );
    return state;
}

xi_state_t xi_sft_free_context( xi_sft_context_t** context )
{
    if ( NULL != context && NULL != *context )
    {
        XI_SAFE_FREE( *context );
    }

    return XI_STATE_OK;
}

xi_state_t xi_sft_on_connected( xi_sft_context_t* context )
{
    // printf( "%s, updateable_files_count: %d\n", __FUNCTION__,
    //         context->updateable_files_count );

    if ( NULL == context )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_control_message_t* message_file_info = xi_control_message_create_file_info(
        context->updateable_files, NULL, context->updateable_files_count );

    ( *context->fn_send_message )( context->send_message_user_data, message_file_info );

    return XI_STATE_OK;
}

xi_state_t
xi_sft_on_message( xi_sft_context_t* context, const xi_control_message_t* sft_message )
{
    if ( NULL == context || NULL == sft_message )
    {
        return XI_INVALID_PARAMETER;
    }

    switch ( sft_message->common.msgtype )
    {
        case XI_CONTROL_MESSAGE_BD_FILE_UPDATE_AVAILABLE:
            // - check whether device is ready to start download of file
            // - get the first chunk of file in quiestion with FILE_GET_CHUNK
            break;
        case XI_CONTROL_MESSAGE_BD_FILE_CHUNK:
            // - store the chunk through file/firmware BSP
            // - get the next chunk, XI_CONTROL_MESSAGE_DB_FILE_GET_CHUNK
            // - if last chunk close file, start appropriate action
            //      - if firmware: start firmware update or "just" report downloaded and
            //        wait for remote FWU trigger
            //      - other files may have their own action, no-operation is an option
            break;
        default:;
    }

    return XI_STATE_OK;
}
