/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_allocator.h"
#include "xi_bsp_mem.h"

extern void* memset( void* ptr, int value, size_t num );

void* __xi_alloc( size_t byte_count )
{
    return xi_bsp_mem_alloc( byte_count );
}

void* __xi_calloc( size_t num, size_t byte_count )
{
    const size_t size_to_allocate = num * byte_count;
    void* ret                     = xi_bsp_mem_alloc( size_to_allocate );

    /* it's unspecified if memset works with NULL pointer */
    if ( NULL != ret )
    {
        memset( ret, 0, size_to_allocate );
    }

    return ret;
}

void* __xi_realloc( void* ptr, size_t byte_count )
{
    return xi_bsp_mem_realloc( ptr, byte_count );
}

void __xi_free( void* ptr )
{
    xi_bsp_mem_free( ptr );
}
