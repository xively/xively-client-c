/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_sft_logic.h>
#include <stddef.h>
#include <xi_macros.h>
#include <xi_debug.h>

#include <xi_sft_revision.h>
#include <xi_sft_logic_file_chunk_handlers.h>
#include <xi_control_message_sft.h>

#include <xi_bsp_io_fs.h>
#include <xi_bsp_fwu.h>

#include <xi_fs_bsp_to_xi_mapping.h>

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
    ( *context )->update_message_fua     = NULL;
    ( *context )->update_current_file    = NULL;
    ( *context )->update_firmware        = NULL;
    ( *context )->update_file_handle     = 0;
    ( *context )->checksum_context       = NULL;

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
    /* todo_atigyi: commit after a self check, also find a places where new FW can be
     * denied */

    xi_state_t state = XI_STATE_OK;

    /* emit a Firmware OK notification towards the BSP module, and depending
     * on the return value, whether this is the first run of the new firmware
     * update the firmware revision */
    if ( XI_BSP_FWU_ACTUAL_COMMIT_HAPPENED == xi_bsp_fwu_on_new_firmware_ok() )
    {
        xi_sft_revision_firmware_ok();
    }

    if ( NULL == context )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_control_message_t* message_file_info = xi_control_message_create_file_info(
        context->updateable_files, context->updateable_files_count );

    ( *context->fn_send_message )( context->send_message_user_data, message_file_info );

    return state;
}

xi_state_t xi_sft_on_connection_failed( xi_sft_context_t* context )
{
    XI_UNUSED( context );

    xi_bsp_fwu_on_new_firmware_failure();

    return XI_STATE_OK;
}

static void
xi_sft_send_file_get_chunk( xi_sft_context_t* context, uint32_t offset, uint32_t length )
{
    xi_control_message_t* message_file_get_chunk =
        xi_control_message_create_file_get_chunk(
            context->update_current_file->name, context->update_current_file->revision,
            offset, XI_MIN( XI_SFT_FILE_CHUNK_SIZE, length ) );

    ( *context->fn_send_message )( context->send_message_user_data,
                                   message_file_get_chunk );
}

static void
xi_sft_send_file_status( const xi_sft_context_t* context,
                         const xi_control_message_file_desc_ext_t* file_desc_ext,
                         xi_control_message__sft_file_status_phase_t phase,
                         xi_control_message__sft_file_status_code_t code )
{
    xi_control_message_t* message_file_status = xi_control_message_create_file_status(
        file_desc_ext ? file_desc_ext->name : context->update_current_file->name,
        file_desc_ext ? file_desc_ext->revision : context->update_current_file->revision,
        phase, code );

    ( *context->fn_send_message )( context->send_message_user_data, message_file_status );
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

            /* passing memory ownership */
            context->update_message_fua = sft_message_in;
            /* prevent deallocation */
            sft_message_in = NULL;

            /* SFT flow: start file download with the first file in list */
            if ( 0 < context->update_message_fua->file_update_available.list_len &&
                 NULL != context->update_message_fua->file_update_available.list )
            {
                context->update_current_file =
                    &context->update_message_fua->file_update_available.list[0];

                xi_sft_send_file_get_chunk( context, 0,
                                            context->update_current_file->size_in_bytes );
            }
        }
        break;

        case XI_CONTROL_MESSAGE_SC__SFT_FILE_CHUNK:
        {
            /* check whether FILE_CHUNK's filename matches the requested filename */
            if ( NULL != context->update_current_file &&
                 NULL != context->update_current_file->name &&
                 NULL != sft_message_in->file_chunk.name &&
                 0 == strcmp( context->update_current_file->name,
                              sft_message_in->file_chunk.name ) )
            {
                /* process file chunk */
                {
                    const xi_control_message__sft_file_status_code_t
                        chunk_handling_status_code =
                            xi_sft_on_message_file_chunk_process_file_chunk(
                                context, sft_message_in );

                    /* error handling */
                    if ( XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS !=
                         chunk_handling_status_code )
                    {
                        xi_sft_send_file_status(
                            context, NULL,
                            XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_PROCESSING,
                            chunk_handling_status_code );

                        xi_bsp_fwu_on_package_download_failure();

                        goto err_handling;
                    }
                }

                const uint32_t all_downloaded_bytes =
                    sft_message_in->file_chunk.offset + sft_message_in->file_chunk.length;

#if 1
                printf( "         === === === downloading file: %s, %d / %d, [%d%%], "
                        "status: %d\n",
                        context->update_current_file->name,
                        context->update_current_file->size_in_bytes, all_downloaded_bytes,
                        ( all_downloaded_bytes * 100 ) /
                            context->update_current_file->size_in_bytes,
                        sft_message_in->file_chunk.status );
#endif

                /* Secure File Transfer (SFT) flow management */
                if ( all_downloaded_bytes < context->update_current_file->size_in_bytes )
                {
                    /* SFT flow: file is not downloaded yet, continue with this file */

                    xi_sft_send_file_get_chunk(
                        context, all_downloaded_bytes,
                        context->update_current_file->size_in_bytes -
                            all_downloaded_bytes );
                }
                else
                {
                    /* SFT flow: file downloaded, checksum handling, continue with next
                     * file in list */

                    /* checksum handling */
                    {
                        const xi_control_message__sft_file_status_code_t
                            checksum_status_code =
                                xi_sft_on_message_file_chunk_checksum_final( context );

                        xi_sft_send_file_status(
                            context, NULL,
                            XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
                            checksum_status_code );

                        if ( XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS !=
                             checksum_status_code )
                        {
                            xi_bsp_fwu_on_package_download_failure();
                            /* todo_atigyi: another option beyond exiting the whole update
                             * process is to retry the broken file download */
                            goto err_handling;
                        }
                    }

                    /* close file */
                    {
                        state = xi_fs_bsp_io_fs_2_xi_state(
                            xi_bsp_io_fs_close( context->update_file_handle ) );
                        context->update_file_handle =
                            XI_BSP_IO_FS_INVALID_RESOURCE_HANDLE;
                        // printf( " --- %s, close, state: %d\n", __FUNCTION__, state );

                        if ( XI_STATE_OK != state )
                        {
                            xi_sft_send_file_status(
                                context, NULL,
                                XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_PROCESSING,
                                XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__FILE_CLOSE );

                            // what to do here?
                            // downloaded but file close error: abort or continue?
                            // goto err_handling;
                        }
                        else if ( context->update_firmware !=
                                  context->update_current_file )
                        {
                            /* handling non-firmware files */
                            xi_sft_send_file_status(
                                context, NULL,
                                XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_FINISHED,
                                XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );

                            xi_sft_revision_set( context->update_current_file->name,
                                                 context->update_current_file->revision );
                        }
                        else
                        {
                            /* handling firmware file(s) */
                            xi_sft_revision_set_firmware_update(
                                context->update_current_file->name,
                                context->update_current_file->revision );
                        }
                    }

                    /* continue the update package download  */
                    {
                        context->update_current_file =
                            xi_control_message_file_update_available_get_next_file_desc_ext(
                                &context->update_message_fua->file_update_available,
                                sft_message_in->file_chunk.name );

                        if ( NULL != context->update_current_file )
                        {
                            /* continue download with next file */
                            xi_sft_send_file_get_chunk(
                                context, 0, context->update_current_file->size_in_bytes );
                        }
                        else
                        {
                            /* finished with package download */

                            if ( NULL != context->update_firmware )
                            {
                                xi_sft_send_file_status(
                                    context, context->update_firmware,
                                    XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_PROCESSING,
                                    XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
                            }

                            /* report the finish of package download to the application,
                               add firmware name if available */
                            xi_bsp_fwu_on_package_download_finished(
                                ( NULL != context->update_firmware )
                                    ? context->update_firmware->name
                                    : NULL );

                            /* no further files to download, finished with download
                             * process */
                            xi_control_message_free( &context->update_message_fua );
                        }
                    }
                }
            }
            else
            {
                /* Something went wrong. Somehow the current file under update
                 * is not in sync with arrived FILE_CHUNK message. */

                xi_sft_send_file_status(
                    context, NULL, XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_PROCESSING,
                    XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__UNEXPECTED_FILE_CHUNK );

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

err_handling:

    xi_control_message_free( &sft_message_in );

    return state;
}
