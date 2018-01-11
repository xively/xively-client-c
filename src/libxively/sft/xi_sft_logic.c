/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
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
#include <xi_sft_logic_internal_methods.h>

xi_state_t xi_sft_make_context( xi_sft_context_t** context,
                                const char** updateable_files,
                                uint16_t updateable_files_count,
                                fn_send_control_message_t fn_send_message,
                                xi_sft_url_handler_callback_t* sft_url_handler_callback,
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

    ( *context )->fn_send_message          = fn_send_message;
    ( *context )->send_message_user_data   = user_data;
    ( *context )->updateable_files         = updateable_files;
    ( *context )->updateable_files_count   = updateable_files_count;
    ( *context )->update_message_fua       = NULL;
    ( *context )->update_current_file      = NULL;
    ( *context )->update_firmware          = NULL;
    ( *context )->update_file_handle       = XI_BSP_IO_FS_INVALID_RESOURCE_HANDLE;
    ( *context )->sft_url_handler_callback = sft_url_handler_callback;
    ( *context )->checksum_context         = NULL;

    return state;

err_handling:
    xi_sft_free_context( context );

    return state;
}

xi_state_t xi_sft_free_context( xi_sft_context_t** context )
{
    if ( NULL != context && NULL != *context )
    {
        xi_sft_on_message_file_chunk_checksum_final( *context );

        xi_bsp_io_fs_close( ( *context )->update_file_handle );

        xi_control_message_free( &( *context )->update_message_fua );
        XI_SAFE_FREE( ( *context )->updateable_files_download_order );
        XI_SAFE_FREE( *context );
    }

    return XI_STATE_OK;
}

xi_state_t xi_sft_on_connected( xi_sft_context_t* context )
{
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

    if ( NULL != context->fn_send_message )
    {
        xi_control_message_t* message_file_info = xi_control_message_create_file_info(
            context->updateable_files, context->updateable_files_count,
            ( NULL == context->sft_url_handler_callback ) ? 0 : 1 );

        ( *context->fn_send_message )( context->send_message_user_data,
                                       message_file_info );
    }

    return state;
}

xi_state_t xi_sft_on_connection_failed( xi_sft_context_t* context )
{
    XI_UNUSED( context );

    xi_bsp_fwu_on_new_firmware_failure();

    return XI_STATE_OK;
}

static xi_state_t _xi_sft_order_resource_downloads( xi_sft_context_t* context )
{
    xi_state_t state = XI_STATE_OK;
    uint16_t i       = 0; /* forward declaration to suppress toolchain warnings */

    const uint16_t num_incoming_resources =
        context->update_message_fua->file_update_available.list_len;

    XI_ALLOC_BUFFER( char*, resource_names, sizeof( char* ) * num_incoming_resources,
                     state );

    XI_SAFE_FREE( context->updateable_files_download_order );
    XI_ALLOC_BUFFER_AT( int32_t, context->updateable_files_download_order,
                        sizeof( int32_t ) * num_incoming_resources, state );

    for ( i = 0; i < num_incoming_resources; ++i )
    {
        context->updateable_files_download_order[i] = i;
        resource_names[i] =
            context->update_message_fua->file_update_available.list[i].name;
    }

    xi_bsp_fwu_order_resource_downloads( ( const char* const* )resource_names,
                                         num_incoming_resources,
                                         context->updateable_files_download_order );

    XI_SAFE_FREE( resource_names );

    /* Check for index out of bounds */
    for ( i = 0; i < num_incoming_resources; ++i )
    {
        XI_CHECK_CND(
            ( 0 > context->updateable_files_download_order[i] ||
              num_incoming_resources <= context->updateable_files_download_order[i] ),
            XI_ELEMENT_NOT_FOUND, state );
    }

    return state;

err_handling:
    XI_SAFE_FREE( context->updateable_files_download_order );
    XI_SAFE_FREE( resource_names );
    return state;
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
                state = _xi_sft_order_resource_downloads( context );
                XI_CHECK_STATE( state );

                _xi_sft_continue_package_download( context );
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
                        _xi_sft_send_file_status(
                            context, NULL,
                            XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_PROCESSING,
                            chunk_handling_status_code );

                        xi_debug_logger( "Error: File chunk handling failed" );
                        xi_bsp_fwu_on_package_download_failure();

                        goto err_handling;
                    }
                }

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

                /* Secure File Transfer (SFT) flow management */
                if ( all_downloaded_bytes < context->update_current_file->size_in_bytes )
                {
                    /* SFT flow: file is not downloaded yet, continue with this file */

                    _xi_sft_send_file_get_chunk(
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

                        _xi_sft_send_file_status(
                            context, NULL,
                            XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
                            checksum_status_code );

                        if ( XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS !=
                             checksum_status_code )
                        {
                            xi_debug_logger( "Error: File checksum validation failed" );
                            xi_bsp_fwu_on_package_download_failure();
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
                            _xi_sft_send_file_status(
                                context, NULL,
                                XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_PROCESSING,
                                XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__FILE_CLOSE );

                            goto err_handling;
                        }
                        else
                        {
                            _xi_sft_current_file_revision_handling( context );
                        }
                    }

                    _xi_sft_continue_package_download( context );
                }
            }
            else
            {
                /* Something went wrong. Somehow the current file under update
                 * is not in sync with arrived FILE_CHUNK message. */

                _xi_sft_send_file_status(
                    context, NULL, XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
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
    if ( XI_STATE_OK != state )
    {
        xi_debug_format( "WARNING: SFT encountered invalid state: %d", state );
    }

    return state;
}
