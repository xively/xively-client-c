/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_sft_logic.h>
#include <stddef.h>
#include <xi_macros.h>
#include <xi_debug.h>

#include <xi_bsp_io_fs.h>
#include <xi_bsp_fwu.h>

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

    ( *context )->fn_send_message            = fn_send_message;
    ( *context )->send_message_user_data     = user_data;
    ( *context )->updateable_files           = updateable_files;
    ( *context )->updateable_files_count     = updateable_files_count;
    ( *context )->update_message_fua         = NULL;
    ( *context )->update_current_file        = NULL;
    ( *context )->update_file_handle         = 0;
    ( *context )->firmware_in_update_package = 0;

    return state;

err_handling:

    XI_SAFE_FREE( *context );
    return state;
}

xi_state_t xi_sft_free_context( xi_sft_context_t** context )
{
    if ( NULL != context && NULL != *context )
    {
        xi_control_message_free( &( *context )->update_message_fua );

        XI_SAFE_FREE( *context );
    }

    return XI_STATE_OK;
}

xi_state_t xi_sft_on_connected( xi_sft_context_t* context )
{
    // printf( "%s, updateable_files_count: %d\n", __FUNCTION__,
    //         context->updateable_files_count );

    /* todo_atigyi: commit after a self check, also find a place where new FW can be
     * denied */
    xi_bsp_fwu_commit();

    if ( NULL == context )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_control_message_t* message_file_info = xi_control_message_create_file_info(
        context->updateable_files, NULL /* todo_atigyi: handle revisions */,
        context->updateable_files_count );

    ( *context->fn_send_message )( context->send_message_user_data, message_file_info );

    return XI_STATE_OK;
}

xi_state_t
xi_sft_on_message( xi_sft_context_t* context, xi_control_message_t* sft_message_in )
{
    if ( NULL == context || NULL == sft_message_in )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t state = XI_STATE_OK;

    switch ( sft_message_in->common.msgtype )
    {
        case XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE:
        {
            /* todo?: check whether device is ready to start download of file */

            if ( NULL != context->update_message_fua )
            {
                xi_control_message_free( &context->update_message_fua );
            }

            context->update_message_fua = sft_message_in;
            /* prevent deallocation */
            sft_message_in = NULL;

            /* SFT flow: start file download with the first file in list */
            if ( 0 < context->update_message_fua->file_update_available.list_len &&
                 NULL != context->update_message_fua->file_update_available.list )
            {
                context->update_current_file =
                    &context->update_message_fua->file_update_available.list[0];

                xi_control_message_t* message_file_get_chunk =
                    xi_control_message_create_file_get_chunk(
                        context->update_current_file->name,
                        context->update_current_file->revision, 0,
                        XI_MIN( XI_SFT_FILE_CHUNK_SIZE,
                                context->update_current_file->size_in_bytes ) );

                ( *context->fn_send_message )( context->send_message_user_data,
                                               message_file_get_chunk );
            }
        }
        break;
        case XI_CONTROL_MESSAGE_SC__SFT_FILE_CHUNK:
        {
            /*
             *  - store the chunk through file/firmware BSP
             *  - if last chunk close file, start appropriate action
             *      - if firmware: start firmware update or "just" report downloaded
             *        and wait for remote FWU trigger
             *       - other files may have their own action, no-operation is an option
             */

            /* check whether FILE_CHUNK's filename matches the requested filename */
            if ( NULL != context->update_current_file &&
                 NULL != context->update_current_file->name &&
                 NULL != sft_message_in->file_chunk.name &&
                 0 == strcmp( context->update_current_file->name,
                              sft_message_in->file_chunk.name ) )
            {
                const uint32_t all_downloaded_bytes =
                    sft_message_in->file_chunk.offset + sft_message_in->file_chunk.length;

#if 0
                printf( "         === === === downloading file: %s, %d / %d, [%d%%], "
                        "status: %d\n",
                        context->update_current_file->name,
                        context->update_current_file->size_in_bytes, all_downloaded_bytes,
                        ( all_downloaded_bytes * 100 ) /
                            context->update_current_file->size_in_bytes,
                        sft_message_in->file_chunk.status );
#endif

                { /* processing content */
                    if ( 0 == sft_message_in->file_chunk.offset )
                    {
                        /* open file at first chunk */
                        state = xi_bsp_io_fs_open(
                            sft_message_in->file_chunk.name,
                            context->update_current_file->size_in_bytes, XI_FS_OPEN_WRITE,
                            &context->update_file_handle );

                        // printf( " --- %s, open, filename: %s, state: %d, handle:
                        // %lu\n",
                        //         __FUNCTION__, sft_message_in->file_chunk.name, state,
                        //         context->update_file_handle );

                        if ( 1 ==
                             xi_bsp_fwu_is_firmware( sft_message_in->file_chunk.name ) )
                        {
                            context->firmware_in_update_package = 1;
                        }
                        else if ( XI_STATE_OK != state )
                        {
                            /* ERROR: report error file status, do not send FILE_GET_CHUNK
                             */
                        }
                    }

                    size_t bytes_written = 0;
                    /* pass bytes to FILE BSP - write bytes */
                    state = xi_bsp_io_fs_write(
                        context->update_file_handle, sft_message_in->file_chunk.chunk,
                        sft_message_in->file_chunk.length,
                        sft_message_in->file_chunk.offset, &bytes_written );

                    if ( 0 == sft_message_in->file_chunk.offset )
                    {
                        // printf( " --- %s, write, state: %d\n", __FUNCTION__, state );
                    }

                    if ( XI_STATE_OK != state )
                    {
                        /* ERROR: report error file status, do not send FILE_GET_CHUNK */
                    }
                }

                /* Secure File Transfer flow management */
                if ( all_downloaded_bytes < context->update_current_file->size_in_bytes )
                {
                    /* SFT flow: file is not downloaded yet, continue with this file */

                    xi_control_message_t* message_file_get_chunk =
                        xi_control_message_create_file_get_chunk(
                            context->update_current_file->name,
                            context->update_current_file->revision, all_downloaded_bytes,
                            XI_MIN( XI_SFT_FILE_CHUNK_SIZE,
                                    context->update_current_file->size_in_bytes -
                                        all_downloaded_bytes /* == bytes left */ ) );

                    ( *context->fn_send_message )( context->send_message_user_data,
                                                   message_file_get_chunk );
                }
                else
                {
                    /* SFT flow: file downloaded, continue with next file in list */

                    state = xi_bsp_io_fs_close( context->update_file_handle );
                    context->update_file_handle = XI_FS_INVALID_RESOURCE_HANDLE;
                    // printf( " --- %s, close, state: %d\n", __FUNCTION__, state );

                    if ( XI_STATE_OK != state )
                    {
                        /* ERROR during file close, what to do? */
                    }

                    { /* temporary: report file status messages */
                        xi_control_message_t* message_file_status =
                            xi_control_message_create_file_status(
                                context->update_current_file->name,
                                context->update_current_file->revision,
                                XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
                                XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );

                        ( *context->fn_send_message )( context->send_message_user_data,
                                                       message_file_status );

                        message_file_status = xi_control_message_create_file_status(
                            context->update_current_file->name,
                            context->update_current_file->revision,
                            XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_PROCESSING,
                            XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );

                        ( *context->fn_send_message )( context->send_message_user_data,
                                                       message_file_status );

                        message_file_status = xi_control_message_create_file_status(
                            context->update_current_file->name,
                            context->update_current_file->revision,
                            XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_FINISHED,
                            XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );

                        ( *context->fn_send_message )( context->send_message_user_data,
                                                       message_file_status );
                    }

                    context->update_current_file =
                        xi_control_message_file_update_available_get_next_file_desc_ext(
                            &context->update_message_fua->file_update_available,
                            sft_message_in->file_chunk.name );

                    if ( NULL != context->update_current_file )
                    {
                        xi_control_message_t* message_file_get_chunk =
                            xi_control_message_create_file_get_chunk(
                                context->update_current_file->name,
                                context->update_current_file->revision, 0,
                                XI_MIN( XI_SFT_FILE_CHUNK_SIZE,
                                        context->update_current_file->size_in_bytes ) );

                        ( *context->fn_send_message )( context->send_message_user_data,
                                                       message_file_get_chunk );
                    }
                    else
                    {
                        /* no further files to download, finished with download process */
                        xi_control_message_free( &context->update_message_fua );

                        /* if there was firmware in the update package, then try to
                         * execute it */
                        if ( 1 == context->firmware_in_update_package )
                        {
                            xi_bsp_fwu_test();

                            xi_bsp_fwu_reboot();
                        }
                    }
                }
            }
            else
            {
                /* Something went wrong. Somehow the current file under update
                 * is not in sync with arrived FILE_CHUNK message. */

                xi_control_message_t* message_file_status =
                    xi_control_message_create_file_status(
                        context->update_current_file->name,
                        context->update_current_file->revision,
                        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
                        XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__UNEXPECTED_FILE_CHUNK_ARRIVED );

                ( *context->fn_send_message )( context->send_message_user_data,
                                               message_file_status );

                xi_debug_format( "ERROR: context->update_current_file is out of sync. "
                                 "[%p] [%s]. Dropping this FILE_CHUNK message, waiting "
                                 "for the proper one...",
                                 context->update_current_file,
                                 context->update_current_file
                                     ? context->update_current_file->name
                                     : "n/a" );
            }
        }
        break;

        default:

            xi_debug_format( "WARNING: unhandled incoming SFT message with type: %d",
                             sft_message_in->common.msgtype );
    }

    xi_control_message_free( &sft_message_in );

    return state;
}
