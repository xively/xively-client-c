/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_fs_header.h"
#include "xi_macros.h"
#include "xi_bsp_io_fs.h"
#include "xi_fs_bsp_to_xi_mapping.h"

const size_t xi_fs_buffer_size = 1024;

xi_state_t xi_fs_stat( const void* context,
                       const xi_fs_resource_type_t resource_type,
                       const char* const resource_name,
                       xi_fs_stat_t* resource_stat )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_type );

    if ( NULL == resource_stat || NULL == resource_name )
    {
        return XI_INVALID_PARAMETER;
    }
    
    xi_bsp_io_fs_stat_t bsp_io_fs_resource_stat;
    xi_state_t result = xi_fs_bsp_io_fs_2_xi_state( xi_bsp_io_fs_stat( resource_name, &bsp_io_fs_resource_stat ) );
    if( XI_STATE_OK == result )
    {
        resource_stat->resource_size = bsp_io_fs_resource_stat.resource_size;
    }

    return result;
}

xi_state_t xi_fs_open( const void* context,
                       const xi_fs_resource_type_t resource_type,
                       const char* const resource_name,
                       const xi_fs_open_flags_t open_flags,
                       xi_fs_resource_handle_t* resource_handle )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_type );

    return xi_fs_bsp_io_fs_2_xi_state( xi_bsp_io_fs_open( resource_name, 0 /* not used in POSIX version */,
                                      (xi_bsp_io_fs_open_flags_t) open_flags, resource_handle ) );
}

xi_state_t xi_fs_read( const void* context,
                       const xi_fs_resource_handle_t resource_handle,
                       const size_t offset,
                       const uint8_t** buffer,
                       size_t* const buffer_size )
{
    XI_UNUSED( context );

    return xi_fs_bsp_io_fs_2_xi_state( xi_bsp_io_fs_read( resource_handle, offset, buffer, buffer_size ) );
}

xi_state_t xi_fs_write( const void* context,
                        const xi_fs_resource_handle_t resource_handle,
                        const uint8_t* const buffer,
                        const size_t buffer_size,
                        const size_t offset,
                        size_t* const bytes_written )
{
    XI_UNUSED( context );

    return xi_fs_bsp_io_fs_2_xi_state( xi_bsp_io_fs_write( resource_handle, buffer, buffer_size, offset,
                                                           bytes_written ) );
}

xi_state_t
xi_fs_close( const void* context, const xi_fs_resource_handle_t resource_handle )
{
    XI_UNUSED( context );

    return xi_fs_bsp_io_fs_2_xi_state( xi_bsp_io_fs_close( resource_handle ) );
}

xi_state_t xi_fs_remove( const void* context,
                         const xi_fs_resource_type_t resource_type,
                         const char* const resource_name )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_type );

    return xi_fs_bsp_io_fs_2_xi_state( xi_bsp_io_fs_remove( resource_name ) );
}
