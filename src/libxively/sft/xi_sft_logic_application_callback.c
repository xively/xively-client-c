/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_sft_logic_application_callback.h"

#include <xi_sft_logic.h>
#include <stdio.h>
#include <xi_sft_logic.h>
#include <xi_globals.h>
#include <xi_handle.h>

#include <xi_sft_logic_internal_methods.h>

xi_state_t
_xi_sft_on_file_downloaded_task( void* sft_context_void,
                                 void* filename_void,
                                 xi_state_t state,
                                 void* flag_download_finished_successfully_void )
{
    XI_UNUSED( state );

    xi_sft_context_t* context = ( xi_sft_context_t* )sft_context_void;
    const char* filename      = ( const char* )filename_void;
    const uint8_t flag_download_finished_successfully =
        ( intptr_t )flag_download_finished_successfully_void;


    if ( 0 != flag_download_finished_successfully )
    {
        if ( NULL != context->update_current_file &&
             NULL != context->update_current_file->name && NULL != filename &&
             0 == strcmp( context->update_current_file->name, filename ) )
        {
            _xi_sft_send_file_status(
                context, NULL, XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
                XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS );

            _xi_sft_current_file_revision_handling( context );
            _xi_sft_continue_package_download( context );
        }
        else
        {
            _xi_sft_send_file_status(
                context, NULL, XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
                XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__URLDL_UNEXPECTED_FILE_NAME );
        }
    }
    else
    {
        _xi_sft_send_file_status(
            context, NULL, XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED,
            XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__URLDL_FAILED );

        /* fallback to MQTT: starting the internal MQTT file download process */
        _xi_sft_send_file_get_chunk( context, 0,
                                     context->update_current_file->size_in_bytes );
    }

    return XI_STATE_OK;
}

void xi_sft_on_file_downloaded_application_callback(
    void* sft_context_void,
    const char* filename,
    uint8_t flag_download_finished_successfully )
{
    printf( "[ LIBXIVELY   ] - %s, sft_context_void: %p, filename: %s, success: %x\n",
            __FUNCTION__, sft_context_void, filename,
            flag_download_finished_successfully );

    xi_evtd_execute(
        xi_globals.evtd_instance,
        xi_make_handle( &_xi_sft_on_file_downloaded_task, ( void* )sft_context_void,
                        ( void* )filename, XI_STATE_OK,
                        ( void* )( intptr_t )flag_download_finished_successfully ) );
}
