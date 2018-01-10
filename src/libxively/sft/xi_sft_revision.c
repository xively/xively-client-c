/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_sft_revision.h"

#include <xi_bsp_io_fs.h>

#include <xi_helpers.h>
#include <xi_macros.h>

#include <string.h>
#include <xi_fs_bsp_to_xi_mapping.h>

#define XI_SFT_REVISION_RESOURCENAME( resource_name )                                    \
    xi_str_cat( resource_name, ".xirev" )

static xi_state_t
_xi_sft_revision_write_string_to_file( const char* const resource_name,
                                       const char* const string_to_write )
{
    if ( NULL == resource_name || NULL == string_to_write )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_bsp_io_fs_resource_handle_t resource_handle = XI_BSP_IO_FS_INVALID_RESOURCE_HANDLE;
    xi_bsp_io_fs_state_t bsp_io_fs_state =
        xi_bsp_io_fs_open( resource_name, strlen( string_to_write ),
                           XI_BSP_IO_FS_OPEN_WRITE, &resource_handle );
    xi_state_t state     = xi_fs_bsp_io_fs_2_xi_state( bsp_io_fs_state );
    size_t bytes_written = 0;

    XI_CHECK_STATE( state );

    bsp_io_fs_state =
        xi_bsp_io_fs_write( resource_handle, ( const uint8_t* )string_to_write,
                            strlen( string_to_write ), 0, &bytes_written );

    state = xi_fs_bsp_io_fs_2_xi_state( bsp_io_fs_state );

    xi_bsp_io_fs_close( resource_handle );

err_handling:

    return state;
}

static xi_state_t _xi_sft_revision_read_string_from_file( const char* const resource_name,
                                                          char** string_read_out )
{
    if ( NULL == resource_name || NULL == string_read_out )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_bsp_io_fs_resource_handle_t resource_handle = XI_BSP_IO_FS_INVALID_RESOURCE_HANDLE;

    xi_bsp_io_fs_state_t bsp_io_fs_state =
        xi_bsp_io_fs_open( resource_name, 0 /* not used at READ */,
                           XI_BSP_IO_FS_OPEN_READ, &resource_handle );

    const uint8_t* buffer = NULL;
    size_t buffer_size    = 0;
    xi_state_t state      = xi_fs_bsp_io_fs_2_xi_state( bsp_io_fs_state );

    XI_CHECK_STATE( state );

    /* note: single read call "only" supports revision not longer than file read buffer.
     * At time of writing this it's 1024 bytes. */
    bsp_io_fs_state = xi_bsp_io_fs_read( resource_handle, 0, &buffer, &buffer_size );
    state           = xi_fs_bsp_io_fs_2_xi_state( bsp_io_fs_state );

    XI_CHECK_STATE( state );

    /* allocate output buffer then copy revision buffer into output buffer */
    XI_ALLOC_BUFFER_AT( char, *string_read_out, buffer_size + 1, state );

    memcpy( *string_read_out, buffer, buffer_size );

err_handling:

    xi_bsp_io_fs_close( resource_handle );

    return state;
}

xi_state_t
xi_sft_revision_set( const char* const resource_name, const char* const revision )
{
    char* resource_name_revision = XI_SFT_REVISION_RESOURCENAME( resource_name );

    const xi_state_t state =
        _xi_sft_revision_write_string_to_file( resource_name_revision, revision );

    XI_SAFE_FREE( resource_name_revision );

    return state;
}

xi_state_t xi_sft_revision_get( const char* const resource_name, char** revision_out )
{
    char* resource_name_revision = XI_SFT_REVISION_RESOURCENAME( resource_name );

    const xi_state_t state =
        _xi_sft_revision_read_string_from_file( resource_name_revision, revision_out );

    XI_SAFE_FREE( resource_name_revision );

    return state;
}

#define XI_SFT_REVISION_FIRMWAREUPDATEREVISION_MAILBOX_TO_NEXT_RUN                       \
    "firmware_update_revision.mailbox"

xi_state_t
xi_sft_revision_set_firmware_update( const char* const xi_firmware_resource_name,
                                     const char* const xi_firmware_revision )
{
    if ( NULL == xi_firmware_resource_name || NULL == xi_firmware_revision )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t state = XI_STATE_OK;

    XI_ALLOC_BUFFER( char, firmware_update_data,
                     strlen( xi_firmware_resource_name ) +
                         strlen( xi_firmware_revision ) + 2 /* newline + trailing zero */,
                     state );

    sprintf( firmware_update_data, "%s\n%s", xi_firmware_resource_name,
             xi_firmware_revision );

    /* "mailing" firmware update data for the new firmware run after device reboot */
    state = _xi_sft_revision_write_string_to_file(
        XI_SFT_REVISION_FIRMWAREUPDATEREVISION_MAILBOX_TO_NEXT_RUN,
        firmware_update_data );

err_handling:

    XI_SAFE_FREE( firmware_update_data );

    return XI_STATE_OK;
}

xi_state_t xi_sft_revision_firmware_ok()
{
    char* firmware_update_data = NULL;

    /* reading "mailbox" expected containing currently running (this) firmware data */
    const xi_state_t state = _xi_sft_revision_read_string_from_file(
        XI_SFT_REVISION_FIRMWAREUPDATEREVISION_MAILBOX_TO_NEXT_RUN,
        &firmware_update_data );

    char* xi_firmware_resource_name = firmware_update_data;
    char* xi_firmware_revision      = firmware_update_data;

    XI_CHECK_STATE( state );

    while ( '\n' != *xi_firmware_revision && 0 != *xi_firmware_revision )
    {
        ++xi_firmware_revision;
    }

    if ( '\n' == *xi_firmware_revision )
    {
        /* replace newline with string terminator zero */
        *xi_firmware_revision = 0;

        /* step to next character which is the beginning of the firmware revision */
        ++xi_firmware_revision;

        xi_sft_revision_set( xi_firmware_resource_name, xi_firmware_revision );
    }

    xi_bsp_io_fs_remove( XI_SFT_REVISION_FIRMWAREUPDATEREVISION_MAILBOX_TO_NEXT_RUN );

    XI_SAFE_FREE( firmware_update_data );

err_handling:

    return state;
}
