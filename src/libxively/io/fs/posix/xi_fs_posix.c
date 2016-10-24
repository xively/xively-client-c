/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
  * This is part of Xively C library. */

#include "xi_fs_header.h"
#include "xi_macros.h"
#include "xi_debug.h"
#include "xi_list.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

/* The size of the buffer to be used for reads. */
const size_t xi_fs_buffer_size = 1024;

/**
 * @brief xi_fs_posix_file_handle_map_s
 *
 * Local database of file handles and allocated memory chunks.
 */
typedef struct xi_fs_posix_file_handle_container_s
{
    FILE* posix_fp;
    uint8_t* memory_buffer;
    struct xi_fs_posix_file_handle_container_s* __next;
} xi_fs_posix_file_handle_container_t;

/* local list of open files */
static xi_fs_posix_file_handle_container_t* xi_fs_posix_files_container;

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
xi_fs_posix_file_list_cnd( xi_fs_posix_file_handle_container_t* list, FILE* fp )
{
    assert( NULL != list );
    assert( NULL != fp );

    return ( list->posix_fp == fp ) ? 1 : 0;
}

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

    xi_state_t ret = XI_STATE_OK;

    struct stat stat_struct;
    memset( &stat_struct, 0, sizeof( stat_struct ) );

    int res = stat( resource_name, &stat_struct );

    /* verification of the os function result */
    XI_CHECK_CND( 0 != res, xi_fs_posix_errno_2_xi_state( errno ), ret );

    /* here we translate stat posix os stat structure to xi internal version */
    ret = xi_fs_posix_stat_2_xi_stat( &stat_struct, resource_stat );

    return ret;

err_handling:
    return ret;
}

xi_state_t xi_fs_open( const void* context,
                       const xi_fs_resource_type_t resource_type,
                       const char* const resource_name,
                       const xi_fs_open_flags_t open_flags,
                       xi_fs_resource_handle_t* resource_handle )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_type );
    XI_UNUSED( open_flags );

    if ( NULL == resource_name || NULL == resource_handle )
    {
        return XI_INVALID_PARAMETER;
    }

    /* we only allow XI_FS_OPEN_READ */
    if ( XI_FS_OPEN_APPEND == ( open_flags & XI_FS_OPEN_APPEND ) ||
         XI_FS_OPEN_WRITE == ( open_flags & XI_FS_OPEN_WRITE ) )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t ret                                 = XI_STATE_OK;
    xi_fs_posix_file_handle_container_t* new_entry = NULL;

    /* at this stage we will use "rb" only */
    FILE* fp = fopen( resource_name, "rb" );

    /* if error on fopen check the errno value */
    XI_CHECK_CND( NULL == fp, xi_fs_posix_errno_2_xi_state( errno ), ret );

    /* allocate memory for the files database element */
    XI_ALLOC_AT( xi_fs_posix_file_handle_container_t, new_entry, ret );

    /* store the posix file pointer */
    new_entry->posix_fp = fp;

    /* add the entry to the database */
    XI_LIST_PUSH_BACK( xi_fs_posix_file_handle_container_t, xi_fs_posix_files_container,
                       new_entry );

    /* make sure that size does not matter */
    assert( sizeof( fp ) == sizeof( xi_fs_resource_handle_t ) );

    /* return fp as a resource handle */
    *resource_handle = ( xi_fs_resource_handle_t )fp;

    return ret;

err_handling:
    if ( NULL != fp )
    {
        fclose( fp );
    }
    XI_SAFE_FREE( new_entry );
    *resource_handle = xi_fs_init_resource_handle();
    return ret;
}

xi_state_t xi_fs_read( const void* context,
                       const xi_fs_resource_handle_t resource_handle,
                       const size_t offset,
                       const uint8_t** buffer,
                       size_t* const buffer_size )
{
    XI_UNUSED( context );

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

    XI_CHECK_CND( NULL == elem, XI_FS_RESOURCE_NOT_AVAILABLE, ret );

    /* make an allocation for memory block */
    if ( NULL == elem->memory_buffer )
    {
        XI_ALLOC_BUFFER_AT( uint8_t, elem->memory_buffer, xi_fs_buffer_size, ret );
    }

    /* let's set an offset */
    fop_ret = fseek( fp, offset, SEEK_SET );

    /* if error on fseek check errno */
    XI_CHECK_CND( fop_ret != 0, xi_fs_posix_errno_2_xi_state( errno ), ret );

    /* use the fread to read the file chunk */
    fop_ret = fread( elem->memory_buffer, ( size_t )1, xi_fs_buffer_size, fp );

    /* if error on fread check errno */
    XI_CHECK_CND( fop_ret == 0, xi_fs_posix_errno_2_xi_state( errno ), ret );

    /* return buffer, buffer_size */
    *buffer      = elem->memory_buffer;
    *buffer_size = fop_ret;

    return XI_STATE_OK;

err_handling:
    *buffer      = NULL;
    *buffer_size = 0;
    XI_SAFE_FREE( elem->memory_buffer );

    return ret;
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
    XI_CHECK_CND( NULL == elem, XI_FS_RESOURCE_NOT_AVAILABLE, ret );

    /* remove element from the list */
    XI_LIST_DROP( xi_fs_posix_file_handle_container_t, xi_fs_posix_files_container,
                  elem );

    fop_ret = fclose( fp );

    /* if error on fclose check errno */
    XI_CHECK_CND( 0 != fop_ret, xi_fs_posix_errno_2_xi_state( errno ), ret );

    XI_SAFE_FREE( elem->memory_buffer );
    XI_SAFE_FREE( elem );

    return XI_STATE_OK;

err_handling:
    if ( NULL != elem )
    {
        XI_SAFE_FREE( elem->memory_buffer );
        XI_SAFE_FREE( elem );
    }
    return ret;
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
