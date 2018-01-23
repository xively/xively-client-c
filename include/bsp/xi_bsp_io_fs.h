/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_BSP_IO_FS_H__
#define __XI_BSP_IO_FS_H__

/**
 * @file xi_bsp_io.fs.h
 * @brief Xively Client's Board Support Package (BSP) for File Access
 *
 * This file defines the File Management API used by the Xively C Client.
 *
 * The Xively C Client uses this BSP to facilitate non-volatile data
 * storage of certificates and files received via Xively Secure File Transfer.
 *
 * NOTE: the use of this BSP to store certificates used during the TLS
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

typedef intptr_t xi_bsp_io_fs_resource_handle_t;

#define XI_BSP_IO_FS_INVALID_RESOURCE_HANDLE -1
#define xi_bsp_io_fs_init_resource_handle() XI_BSP_IO_FS_INVALID_RESOURCE_HANDLE

/**
 * @typedef xi_bsp_io_fs_state_e
 * @brief Return value of the BSP NET API functions.
 *
 * The implementation reports internal status to Xively Client through these values.
 */
typedef enum xi_bsp_io_fs_state_e {
    /** operation finished successfully */
    XI_BSP_IO_FS_STATE_OK = 0,
    /** operation failed on generic error */
    XI_BSP_IO_FS_ERROR = 1,
    /** invalid parameter passed to function */
    XI_BSP_IO_FS_INVALID_PARAMETER = 2,
    /** resource/file is not available */
    XI_BSP_IO_FS_RESOURCE_NOT_AVAILABLE = 3,
    /** out of memory error */
    XI_BSP_IO_FS_OUT_OF_MEMORY = 4,
    /** function not implemented on target platform */
    XI_BSP_IO_FS_NOT_IMPLEMENTED = 5,
    /** error opening file resource **/
    XI_BSP_IO_FS_OPEN_ERROR = 6,
    /** error that file open could only open read only */
    XI_BSP_IO_FS_OPEN_READ_ONLY = 7,
    /** error reported when file cannot be removed */
    XI_BSP_IO_FS_REMOVE_ERROR = 8,
    /** error when attempting to write file data */
    XI_BSP_IO_FS_WRITE_ERROR = 9,
    /** error reported when file cannot be read */
    XI_BSP_IO_FS_READ_ERROR = 10,
    /** error reported when file cannot be closed */
    XI_BSP_IO_FS_CLOSE_ERROR = 11,
} xi_bsp_io_fs_state_t;

/**
 * @enum xi_bsp_io_fs_resource_type_t
 * @brief describes types of resources that are availible through this API.
 * These types were created in order to differenciate types based on their
 * security class.
 */
typedef enum xi_bsp_io_fs_resource_type_e {
    XI_BSP_IO_FS_CERTIFICATE = 0, /**< 0 **/
    XI_BSP_IO_FS_CREDENTIALS,     /**< 1 **/
    XI_BSP_IO_FS_CONFIG_DATA      /**< 2 **/
} xi_bsp_io_fs_resource_type_t;

/*
 * @name xi_bsp_io_fs_stat_s
 * @brief Information that the Xively Client needs returned
 * when xi_bsp_io_fs_stat() is called.
 */
typedef struct xi_bsp_io_fs_stat_s
{
    size_t resource_size;
} xi_bsp_io_fs_stat_t;

/**
 * @enum xi_bsp_io_fs_open_flags_t
 * @brief Abstracted values that represent the various types of file operations
 * that should be passed to the underlying system when opening a file resource
 * via xi_bsp_io_fs_open().

 * As a bitmask there could be more than one of these flags set on a given
 * open request.
 */
typedef enum xi_bsp_io_fs_open_flags {
    XI_BSP_IO_FS_OPEN_READ   = 1 << 0,
    XI_BSP_IO_FS_OPEN_WRITE  = 1 << 1,
    XI_BSP_IO_FS_OPEN_APPEND = 1 << 2,
} xi_bsp_io_fs_open_flags_t;

/**
 * @function
 * @brief Used by the Xively C Client to determine the existance and size of a
 * file.
 *
 * @param [in] resource_name the name of the file to check
 * @param [out] resource_stat a structure to be populated based on the file
 * size data.
 *
 * @return xi_bsp_io_fs_state_t XI_STATE_OK in case of a success and other in case of an
 * error.
 */
xi_bsp_io_fs_state_t
xi_bsp_io_fs_stat( const char* const resource_name, xi_bsp_io_fs_stat_t* resource_stat );

/**
 * @function
 * @brief Requests that the file be opened for reading or writing, and that
 * a handle to the file be returned via an out parameter.
 *
 * @param [in] resource_name the name of the file to open
 * @param [in] size the size of file in bytes. Necessary on some
 * flash file system implementations which reserve space when
 * the file is opened for writing.  Not used in POSIX implementations.
 * @param [in] open_flags a read/write/append bitmask of operations as
 * defined by a bitmask type xi_bsp_io_fs_open_flags_t.
 * @param [out] resource_handle_out a pointer to an abstracted
 * xi_bsp_io_fs_resource_handle_t data type. This value will be passed to
 * future file operations such as read, write or close.
 *
 * @return xi_bsp_io_fs_state_t XI_STATE_OK in case of a success and other in case of an
 * error.
 */
xi_bsp_io_fs_state_t
xi_bsp_io_fs_open( const char* const resource_name,
                   const size_t size,
                   const xi_bsp_io_fs_open_flags_t open_flags,
                   xi_bsp_io_fs_resource_handle_t* resource_handle_out );

/**
 * @function
 * @brief reads a subset of a file that was previously opened
 * with a call to xi_bsp_io_fs_open.
 *
 * @param [in] resource_handle the handle created by a previous call
 * to xi_bsp_io_fs_open.
 * @param [in] offset the position of the resource, in bytes, from which
 * to start read operations.
 * @param [out] buffer an outgoing pointer to a buffer which contains the bytes
 * read from the file. The BSP is responsible for all memory management of the
 * buffer itself. Please fill this buffer at offset zero each time this
 * function is called. You may reuse the buffer for each invocation.  If
 * you've allocated a buffer for each read operation then you may free the
 * previous buffer on a subsequent xi_bsp_io_fs_read call, or when the file
 * is closed.
 * @param [out] buffer_size the number of bytes read from file and stored
 * in the buffer.
 *
 * @return xi_bsp_io_fs_state_t XI_STATE_OK in case of a success and other in case of an
 * error.
 */
xi_bsp_io_fs_state_t
xi_bsp_io_fs_read( const xi_bsp_io_fs_resource_handle_t resource_handle,
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
 * @return xi_bsp_io_fs_state_t XI_STATE_OK in case of a success and other in case of an
 * error.
 */
xi_bsp_io_fs_state_t
xi_bsp_io_fs_write( const xi_bsp_io_fs_resource_handle_t resource_handle,
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
 * @return xi_bsp_io_fs_state_t XI_STATE_OK in case of a success and other in case of an
 * error.
 */
xi_bsp_io_fs_state_t
xi_bsp_io_fs_close( const xi_bsp_io_fs_resource_handle_t resource_handle );

/**
 * @function
 * @brief request to remove the file/resources with the corresponding
 * resource name.
 *
 * @param [in] resource_name the name of the file to remove
 *
 * @return xi_bsp_io_fs_state_t XI_STATE_OK in case of a success and other in case of an
 * error.
 */
xi_bsp_io_fs_state_t xi_bsp_io_fs_remove( const char* const resource_name );

#ifdef __cplusplus
}
#endif

#endif /* __XI_BSP_IO_FS_H__ */
