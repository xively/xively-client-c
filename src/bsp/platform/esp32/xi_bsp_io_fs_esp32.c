/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <sys/stat.h>
#include <errno.h>
#include <assert.h>

#if 1
#include <xi_bsp_debug.h>
#else
#define xi_bsp_debug_format printf // TODO: delete me
#define xi_bsp_debug_logger printf // TODO: delete me
#endif
#include <xi_list.h>
#include <xi_bsp_io_fs.h>
#include <xi_bsp_fwu.h>
#include <xi_bsp_mem.h>
#include <string.h>
#include <stdio.h>

#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_ota_ops.h"

#define XI_ESP32_MAX_FILENAME_LEN 64
#define ESP32_FS_BASE_PATH "/spiflash"

/* These functions are implemented in the application layer to report status */
extern void esp32_xibsp_notify_update_started( const char* filename, size_t file_size );
extern void esp32_xibsp_notify_chunk_written( size_t chunk_size, size_t offset );

#define XI_BSP_IO_FS_CHECK_CND( cnd, e, s ) \
    if ( ( cnd ) )                          \
    {                                       \
        ( s ) = ( e );                      \
        perror( "errno" );                  \
        goto err_handling;                  \
    }

/**
 * @brief xi_bsp_io_fs_posix_file_handle_container_t
 *
 * Database type of file handles and allocated memory chunks.
 */
typedef struct xi_bsp_io_fs_posix_file_handle_container_s
{
    FILE* posix_fp;
    char* file_path;
    uint8_t* memory_buffer;
    struct xi_bsp_io_fs_posix_file_handle_container_s* __next;
} xi_bsp_io_fs_posix_file_handle_container_t;

/* local list of open files */
static xi_bsp_io_fs_posix_file_handle_container_t* 
    xi_bsp_io_fs_posix_files_container;

/* The size of the buffer to be used for reads. */
const size_t xi_bsp_io_fs_buffer_size = 1024;
/* ESP32 Over The Air FW updates API handle */
static esp_ota_handle_t open_firmware_bin_handle = 0;

/* translates bsp errno errors to the xi_bsp_io_fs_state_t values */
xi_bsp_io_fs_state_t xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( int errno_value )
{
    xi_bsp_io_fs_state_t ret = XI_BSP_IO_FS_STATE_OK;

    switch ( errno_value )
    {
        case EBADF:
        case EACCES:
        case EFAULT:
        case ENOENT:
        case ENOTDIR:
            ret = XI_BSP_IO_FS_RESOURCE_NOT_AVAILABLE;
            break;
        case ELOOP:
        case EOVERFLOW:
            ret = XI_BSP_IO_FS_ERROR;
            break;
        case EINVAL:
            ret = XI_BSP_IO_FS_INVALID_PARAMETER;
            break;
        case ENAMETOOLONG:
            ret = XI_BSP_IO_FS_INVALID_PARAMETER;
            break;
        case ENOMEM:
            ret = XI_BSP_IO_FS_OUT_OF_MEMORY;
            break;
        default:
            ret = XI_BSP_IO_FS_ERROR;
    }

    return ret;
}

/* helper function that translates posix stat to xi stat */
static xi_bsp_io_fs_state_t xi_bsp_io_fs_posix_stat_2_xi_bsp_io_fs_stat( const struct stat* const posix_stat,
                                                                         xi_bsp_io_fs_stat_t* const xi_stat )
{
    assert( NULL != posix_stat );
    assert( NULL != xi_stat );

    xi_stat->resource_size = posix_stat->st_size;

    return XI_BSP_IO_FS_STATE_OK;
}

/**
 * @brief xi_bsp_io_fs_posix_file_list_cnd
 *
 * Functor for list find function.
 *
 * @param list
 * @param fp
 * @return 1 if list element is the one with the matching fp 0 otherwise
 */
static uint8_t
xi_bsp_io_fs_posix_file_list_cnd( xi_bsp_io_fs_posix_file_handle_container_t* list_element,
                                  FILE* fp )
{
    assert( NULL != list_element );
    assert( NULL != fp );

    return ( list_element->posix_fp == fp ) ? 1 : 0;
}

xi_bsp_io_fs_state_t
xi_bsp_io_fs_stat( const char* const resource_name, xi_bsp_io_fs_stat_t* resource_stat )
{
    if ( NULL == resource_stat || NULL == resource_name )
    {
        return XI_BSP_IO_FS_INVALID_PARAMETER;
    }

    xi_bsp_io_fs_state_t ret = XI_BSP_IO_FS_STATE_OK;

    struct stat stat_struct;
    memset( &stat_struct, 0, sizeof( stat_struct ) );

    int res = stat( resource_name, &stat_struct );

    /* Verification of the os function result.
     * Jump to err_handling label in case of failure. */
    XI_BSP_IO_FS_CHECK_CND( 0 != res, xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( errno ), ret );

    /* here we translate stat posix os stat structure to libxively version */
    ret = xi_bsp_io_fs_posix_stat_2_xi_bsp_io_fs_stat( &stat_struct, resource_stat );

err_handling:
    return ret;
}

xi_bsp_io_fs_state_t
xi_bsp_io_fs_open( const char* const resource_name,
                   const size_t size,
                   const xi_bsp_io_fs_open_flags_t open_flags,
                   xi_bsp_io_fs_resource_handle_t* resource_handle_out )
{
    xi_bsp_io_fs_posix_file_handle_container_t* new_entry = NULL;
    xi_bsp_io_fs_state_t ret = XI_BSP_IO_FS_STATE_OK;
    esp_err_t esp_retv       = ESP_OK;
    char* filepath           = NULL;
    FILE* fp                 = NULL;

    xi_bsp_debug_format( "Opening file [%s] requested\n", resource_name );
    if ( NULL == resource_name || NULL == resource_handle_out )
    {
        return XI_BSP_IO_FS_INVALID_PARAMETER;
    }

    if ( xi_bsp_fwu_is_this_firmware( resource_name ) )
    {
        const esp_partition_t* next_partition = esp_ota_get_next_update_partition( NULL );

        if ( NULL == next_partition )
        {
            xi_bsp_debug_logger( "Failed to get next OTA update partition" );
            return XI_BSP_IO_FS_OPEN_ERROR;
        }

        esp_retv = esp_ota_begin( next_partition, size,
                                  ( esp_ota_handle_t* )resource_handle_out );

        if ( ESP_OK != esp_retv )
        {
            xi_bsp_debug_format( "esp_ota_begin() failed with error %d", esp_retv );
            return XI_BSP_IO_FS_OPEN_ERROR;
        }

        esp32_xibsp_notify_update_started( resource_name, size );
        open_firmware_bin_handle = *resource_handle_out;
    }
    else
    {
        /* append not supported */
        if ( XI_BSP_IO_FS_OPEN_APPEND == ( open_flags & XI_BSP_IO_FS_OPEN_APPEND ) )
        {
            return XI_BSP_IO_FS_INVALID_PARAMETER;
        }

        /* Create path to the mounted partition and fopen the file */
        if ( XI_ESP32_MAX_FILENAME_LEN <
             ( strlen( resource_name ) + strlen( ESP32_FS_BASE_PATH ) ) )
        {
            return XI_BSP_IO_FS_INVALID_PARAMETER;
        }

        filepath = xi_bsp_mem_alloc( XI_ESP32_MAX_FILENAME_LEN );
        if ( NULL == filepath )
        {
            return XI_BSP_IO_FS_OPEN_ERROR;
        }
        snprintf( filepath, XI_ESP32_MAX_FILENAME_LEN, "%s/%s", ESP32_FS_BASE_PATH,
                  resource_name );
        fp = fopen( ( const char* )filepath,
                    ( open_flags & XI_BSP_IO_FS_OPEN_READ ) ? "rb" : "wb" );

        /* if error on fopen check the errno value */
        XI_BSP_IO_FS_CHECK_CND(
            NULL == fp, xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( errno ), ret );

        /* allocate memory for the files database element */
        new_entry =
            xi_bsp_mem_alloc( sizeof( xi_bsp_io_fs_posix_file_handle_container_t ) );
        memset( new_entry, 0, sizeof( xi_bsp_io_fs_posix_file_handle_container_t ) );
        new_entry->file_path = filepath;

        /* store the posix file pointer */
        new_entry->posix_fp = fp;

        /* add the entry to the database */
        XI_LIST_PUSH_BACK( xi_bsp_io_fs_posix_file_handle_container_t,
                           xi_bsp_io_fs_posix_files_container, new_entry );

        /* make sure that the size is as expected. */
        assert( sizeof( fp ) == sizeof( xi_bsp_io_fs_resource_handle_t ) );

        /* return fp as a resource handle */
        *resource_handle_out = ( xi_bsp_io_fs_resource_handle_t )fp;
    }

    return XI_BSP_IO_FS_STATE_OK;

err_handling:
    if ( NULL != fp )
    {
        fclose( fp );
    }
    if ( NULL != filepath )
    {
        xi_bsp_mem_free( filepath );
    }
    xi_bsp_mem_free( new_entry );
    *resource_handle_out = xi_bsp_io_fs_init_resource_handle();
    return ret;
}

xi_bsp_io_fs_state_t
xi_bsp_io_fs_write( const xi_bsp_io_fs_resource_handle_t resource_handle,
                    const uint8_t* const buffer,
                    const size_t buffer_size,
                    const size_t offset,
                    size_t* const bytes_written )
{
    ( void )offset;
    xi_bsp_io_fs_state_t ret = XI_BSP_IO_FS_STATE_OK;
    esp_err_t retv = ESP_OK;
    if ( ( esp_ota_handle_t )resource_handle == open_firmware_bin_handle )
    {
        retv = esp_ota_write( resource_handle, buffer, buffer_size );
        if ( ESP_OK != retv )
        {
            xi_bsp_debug_format( "esp_ota_write() failed with error %d", retv );
            return XI_BSP_IO_FS_WRITE_ERROR;
        }

        *bytes_written = buffer_size;
        esp32_xibsp_notify_chunk_written( buffer_size, offset );
    }
    else
    {
        if ( NULL == buffer || 0 == buffer_size ||
             XI_BSP_IO_FS_INVALID_RESOURCE_HANDLE == resource_handle )
        {
            return XI_BSP_IO_FS_INVALID_PARAMETER;
        }

        FILE* fp = ( FILE* )resource_handle;

        xi_bsp_io_fs_posix_file_handle_container_t* elem = NULL;
        XI_LIST_FIND( xi_bsp_io_fs_posix_file_handle_container_t,
                      xi_bsp_io_fs_posix_files_container,
                      xi_bsp_io_fs_posix_file_list_cnd, fp, elem );

        XI_BSP_IO_FS_CHECK_CND( NULL == elem, XI_BSP_IO_FS_RESOURCE_NOT_AVAILABLE, ret );

        /* let's set the offset */
        const int fop_ret = fseek( fp, offset, SEEK_SET );

        /* if error on fseek check errno */
        XI_BSP_IO_FS_CHECK_CND(
            fop_ret != 0, xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( errno ), ret );

        *bytes_written = fwrite( buffer, ( size_t )1, buffer_size, fp );

        /* if error on fwrite check errno */
        XI_BSP_IO_FS_CHECK_CND(
            buffer_size != *bytes_written,
            xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( ferror( fp ) ), ret );
    }
    return XI_BSP_IO_FS_STATE_OK;

err_handling:
    return ret;
}

xi_bsp_io_fs_state_t
xi_bsp_io_fs_read( const xi_bsp_io_fs_resource_handle_t resource_handle,
                   const size_t offset,
                   const uint8_t** buffer,
                   size_t* const buffer_size )
{
    xi_bsp_io_fs_posix_file_handle_container_t* elem = NULL;
    xi_bsp_io_fs_state_t ret = XI_BSP_IO_FS_STATE_OK;
    FILE* fp                 = ( FILE* )resource_handle;
    int fop_ret              = 0;

    if ( XI_BSP_IO_FS_INVALID_RESOURCE_HANDLE == resource_handle )
    {
        xi_bsp_debug_logger( "\nInvalid resource handle - Filesystem read failed" );
        return XI_BSP_IO_FS_INVALID_PARAMETER;
    }

    if ( ( esp_ota_handle_t )resource_handle == open_firmware_bin_handle )
    {
        xi_bsp_debug_logger( "Error: No FS read of FW files in ESP32's OTA API" );
        return XI_BSP_IO_FS_NOT_IMPLEMENTED;
    }
    else
    {
        XI_LIST_FIND( xi_bsp_io_fs_posix_file_handle_container_t,
                      xi_bsp_io_fs_posix_files_container,
                      xi_bsp_io_fs_posix_file_list_cnd, fp, elem );

        XI_BSP_IO_FS_CHECK_CND( NULL == elem, XI_BSP_IO_FS_RESOURCE_NOT_AVAILABLE, ret );

        /* make an allocation for memory block */
        if ( NULL == elem->memory_buffer )
        {
            elem->memory_buffer =
                ( uint8_t* )xi_bsp_mem_alloc( xi_bsp_io_fs_buffer_size );
            memset( elem->memory_buffer, 0, xi_bsp_io_fs_buffer_size );
        }

        /* let's set an offset */
        fop_ret = fseek( fp, offset, SEEK_SET );

        /* if error on fseek check errno */
        XI_BSP_IO_FS_CHECK_CND(
            fop_ret != 0, xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( errno ), ret );

        /* use the fread to read the file chunk */
        fop_ret = fread( elem->memory_buffer, ( size_t )1, xi_bsp_io_fs_buffer_size, fp );

        /* if error on fread check errno */
        XI_BSP_IO_FS_CHECK_CND(
            fop_ret == 0, xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( errno ), ret );

        /* return buffer, buffer_size */
        *buffer      = elem->memory_buffer;
        *buffer_size = fop_ret;
    }
    return XI_BSP_IO_FS_STATE_OK;

err_handling:
    *buffer      = NULL;
    *buffer_size = 0;
    xi_bsp_mem_free( elem->file_path );
    xi_bsp_mem_free( elem->memory_buffer );

    return ret;
}

xi_bsp_io_fs_state_t
xi_bsp_io_fs_close( const xi_bsp_io_fs_resource_handle_t resource_handle )
{
    xi_bsp_io_fs_posix_file_handle_container_t* elem = NULL;
    xi_bsp_io_fs_state_t ret = XI_BSP_IO_FS_STATE_OK;
    FILE* fp                 = ( FILE* )resource_handle;
    int fop_ret              = 0;

    if ( ( esp_ota_handle_t )resource_handle == open_firmware_bin_handle )
    {
        const esp_partition_t* next_partition = esp_ota_get_next_update_partition( NULL );
        esp_err_t retv                        = ESP_OK;

        xi_bsp_debug_format( "Finished FW download to partition [%p]", next_partition );

        retv = esp_ota_end( resource_handle );
        if ( ESP_OK != retv )
        {
            xi_bsp_debug_format( "esp_ota_end() failed with error %d", retv );
            return XI_BSP_IO_FS_WRITE_ERROR;
        }
    }
    else
    {
        if ( XI_BSP_IO_FS_INVALID_RESOURCE_HANDLE == resource_handle )
        {
            return XI_BSP_IO_FS_INVALID_PARAMETER;
        }

        XI_LIST_FIND( xi_bsp_io_fs_posix_file_handle_container_t,
                      xi_bsp_io_fs_posix_files_container,
                      xi_bsp_io_fs_posix_file_list_cnd, fp, elem );

        /* if element not on the list return resource not available error */
        XI_BSP_IO_FS_CHECK_CND( NULL == elem, XI_BSP_IO_FS_RESOURCE_NOT_AVAILABLE, ret );

        /* remove element from the list */
        XI_LIST_DROP( xi_bsp_io_fs_posix_file_handle_container_t,
                      xi_bsp_io_fs_posix_files_container, elem );

        fop_ret = fclose( fp );

        /* if error on fclose check errno */
        XI_BSP_IO_FS_CHECK_CND(
            0 != fop_ret, xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( errno ), ret );

        xi_bsp_mem_free( elem->file_path );
        xi_bsp_mem_free( elem->memory_buffer );
        xi_bsp_mem_free( elem );

        return XI_BSP_IO_FS_STATE_OK;
    }
    return XI_BSP_IO_FS_STATE_OK;

err_handling:
    if ( NULL != elem )
    {
        xi_bsp_mem_free( elem->file_path );
        xi_bsp_mem_free( elem->memory_buffer );
        xi_bsp_mem_free( elem );
    }
    return ret;
}

xi_bsp_io_fs_state_t xi_bsp_io_fs_remove( const char* const resource_name )
{
    int ret = -1;
    if ( !xi_bsp_fwu_is_this_firmware( resource_name ) )
    {
        xi_bsp_debug_logger( "FS Remove not implemented for FW images" );
        return XI_BSP_IO_FS_NOT_IMPLEMENTED;
    }
    else
    {
        char filepath[64] = "";
        snprintf( filepath, 64, "%s/%s", ESP32_FS_BASE_PATH, resource_name );

        ret = remove( filepath );
        XI_BSP_IO_FS_CHECK_CND(
            0 != ret, xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( errno ), ret );
    }
    return XI_BSP_IO_FS_STATE_OK;

err_handling:
    return ret;
}
