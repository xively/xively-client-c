/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_BSP_IO_FS_H__
#define __XI_BSP_IO_FS_H__

#include <stdlib.h>
#include <xively_error.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef intptr_t xi_fs_resource_handle_t;

#define XI_FS_INVALID_RESOURCE_HANDLE -1
#define xi_fs_init_resource_handle() XI_FS_INVALID_RESOURCE_HANDLE

/**
 * @enum xi_fs_resource_type_t
 * @brief describes types of resources that are availible through fs API the
 * types were created in order to differenciate types based on their security
 * class
 */
typedef enum {
    XI_FS_CERTIFICATE = 0,
    XI_FS_CREDENTIALS,
    XI_FS_CONFIG_DATA
} xi_fs_resource_type_t;

/*
 * @name xi_fs_stat_s
 * @brief information returned via stat call
 */
typedef struct xi_fs_stat_s
{
    size_t resource_size;
} xi_fs_stat_t;

/**
 * @enum xi_fs_open_flags_t
 */
typedef enum {
    XI_FS_OPEN_READ   = 1 << 0,
    XI_FS_OPEN_WRITE  = 1 << 1,
    XI_FS_OPEN_APPEND = 1 << 2,
} xi_fs_open_flags_t;

xi_state_t xi_bsp_io_fs_stat( const xi_fs_resource_type_t resource_type,
                              const char* const resource_name,
                              xi_fs_stat_t* resource_stat );

xi_state_t xi_bsp_io_fs_open( const xi_fs_resource_type_t resource_type,
                              const char* const resource_name,
                              const xi_fs_open_flags_t open_flags,
                              xi_fs_resource_handle_t* resource_handle );

xi_state_t xi_bsp_io_fs_read( const xi_fs_resource_handle_t resource_handle,
                              const size_t offset,
                              const uint8_t** buffer,
                              size_t* const buffer_size );

xi_state_t xi_bsp_io_fs_write( const xi_fs_resource_handle_t resource_handle,
                               const uint8_t* const buffer,
                               const size_t buffer_size,
                               const size_t offset,
                               size_t* const bytes_written );

xi_state_t xi_bsp_io_fs_close( const xi_fs_resource_handle_t resource_handle );

xi_state_t xi_bsp_io_fs_remove( const xi_fs_resource_type_t resource_type,
                                const char* const resource_name );

#ifdef __cplusplus
}
#endif

#endif /* __XI_BSP_IO_FS_H__ */
