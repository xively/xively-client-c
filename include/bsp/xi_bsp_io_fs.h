/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_BSP_IO_FS_H__
#define __XI_BSP_IO_FS_H__

/**
 * @file xi_bsp_io.fs.h
 * @brief Xively Client's Board Support Platform (BSP) for File Access
 *
 * This file defines the File Management API used by the Xively C Client.
 *
 * The Xivley C Client facilitates non-volatile data storage of
 * certificates and Secure File Transfer managed  through these functions.
 * 
 * NOTE: the use of this BSP to store certificates used during the TLSX
 * handshake process is currently an ongoing project. 
 * 
 * All files are referenced by resource_name strings.  For Xively SFT
 * storage, these resource names are the same as the string that was used
 * to name the file in the SFT Package Contents.
 */
#include <stdlib.h>
#include <stdint.h>
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


/**
 * @function
 * @brief Used by the Xively C Client to determine the existance and size of a
 * file.
 *
 * @param [in] resource_name the name of the file to check
 * @param [out] resource_stat a structure to be populated based on the file
 * size data.
 *
 * @return xi_state_t XI_STATE_OK in case of a success and other in case of an
 * error.
 */
xi_state_t
xi_bsp_io_fs_stat( const char* const resource_name, xi_fs_stat_t* resource_stat );

/**
 * @function
 * @brief Requests that the file be opened for reading or writing, and that
 * a handle to the file be returned via an out parameter.
 *
 * @param [in] resource_name the name of the file to open
 * @param [in] size the size of file in bytes, useful when opening
 * files for write.
 * @param [in] open_flags a read/write/append bitmask of operations as
 * defined by a bitmask type xi_fs_open_flags_t.
 * @param [out] resource_handle_out a pointer to an abstracted
 * xi_fs_resource_handle_t data type. This value will be passed to
 * future file operations such as read, write or close.
 *
 * @return xi_state_t XI_STATE_OK in case of a success and other in case of an
 * error.
 */
xi_state_t xi_bsp_io_fs_open( const char* const resource_name,
                              const size_t size,
                              const xi_fs_open_flags_t open_flags,
                              xi_fs_resource_handle_t* resource_handle_out );

/**
 * @function
 * @brief reads a subset of a file that was previously opened
 * with a call to xi_bsp_io_fs_open.
 *
 * @param [in] resource_handle the handle created by a previous call
 * to xi_bsp_io_fs_open.
 * @param [in] offset the position of the resource, in bytes, from which
 * to start read operations.
 * @param [in/out] buffer a pointer to a byte array. Data read from the
 * resource should be stored in this array starting at offset zero.
 * @param [in] buffer_size the length of the buffer in bytes.
 *
 * @return xi_state_t XI_STATE_OK in case of a success and other in case of an
 * error.
 */
xi_state_t xi_bsp_io_fs_read( const xi_fs_resource_handle_t resource_handle,
                              const size_t offset,
                              const uint8_t** buffer,
                              size_t* const buffer_size );

/**
 * @function
 * @brief writes to a portion of a file that was previously opened
 * with a call to xi_bsp_io_fs_open.
 *
 * @param [in] resource_handle created by a previous call
 * to xi_bsp_io_fs_open.
 * @param [in] buffer a pointer to a byte array. Data in this buffer
 * should be written to the corresponding resource. This buffer
 * should not be freed by this function.
 * @param [in] buffer_size the length of the buffer in bytes.
 * @param [in] offset the position of the resource, in bytes, to start
 * to the write operation.
 * @param [out] bytes_written store the number of bytes that were
 * successfully written to the resource.
 *
 * @return xi_state_t XI_STATE_OK in case of a success and other in case of an
 * error.
 */
xi_state_t xi_bsp_io_fs_write( const xi_fs_resource_handle_t resource_handle,
                               const uint8_t* const buffer,
                               const size_t buffer_size,
                               const size_t offset,
                               size_t* const bytes_written );

/**
 * @function
 * @brief signals that the corresponding resource read/write
 * operations are complete and that the resource should be closed.
 *
 * @param [in] resource_handle created by a previous call
 * to xi_bsp_io_fs_open.  This is the handle to the resource
 * that should be closed.
 *
 * @return xi_state_t XI_STATE_OK in case of a success and other in case of an
 * error.
 */
xi_state_t xi_bsp_io_fs_close( const xi_fs_resource_handle_t resource_handle );

/**
 * @function
 * @brief request to remove the file/resources with the corresponding
 * resource name.
 *
 * @param [in] resource_name the name of the file to remove
 *
 * @return xi_state_t XI_STATE_OK in case of a success and other in case of an
 * error.
 */
xi_state_t xi_bsp_io_fs_remove( const char* const resource_name );

#ifdef __cplusplus
}
#endif

#endif /* __XI_BSP_IO_FS_H__ */
