/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_mem.h>
#include <stdio.h>

// Forward declarations of external funcitons from FreeRTOS portable.h
void* pvPortMalloc( size_t );
void vPortFree( void* );

void* xi_bsp_mem_alloc( size_t byte_count )
{
    void* ptr = ( void* )pvPortMalloc( byte_count );

    if ( NULL == ptr )
    {
        printf( "Failed to allocate %d\n", byte_count );
    }

    return ptr;
}

void* xi_bsp_mem_realloc( void* ptr, size_t byte_count )
{
    ( void )ptr;
    void* new_ptr = NULL;

    if ( NULL == new_ptr )
    {
        printf( "Failed to reallocate %d\n", byte_count );
    }

    return new_ptr;
}

void xi_bsp_mem_free( void* ptr )
{
    vPortFree( ptr );
}
