/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_fwu.h>
#include <stdio.h>
#include <string.h>
#include <xi_bsp_mem.h>
#include <xi_bsp_io_fs.h>
#include <xi_helpers.h>
#include <xi_macros.h>

uint8_t xi_bsp_fwu_is_this_firmware( const char* const resource_name )
{
    return ( 0 == strcmp( "firmware.bin", resource_name ) ) ? 1 : 0;
}

xi_state_t xi_bsp_fwu_commit()
{
    printf( "--- %s, no operation\n", __FUNCTION__ );
    return XI_STATE_OK;
}

xi_state_t xi_bsp_fwu_test()
{
    printf( "--- %s, no operation\n", __FUNCTION__ );
    return XI_STATE_OK;
}

xi_state_t xi_bsp_fwu_reboot()
{
    printf( "--- %s, no operation\n", __FUNCTION__ );
    return XI_STATE_OK;
}

#define XI_BSP_FWU_GET_REVISION_RESOURCENAME( resource_name )                            \
    xi_str_cat( resource_name, ".xirev" )

xi_state_t
xi_bsp_fwu_set_revision( const char* const resource_name, const char* const revision )
{
    char* resource_name_revision = XI_BSP_FWU_GET_REVISION_RESOURCENAME( resource_name );

    xi_fs_resource_handle_t resource_handle = XI_FS_INVALID_RESOURCE_HANDLE;

    xi_bsp_io_fs_open( resource_name_revision, 0, XI_FS_OPEN_WRITE, &resource_handle );

    size_t bytes_written = 0;

    xi_bsp_io_fs_write( resource_handle, ( const uint8_t* )revision, strlen( revision ),
                        0, &bytes_written );

    xi_bsp_io_fs_close( resource_handle );

    XI_SAFE_FREE( resource_name_revision );

    return XI_STATE_OK;
}

xi_state_t xi_bsp_fwu_get_revision( const char* const resource_name, char** revision_out )
{
    char* resource_name_revision = XI_BSP_FWU_GET_REVISION_RESOURCENAME( resource_name );

    xi_fs_resource_handle_t resource_handle = XI_FS_INVALID_RESOURCE_HANDLE;

    xi_state_t state = xi_bsp_io_fs_open( resource_name_revision, 0 /* not used */,
                                          XI_FS_OPEN_READ, &resource_handle );

    if ( XI_STATE_OK != state )
    {
        goto err_handling;
    }

    const uint8_t* buffer = NULL;
    size_t buffer_size    = 0;

    state = xi_bsp_io_fs_read( resource_handle, 0, &buffer, &buffer_size );

    if ( XI_STATE_OK != state )
    {
        goto err_handling;
    }

    state = xi_bsp_io_fs_close( resource_handle );

    {
        /* copy revision buffer into output buffer */
        const size_t revision_len = buffer_size + 1;

        *revision_out = xi_bsp_mem_alloc( revision_len );

        if ( NULL == *revision_out )
        {
            state = XI_OUT_OF_MEMORY;
            goto err_handling;
        }

        memset( *revision_out, 0, revision_len );

        memcpy( *revision_out, buffer, buffer_size );
    }

err_handling:

    XI_SAFE_FREE( resource_name_revision );

    return state;
}
