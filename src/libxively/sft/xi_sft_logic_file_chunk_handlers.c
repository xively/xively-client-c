/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_sft_logic_file_chunk_handlers.h"
#include <xi_sft_logic.h>
#include <string.h>

#include <xi_bsp_io_fs.h>
#include <xi_bsp_fwu.h>
#include <xi_fs_bsp_to_xi_mapping.h>

xi_control_message__sft_file_status_code_t
xi_sft_on_message_file_chunk_process_file_chunk( xi_sft_context_t* context,
                                                 xi_control_message_t* sft_message_in )
{
    xi_state_t state = XI_STATE_OK;

    /* at first chunk open file */
    if ( 0 == sft_message_in->file_chunk.offset )
    {
        state = xi_fs_bsp_io_fs_2_xi_state( xi_bsp_io_fs_open(
            sft_message_in->file_chunk.name, context->update_current_file->size_in_bytes,
            XI_BSP_IO_FS_OPEN_WRITE, &context->update_file_handle ) );

        if ( XI_STATE_OK != state )
        {
            return XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__FILE_OPEN;
        }

        xi_bsp_fwu_checksum_init( &context->checksum_context );
    }

    /* write bytes through FILE BSP */
    size_t bytes_written = 0;

    state = xi_fs_bsp_io_fs_2_xi_state(
        xi_bsp_io_fs_write( context->update_file_handle, sft_message_in->file_chunk.chunk,
                            sft_message_in->file_chunk.length,
                            sft_message_in->file_chunk.offset, &bytes_written ) );

    if ( XI_STATE_OK != state )
    {
        return XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__FILE_WRITE;
    }

    xi_bsp_fwu_checksum_update( context->checksum_context,
                                sft_message_in->file_chunk.chunk,
                                sft_message_in->file_chunk.length );

    return XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS;
}

xi_control_message__sft_file_status_code_t
xi_sft_on_message_file_chunk_checksum_final( xi_sft_context_t* context )
{
    if ( NULL == context || NULL == context->update_current_file ||
         NULL == context->checksum_context )
    {
        return XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__FILE_CHECKSUM_MISMATCH;
    }

    uint8_t* locally_calculated_fingerprint     = NULL;
    uint16_t locally_calculated_fingerprint_len = 0;

    xi_bsp_fwu_checksum_final( &context->checksum_context,
                               &locally_calculated_fingerprint,
                               &locally_calculated_fingerprint_len );

    /* integrity check based on checksum values */
    if ( context->update_current_file->fingerprint_len !=
             locally_calculated_fingerprint_len ||
         0 != memcmp( context->update_current_file->fingerprint,
                      locally_calculated_fingerprint,
                      locally_calculated_fingerprint_len ) )
    {
        /* checksum mismatch */
        return XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__FILE_CHECKSUM_MISMATCH;
    }
    else
    {
        /* checksum OK */
        return XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS;
    }
}
