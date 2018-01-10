/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_fs_header.h"
#include "xi_fs_filename_defs.h"
#include "xi_macros.h"
#include "xi_debug.h"

#ifndef XI_NO_TLS_LAYER
#include "xi_RootCA_list.h"
#endif

/* The size of the buffer to be used for reads. */
#ifndef XI_NO_TLS_LAYER
const size_t xi_fs_buffer_size = XI_ROOTCA_LIST_BYTE_LENGTH;
#else
const size_t xi_fs_buffer_size = 512;
#endif /* XI_NO_TLS_LAYER */

/* local stat handler function type defined per resource id */
typedef xi_state_t( xi_fs_memory_stat_t )( const xi_fs_resource_handle_t resource_id,
                                           xi_fs_stat_t* resource_stat );

/* local open handler function type defined per resource id */
typedef xi_state_t( xi_fs_memory_open_t )( const xi_fs_resource_handle_t resource_id );

typedef enum xi_fs_memory_resource_state_e {
    XI_FS_MEMORY_RESOURCE_STATE_CLOSED = 0,
    XI_FS_MEMORY_RESOURCE_STATE_OPEN
} xi_fs_memory_resource_state_t;

/* declarations of all memory functions */
#ifndef XI_NO_TLS_LAYER
xi_state_t xi_fs_memory_stat_builtin_cert( const xi_fs_resource_handle_t resource_id,
                                           xi_fs_stat_t* resource_stat );
xi_state_t xi_fs_memory_open_builtin_cert( const xi_fs_resource_handle_t resource_id );
#endif

/*
 * @struct xi_fs_memory_database_s
 * @brief describes a single entry in a memory of fs database
 */
typedef struct xi_fs_memory_database_s
{
    const char* const file_name;                  /* file name of the entry */
    xi_fs_memory_stat_t* stat_handler_function;   /* associated stat handler function */
    xi_fs_memory_open_t* open_handler_function;   /* associated open handler function */
    const void* memory_ptr;                       /* resource memory pointer */
    size_t open_counter;                          /* the counter for open counts */
    xi_fs_memory_resource_state_t resource_state; /* state associated to that
                                                     resource */
} xi_fs_memory_database_t;

xi_fs_memory_database_t XI_FS_MEMORY_DATABASE[] = {
#ifndef XI_NO_TLS_LAYER
    {.file_name             = XI_GLOBAL_CERTIFICATE_FILE_NAME_DEF,
     .stat_handler_function = &xi_fs_memory_stat_builtin_cert,
     .open_handler_function = &xi_fs_memory_open_builtin_cert,
     .resource_state        = XI_FS_MEMORY_RESOURCE_STATE_CLOSED,
     .open_counter          = 0,
     .memory_ptr            = NULL}
#endif
};

#ifndef XI_NO_TLS_LAYER
xi_state_t xi_fs_memory_stat_builtin_cert( const xi_fs_resource_handle_t resource_id,
                                           xi_fs_stat_t* resource_stat )
{
    XI_UNUSED( resource_id );

    if ( NULL == resource_stat )
    {
        return XI_INVALID_PARAMETER;
    }

    /* update the resource size by taking it from the compiled in array */
    resource_stat->resource_size = sizeof( xi_RootCA_list );

    return XI_STATE_OK;
}

xi_state_t xi_fs_memory_open_builtin_cert( const xi_fs_resource_handle_t resource_id )
{
    assert( resource_id >= 0 );
    assert( resource_id < ( signed )XI_ARRAYSIZE( XI_FS_MEMORY_DATABASE ) );

    xi_fs_memory_database_t* const entry = &XI_FS_MEMORY_DATABASE[resource_id];

    entry->memory_ptr = xi_RootCA_list;

    return XI_STATE_OK;
}
#endif

inline static xi_state_t
xi_fs_memory_find_entry( const char* resource_name, xi_fs_resource_handle_t* resource_id )
{
    const size_t database_size = XI_ARRAYSIZE( XI_FS_MEMORY_DATABASE );

    // PRE-CONDITIONS
    assert( NULL != resource_name );
    assert( NULL != resource_id );

    size_t i = 0;

    *resource_id = XI_FS_INVALID_RESOURCE_HANDLE;

    for ( ; i < database_size; ++i )
    {
        if ( 0 == strcmp( resource_name, XI_FS_MEMORY_DATABASE[i].file_name ) )
        {
            /* update the return resource_id parameter value */
            *resource_id = ( xi_fs_resource_handle_t )i;

            /* bail out as early as it's possible */
            return XI_STATE_OK;
        }
    }

    return XI_FS_RESOURCE_NOT_AVAILABLE;
}

xi_state_t xi_fs_stat( const void* context,
                       const xi_fs_resource_type_t resource_type,
                       const char* const resource_name,
                       xi_fs_stat_t* resource_stat )
{
    XI_UNUSED( context );

    xi_state_t ret                      = XI_FS_RESOURCE_NOT_AVAILABLE;
    xi_fs_resource_handle_t resource_id = xi_fs_init_resource_handle();

    if ( NULL == resource_stat || NULL == resource_name )
    {
        return XI_INVALID_PARAMETER;
    }

    switch ( resource_type )
    {
        case XI_FS_CERTIFICATE:
        case XI_FS_CREDENTIALS:
        case XI_FS_CONFIG_DATA:
            ret = xi_fs_memory_find_entry( resource_name, &resource_id );
            break;
        default:
            assert( 0 );
            ret = XI_INTERNAL_ERROR;
    }

    if ( XI_STATE_OK == ret )
    {
        return ( *XI_FS_MEMORY_DATABASE[resource_id].stat_handler_function )(
            resource_id, resource_stat );
    }

    return XI_FS_RESOURCE_NOT_AVAILABLE;
}

xi_state_t xi_fs_open( const void* context,
                       const xi_fs_resource_type_t resource_type,
                       const char* const resource_name,
                       const xi_fs_open_flags_t open_flags,
                       xi_fs_resource_handle_t* resource_handle )
{
    XI_UNUSED( resource_type );
    XI_UNUSED( context );

    if ( NULL == resource_name || NULL == resource_handle )
    {
        return XI_INVALID_PARAMETER;
    }

    /* it's read only filesystem */
    if ( XI_FS_OPEN_WRITE == ( open_flags & XI_FS_OPEN_WRITE ) ||
         XI_FS_OPEN_APPEND == ( open_flags & XI_FS_OPEN_APPEND ) )
    {
        return XI_FS_ERROR;
    }

    xi_state_t res = xi_fs_memory_find_entry( resource_name, resource_handle );

    if ( XI_STATE_OK == res )
    {
        /* PRE-CONDITION */
        assert( *resource_handle >= 0 );
        assert( *resource_handle < ( signed )XI_ARRAYSIZE( XI_FS_MEMORY_DATABASE ) );

        xi_fs_memory_database_t* const entry = &XI_FS_MEMORY_DATABASE[*resource_handle];

        entry->resource_state = XI_FS_MEMORY_RESOURCE_STATE_OPEN;
        entry->open_counter += 1;

        /* PRE-CONDITION */
        assert( NULL != entry->open_handler_function );

        return entry->open_handler_function( *resource_handle );
    }

    return res;
}

xi_state_t xi_fs_read( const void* context,
                       const xi_fs_resource_handle_t resource_handle,
                       const size_t offset,
                       const uint8_t** buffer,
                       size_t* const buffer_size )
{
    XI_UNUSED( context );

    const size_t database_size = XI_ARRAYSIZE( XI_FS_MEMORY_DATABASE );

    if ( XI_FS_INVALID_RESOURCE_HANDLE == resource_handle ||
         resource_handle > ( xi_fs_resource_handle_t )database_size )
    {
        return XI_INVALID_PARAMETER;
    }

    if ( XI_FS_MEMORY_RESOURCE_STATE_OPEN !=
         XI_FS_MEMORY_DATABASE[resource_handle].resource_state )
    {
        return XI_FS_ERROR;
    }

    if ( NULL == buffer || NULL == buffer_size )
    {
        return XI_INVALID_PARAMETER;
    }

    if ( NULL != *buffer )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_fs_stat_t resource_stat;

    const xi_fs_memory_database_t* const entry = &XI_FS_MEMORY_DATABASE[resource_handle];

    // PRE-CONDITION
    assert( NULL != entry->stat_handler_function );

    xi_state_t res = entry->stat_handler_function( resource_handle, &resource_stat );

    if ( XI_STATE_OK == res )
    {
        /* calculate the real offset by picking the min of
         * ( offset value, size ) */
        intptr_t real_offset =
            ( intptr_t )( XI_MIN( offset, resource_stat.resource_size ) );

        /* update the return pointer with the proper information */
        *buffer = ( const uint8_t* )( ( intptr_t )entry->memory_ptr + real_offset );

        /* update the size of the buffer to be returned taking into account chunk size */
        *buffer_size =
            XI_MIN( resource_stat.resource_size - real_offset, xi_fs_buffer_size );
    }

    return res;
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

    /* writing is forbidden as this is read only filesystem */
    return XI_FS_ERROR;
}

xi_state_t
xi_fs_close( const void* context, const xi_fs_resource_handle_t resource_handle )
{
    XI_UNUSED( context );

    const size_t database_size = XI_ARRAYSIZE( XI_FS_MEMORY_DATABASE );

    if ( XI_FS_INVALID_RESOURCE_HANDLE == resource_handle ||
         resource_handle > ( xi_fs_resource_handle_t )database_size )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_fs_memory_database_t* const entry = &XI_FS_MEMORY_DATABASE[resource_handle];

    if ( XI_FS_MEMORY_RESOURCE_STATE_OPEN != entry->resource_state )
    {
        return XI_FS_ERROR;
    }

    entry->open_counter -= 1;

    if ( 0 == entry->open_counter )
    {
        entry->memory_ptr     = NULL;
        entry->resource_state = XI_FS_MEMORY_RESOURCE_STATE_CLOSED;
    }

    return XI_STATE_OK;
}

xi_state_t xi_fs_remove( const void* context,
                         const xi_fs_resource_type_t resource_type,
                         const char* const resource_name )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_type );
    XI_UNUSED( resource_name );

    xi_state_t ret                      = XI_FS_ERROR;
    xi_fs_resource_handle_t resource_id = xi_fs_init_resource_handle();

    switch ( resource_type )
    {
        case XI_FS_CERTIFICATE:
        case XI_FS_CREDENTIALS:
        case XI_FS_CONFIG_DATA:
            ret = xi_fs_memory_find_entry( resource_name, &resource_id );
    }

    return ( ret == XI_STATE_OK ) ? XI_FS_ERROR : XI_FS_RESOURCE_NOT_AVAILABLE;
}
