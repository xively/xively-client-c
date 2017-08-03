/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_sft_revision.h"

#include <xi_bsp_io_fs.h>

#include <xi_helpers.h>
#include <xi_macros.h>

#include <string.h>


#define XI_SFT_REVISION_RESOURCENAME( resource_name )                                    \
    xi_str_cat( resource_name, ".xirev" )

xi_state_t
xi_sft_revision_set( const char* const resource_name, const char* const revision )
{
    char* resource_name_revision = XI_SFT_REVISION_RESOURCENAME( resource_name );

    xi_fs_resource_handle_t resource_handle = XI_FS_INVALID_RESOURCE_HANDLE;

    xi_state_t state = xi_bsp_io_fs_open( resource_name_revision, strlen( revision ),
                                          XI_FS_OPEN_WRITE, &resource_handle );

    if ( XI_STATE_OK != state )
    {
        goto err_handling;
    }

    size_t bytes_written = 0;

    state = xi_bsp_io_fs_write( resource_handle, ( const uint8_t* )revision,
                                strlen( revision ), 0, &bytes_written );

    if ( XI_STATE_OK != state )
    {
        goto err_handling;
    }

    xi_bsp_io_fs_close( resource_handle );

err_handling:

    XI_SAFE_FREE( resource_name_revision );

    return state;
}

xi_state_t xi_sft_revision_get( const char* const resource_name, char** revision_out )
{
    char* resource_name_revision = XI_SFT_REVISION_RESOURCENAME( resource_name );

    xi_fs_resource_handle_t resource_handle = XI_FS_INVALID_RESOURCE_HANDLE;

    xi_state_t state = xi_bsp_io_fs_open( resource_name_revision, 0 /* not used */,
                                          XI_FS_OPEN_READ, &resource_handle );

    if ( XI_STATE_OK != state )
    {
        goto err_handling;
    }

    const uint8_t* buffer = NULL;
    size_t buffer_size    = 0;

    /* note: single read call "only" supports revision not longer than file read buffer.
     * At time of writing this it's 1024 bytes. */
    state = xi_bsp_io_fs_read( resource_handle, 0, &buffer, &buffer_size );

    if ( XI_STATE_OK != state )
    {
        goto err_handling;
    }

    xi_bsp_io_fs_close( resource_handle );

    /* copy revision buffer into output buffer */
    XI_ALLOC_BUFFER_AT( char, *revision_out, buffer_size + 1, state );

    memcpy( *revision_out, buffer, buffer_size );

err_handling:

    XI_SAFE_FREE( resource_name_revision );

    return state;
}
