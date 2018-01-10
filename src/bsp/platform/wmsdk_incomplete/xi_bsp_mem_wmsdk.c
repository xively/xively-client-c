/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_bsp_mem.h"
#include <wm_os.h>

void* xi_bsp_mem_alloc( size_t byte_count )
{
    return ( void* )os_mem_alloc( byte_count );
}

void* xi_bsp_mem_realloc( void* ptr, size_t byte_count )
{
    return ( void* )os_mem_realloc( ptr, byte_count );
}

void xi_bsp_mem_free( void* ptr )
{
    os_mem_free( ptr );
}
