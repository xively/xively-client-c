/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_sft_logic_internal_methods.h>
#include <xi_control_message_sft.h>
#include <xi_sft_revision.h>
#include <xi_macros.h>
#include <xi_bsp_fwu.h>
#include <xi_sft_logic_application_callback.h>
#include <xi_debug.h>

#include <stdio.h>

void _xi_sft_send_file_status( const xi_sft_context_t* context,
                               const xi_control_message_file_desc_ext_t* file_desc_ext,
                               xi_control_message__sft_file_status_phase_t phase,
                               xi_control_message__sft_file_status_code_t code )
{
    if ( NULL != context->fn_send_message &&
         ( NULL != file_desc_ext || NULL != context->update_current_file ) )
    {
        xi_control_message_t* message_file_status = xi_control_message_create_file_status(
            file_desc_ext ? file_desc_ext->name : context->update_current_file->name,
            file_desc_ext ? file_desc_ext->revision
                          : context->update_current_file->revision,
            phase, code );

        ( *context->fn_send_message )( context->send_message_user_data,
                                       message_file_status );
    }
}

void _xi_sft_send_file_get_chunk( xi_sft_context_t* context,
                                  uint32_t offset,
                                  uint32_t length )
{
    if ( NULL != context->fn_send_message )
    {
        xi_control_message_t* message_file_get_chunk =
            xi_control_message_create_file_get_chunk(
                context->update_current_file->name,
                context->update_current_file->revision, offset,
                XI_MIN( XI_SFT_FILE_CHUNK_SIZE, length ) );

        ( *context->fn_send_message )( context->send_message_user_data,
                                       message_file_get_chunk );
    }
}

void _xi_sft_download_current_file( xi_sft_context_t* context )
{
    if ( 1 == xi_bsp_fwu_is_this_firmware( context->update_current_file->name ) )
    {
        context->update_firmware = context->update_current_file;
    }

    uint8_t download_started_by_callback = 0;

    /* decide how to download the file, if external function and URL are available,
       then try to download with these */
    if ( NULL != context->update_current_file->download_link &&
         NULL != context->sft_url_handler_callback )
    {
        /* trying to use an application provided callback to download the file */
        download_started_by_callback = ( *context->sft_url_handler_callback )(
            context->update_current_file->download_link,
            context->update_current_file->name,
            xi_sft_on_file_downloaded_application_callback, context );
    }


    if ( 0 != download_started_by_callback )
    {
        /* report DOWNLOADING phase to SFT service, since non-MQTT downloads aren't
           detected by SFT service */
        _xi_sft_send_file_status( context, NULL,
                                  XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADING,
                                  XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
    }
    else if ( NULL == context->update_current_file->download_link ||
              0 != context->update_current_file->flag_mqtt_download_also_supported )
    {
        /* starting the internal MQTT file download process */
        _xi_sft_send_file_get_chunk( context, 0,
                                     context->update_current_file->size_in_bytes );
    }
}

void _xi_sft_current_file_revision_handling( xi_sft_context_t* context )
{
    if ( context->update_firmware != context->update_current_file )
    {
        /* handling non-firmware files */
        _xi_sft_send_file_status( context, NULL,
                                  XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_FINISHED,
                                  XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );

        xi_sft_revision_set( context->update_current_file->name,
                             context->update_current_file->revision );
    }
    else
    {
        /* handling firmware file(s) */
        xi_sft_revision_set_firmware_update( context->update_current_file->name,
                                             context->update_current_file->revision );
    }
}

void _xi_sft_continue_package_download( xi_sft_context_t* context )
{
    xi_state_t state = _xi_sft_select_next_resource_to_download( context );
    XI_CHECK_STATE( state );

    if ( NULL != context->update_current_file )
    {
        _xi_sft_download_current_file( context );
    }
    else
    {
        /* finished with package download */
        if ( NULL != context->update_firmware )
        {
            _xi_sft_send_file_status(
                context, context->update_firmware,
                XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_PROCESSING,
                XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );
        }

        printf( "[ LIBXIVELY    ] - %s, package download finished\n", __FUNCTION__ );

        /* report the finish of package download to the application,
           add firmware name if available */
        xi_bsp_fwu_on_package_download_finished( ( NULL != context->update_firmware )
                                                     ? context->update_firmware->name
                                                     : NULL );

        /* no further files to download, finished with download
         * process */
        xi_control_message_free( &context->update_message_fua );
    }

err_handling:
    if ( XI_STATE_OK != state )
    {
        xi_debug_format( "WARNING: SFT encountered invalid state: %d", state );
    }
}
