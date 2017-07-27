/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_io_fs.h>
#include <xi_bsp_mem.h>

#include <xi_list.h>

#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <assert.h>
#include <memory.h>

/* The size of the buffer to be used for reads. */
const size_t xi_fs_buffer_size = 1024;

#define XI_BSP_IO_FS_CHECK_CND( cnd, e, s )                                              \
    if ( ( cnd ) )                                                                       \
    {                                                                                    \
        ( s ) = ( e );                                                                   \
        goto err_handling;                                                               \
    }

/**
 * @brief xi_fs_posix_file_handle_container_t
 *
 * Database type of file handles and allocated memory chunks.
 */
typedef struct xi_fs_posix_file_handle_container_s
{
    FILE* posix_fp;
    uint8_t* memory_buffer;
    struct xi_fs_posix_file_handle_container_s* __next;
} xi_fs_posix_file_handle_container_t;

/* local list of open files */
static xi_fs_posix_file_handle_container_t* xi_fs_posix_files_container;

/* helper function that translates the errno errors to the xi_state_t values */
static xi_state_t xi_fs_posix_errno_2_xi_state( int errno_value )
{
    xi_state_t ret = XI_STATE_OK;

    switch ( errno_value )
    {
        case EBADF:
        case EACCES:
        case EFAULT:
        case ENOENT:
        case ENOTDIR:
            ret = XI_FS_RESOURCE_NOT_AVAILABLE;
            break;
        case ELOOP:
        case EOVERFLOW:
            ret = XI_FS_ERROR;
            break;
        case ENAMETOOLONG:
            ret = XI_INVALID_PARAMETER;
            break;
        case ENOMEM:
            ret = XI_OUT_OF_MEMORY;
            break;
        default:
            ret = XI_FS_ERROR;
    }

    return ret;
}

/* helper function that translates posix stat to xi stat */
static xi_state_t xi_fs_posix_stat_2_xi_stat( const struct stat* const posix_stat,
                                              xi_fs_stat_t* const xi_stat )
{
    assert( NULL != posix_stat );
    assert( NULL != xi_stat );

    xi_stat->resource_size = posix_stat->st_size;

    return XI_STATE_OK;
}

/**
 * @brief xi_fs_posix_file_list_cnd
 *
 * Functor for list find function.
 *
 * @param list
 * @param fp
 * @return 1 if list element is the one with the matching fp 0 otherwise
 */
static uint8_t
xi_fs_posix_file_list_cnd( xi_fs_posix_file_handle_container_t* list_element, FILE* fp )
{
    assert( NULL != list_element );
    assert( NULL != fp );

    return ( list_element->posix_fp == fp ) ? 1 : 0;
}

xi_state_t
xi_bsp_io_fs_stat( const char* const resource_name, xi_fs_stat_t* resource_stat )
{
    if ( NULL == resource_stat || NULL == resource_name )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t ret = XI_STATE_OK;

    struct stat stat_struct;
    memset( &stat_struct, 0, sizeof( stat_struct ) );

    int res = stat( resource_name, &stat_struct );

    /* verification of the os function result */
    XI_BSP_IO_FS_CHECK_CND( 0 != res, xi_fs_posix_errno_2_xi_state( errno ), ret );

    /* here we translate stat posix os stat structure to libxively version */
    ret = xi_fs_posix_stat_2_xi_stat( &stat_struct, resource_stat );

err_handling:
    return ret;
}

xi_state_t xi_bsp_io_fs_open( const char* const resource_name,
                              const uint32_t size,
                              const xi_fs_open_flags_t open_flags,
                              xi_fs_resource_handle_t* resource_handle_out )
{
    ( void )size;

    if ( NULL == resource_name || NULL == resource_handle_out )
    {
        return XI_INVALID_PARAMETER;
    }

    /* append not supported */
    if ( XI_FS_OPEN_APPEND == ( open_flags & XI_FS_OPEN_APPEND ) )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_fs_posix_file_handle_container_t* new_entry = NULL;
    xi_state_t ret                                 = XI_STATE_OK;

    FILE* fp = fopen( resource_name, ( open_flags & XI_FS_OPEN_READ ) ? "rb" : "wb" );

    /* if error on fopen check the errno value */
    XI_BSP_IO_FS_CHECK_CND( NULL == fp, xi_fs_posix_errno_2_xi_state( errno ), ret );

    /* allocate memory for the files database element */
    new_entry = xi_bsp_mem_alloc( sizeof( xi_fs_posix_file_handle_container_t ) );
    memset( new_entry, 0, sizeof( xi_fs_posix_file_handle_container_t ) );

    /* store the posix file pointer */
    new_entry->posix_fp = fp;

    /* add the entry to the database */
    XI_LIST_PUSH_BACK( xi_fs_posix_file_handle_container_t, xi_fs_posix_files_container,
                       new_entry );

    /* make sure that size does not matter */
    assert( sizeof( fp ) == sizeof( xi_fs_resource_handle_t ) );

    /* return fp as a resource handle */
    *resource_handle_out = ( xi_fs_resource_handle_t )fp;

    return ret;

err_handling:
    if ( NULL != fp )
    {
        fclose( fp );
    }
    xi_bsp_mem_free( new_entry );
    *resource_handle_out = xi_fs_init_resource_handle();
    return ret;
}

xi_state_t xi_bsp_io_fs_read( const xi_fs_resource_handle_t resource_handle,
                              const size_t offset,
                              const uint8_t** buffer,
                              size_t* const buffer_size )
{
    if ( NULL == buffer || NULL != *buffer || NULL == buffer_size ||
         XI_FS_INVALID_RESOURCE_HANDLE == resource_handle )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t ret = XI_STATE_OK;
    FILE* fp       = ( FILE* )resource_handle;
    int fop_ret    = 0;

    xi_fs_posix_file_handle_container_t* elem = NULL;
    XI_LIST_FIND( xi_fs_posix_file_handle_container_t, xi_fs_posix_files_container,
                  xi_fs_posix_file_list_cnd, fp, elem );

    XI_BSP_IO_FS_CHECK_CND( NULL == elem, XI_FS_RESOURCE_NOT_AVAILABLE, ret );

    /* make an allocation for memory block */
    if ( NULL == elem->memory_buffer )
    {
        elem->memory_buffer = ( uint8_t* )xi_bsp_mem_alloc( xi_fs_buffer_size );
        memset( elem->memory_buffer, 0, xi_fs_buffer_size );
    }

    /* let's set an offset */
    fop_ret = fseek( fp, offset, SEEK_SET );

    /* if error on fseek check errno */
    XI_BSP_IO_FS_CHECK_CND( fop_ret != 0, xi_fs_posix_errno_2_xi_state( errno ), ret );

    /* use the fread to read the file chunk */
    fop_ret = fread( elem->memory_buffer, ( size_t )1, xi_fs_buffer_size, fp );

    /* if error on fread check errno */
    XI_BSP_IO_FS_CHECK_CND( fop_ret == 0, xi_fs_posix_errno_2_xi_state( errno ), ret );

    /* return buffer, buffer_size */
    *buffer      = elem->memory_buffer;
    *buffer_size = fop_ret;

    return XI_STATE_OK;

err_handling:

    *buffer      = NULL;
    *buffer_size = 0;
    xi_bsp_mem_free( elem->memory_buffer );

    return ret;
}

xi_state_t xi_bsp_io_fs_write( const xi_fs_resource_handle_t resource_handle,
                               const uint8_t* const buffer,
                               const size_t buffer_size,
                               const size_t offset,
                               size_t* const bytes_written )
{
    if ( NULL == buffer || 0 == buffer_size ||
         XI_FS_INVALID_RESOURCE_HANDLE == resource_handle )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t ret = XI_STATE_OK;
    FILE* fp       = ( FILE* )resource_handle;

    xi_fs_posix_file_handle_container_t* elem = NULL;
    XI_LIST_FIND( xi_fs_posix_file_handle_container_t, xi_fs_posix_files_container,
                  xi_fs_posix_file_list_cnd, fp, elem );

    XI_BSP_IO_FS_CHECK_CND( NULL == elem, XI_FS_RESOURCE_NOT_AVAILABLE, ret );

    /* let's set the offset */
    const int fop_ret = fseek( fp, offset, SEEK_SET );

    /* if error on fseek check errno */
    XI_BSP_IO_FS_CHECK_CND( fop_ret != 0, xi_fs_posix_errno_2_xi_state( errno ), ret );

    *bytes_written = fwrite( buffer, ( size_t )1, buffer_size, fp );

    /* if error on fwrite check errno */
    XI_BSP_IO_FS_CHECK_CND( buffer_size != *bytes_written,
                            xi_fs_posix_errno_2_xi_state( ferror( fp ) ), ret );

err_handling:

    return ret;
}

xi_state_t xi_bsp_io_fs_close( const xi_fs_resource_handle_t resource_handle )
{
    if ( XI_FS_INVALID_RESOURCE_HANDLE == resource_handle )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t ret                            = XI_STATE_OK;
    FILE* fp                                  = ( FILE* )resource_handle;
    xi_fs_posix_file_handle_container_t* elem = NULL;
    int fop_ret                               = 0;

    XI_LIST_FIND( xi_fs_posix_file_handle_container_t, xi_fs_posix_files_container,
                  xi_fs_posix_file_list_cnd, fp, elem );

    /* if element not on the list return resource not available error */
    XI_BSP_IO_FS_CHECK_CND( NULL == elem, XI_FS_RESOURCE_NOT_AVAILABLE, ret );

    /* remove element from the list */
    XI_LIST_DROP( xi_fs_posix_file_handle_container_t, xi_fs_posix_files_container,
                  elem );

    fop_ret = fclose( fp );

    /* if error on fclose check errno */
    XI_BSP_IO_FS_CHECK_CND( 0 != fop_ret, xi_fs_posix_errno_2_xi_state( errno ), ret );

    xi_bsp_mem_free( elem->memory_buffer );
    xi_bsp_mem_free( elem );

    return XI_STATE_OK;

err_handling:

    if ( NULL != elem )
    {
        xi_bsp_mem_free( elem->memory_buffer );
        xi_bsp_mem_free( elem );
    }
    return ret;
}

xi_state_t xi_bsp_io_fs_remove( const char* const resource_name )
{
    int ret = remove( resource_name );

    XI_BSP_IO_FS_CHECK_CND( 0 != ret, xi_fs_posix_errno_2_xi_state( errno ), ret );

err_handling:

    return ret;
}
