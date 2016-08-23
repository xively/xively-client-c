/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#include "xi_bsp_mem.h"
#include <stdlib.h>

void* xi_bsp_mem_alloc( size_t byte_count )
{
    return ( void* )malloc( byte_count );
}

void* xi_bsp_mem_realloc( void* ptr, size_t byte_count )
{
    return ( void* )realloc( ptr, byte_count );
}

void xi_bsp_mem_free( void* ptr )
{
    free( ptr );
}
