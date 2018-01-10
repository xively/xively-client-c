/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_fs_header.h"
#include "xi_macros.h"

xi_state_t xi_fs_stat( const void* context,
                       const xi_fs_resource_type_t resource_type,
                       const char* const resource_name,
                       xi_fs_stat_t* resource_stat )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_type );
    XI_UNUSED( resource_name );
    XI_UNUSED( resource_stat );

    return XI_FS_RESOURCE_NOT_AVAILABLE;
}

xi_state_t xi_fs_open( const void* context,
                       const xi_fs_resource_type_t resource_type,
                       const char* const resource_name,
                       const xi_fs_open_flags_t open_flags,
                       xi_fs_resource_handle_t* resource_handle )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_type );
    XI_UNUSED( resource_name );
    XI_UNUSED( open_flags );
    XI_UNUSED( resource_handle );

    return XI_FS_ERROR;
}

xi_state_t xi_fs_read( const void* context,
                       const xi_fs_resource_handle_t resource_handle,
                       const size_t offset,
                       const uint8_t** buffer,
                       size_t* const buffer_size )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_handle );
    XI_UNUSED( offset );
    XI_UNUSED( buffer );
    XI_UNUSED( buffer_size );

    return XI_FS_ERROR;
}

xi_state_t xi_fs_write( const void* context,
                        const xi_fs_resource_handle_t resource_handle,
                        const uint8_t* const buffer,
                        const size_t buffer_size,
                        const size_t offset,
                        size_t* const bytes_written )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_handle );
    XI_UNUSED( buffer );
    XI_UNUSED( buffer_size );
    XI_UNUSED( offset );
    XI_UNUSED( bytes_written );

    return XI_FS_ERROR;
}

xi_state_t
xi_fs_close( const void* context, const xi_fs_resource_handle_t resource_handle )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_handle );

    return XI_FS_ERROR;
}

xi_state_t xi_fs_remove( const void* context,
                         const xi_fs_resource_type_t resource_type,
                         const char* const resource_name )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_type );
    XI_UNUSED( resource_name );

    return XI_FS_RESOURCE_NOT_AVAILABLE;
}
