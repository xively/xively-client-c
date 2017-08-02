/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_io_fs.h>
#include <xi_bsp_fwu.h>

xi_state_t xi_bsp_io_fs_open( const char* const resource_name,
                              const size_t size,
                              const xi_fs_open_flags_t open_flags,
                              xi_fs_resource_handle_t* resource_handle_out )
{
    ( void )size;
    ( void )open_flags;
    ( void )resource_handle_out;

    if ( 1 == xi_bsp_fwu_is_this_firmware( resource_name ) )
    {
        /* the resource is firmware */
    }
    else
    {
        /* it's an ordinary file */
    }

    return XI_NOT_IMPLEMENTED;
}

xi_state_t xi_bsp_io_fs_read( const xi_fs_resource_handle_t resource_handle,
                              const size_t offset,
                              const uint8_t** buffer,
                              size_t* const buffer_size )
{
    ( void )resource_handle;
    ( void )offset;
    ( void )buffer;
    ( void )buffer_size;

    return XI_NOT_IMPLEMENTED;
}

xi_state_t xi_bsp_io_fs_write( const xi_fs_resource_handle_t resource_handle,
                               const uint8_t* const buffer,
                               const size_t buffer_size,
                               const size_t offset,
                               size_t* const bytes_written )
{
    ( void )resource_handle;
    ( void )buffer;
    ( void )buffer_size;
    ( void )offset;
    ( void )bytes_written;

    return XI_NOT_IMPLEMENTED;
}

xi_state_t xi_bsp_io_fs_close( const xi_fs_resource_handle_t resource_handle )
{
    ( void )resource_handle;

    return XI_NOT_IMPLEMENTED;
}

xi_state_t xi_bsp_io_fs_remove( const char* const resource_name )
{
    ( void )resource_name;

    return XI_NOT_IMPLEMENTED;
}
