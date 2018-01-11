/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_FS_HEADER_H__
#define __XI_FS_HEADER_H__

#include "xi_fs_api.h"
#include "xi_err.h"

xi_state_t xi_fs_stat( const void* context,
                       const xi_fs_resource_type_t resource_type,
                       const char* const resource_name,
                       xi_fs_stat_t* resource_stat );

xi_state_t xi_fs_open( const void* context,
                       const xi_fs_resource_type_t resource_type,
                       const char* const resource_name,
                       const xi_fs_open_flags_t open_flags,
                       xi_fs_resource_handle_t* resource_handle );

xi_state_t xi_fs_read( const void* context,
                       const xi_fs_resource_handle_t resource_handle,
                       const size_t offset,
                       const uint8_t** buffer,
                       size_t* const buffer_size );

xi_state_t xi_fs_write( const void* context,
                        const xi_fs_resource_handle_t resource_handle,
                        const uint8_t* const buffer,
                        const size_t buffer_size,
                        const size_t offset,
                        size_t* const bytes_written );

xi_state_t
xi_fs_close( const void* context, const xi_fs_resource_handle_t resource_handle );

xi_state_t xi_fs_remove( const void* context,
                         const xi_fs_resource_type_t resource_type,
                         const char* const resource_name );

#endif /* __XI_FS_HEADER_H__ */
