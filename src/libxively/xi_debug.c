/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <string.h>

#include "xi_debug.h"
#include "xi_allocator.h"

#if XI_DEBUG_OUTPUT
void xi_debug_data_logger_impl( const char* msg, const xi_data_desc_t* data_desc )
{
    char* tmp = xi_alloc( 1024 );
    memset( tmp, 0, 1024 );

    size_t counter = 0;

    unsigned short i = 0;
    for ( ; i < data_desc->length; ++i )
    {
        char c = data_desc->data_ptr[i];

        if ( c < 33 || c > 126 )
        {
            counter += sprintf( tmp + counter, "0x%02x", c );
        }
        else
        {
            counter += sprintf( tmp + counter, "\'%c\'", c );
        }

        counter += sprintf( tmp + counter, " " );
    }

    xi_debug_printf( "%s = [%s]\n", msg, tmp );

    xi_free( tmp );
}

/** Trims the absolute file path to just the file name */
const char* xi_debug_dont_print_the_path( const char* msg )
{
    const char* tmp = msg;

    /* let's find the end */
    while ( *( tmp ) != '\0' )
    {
        ++tmp;
    }

    /* let's crawl back to the beginning */
    while ( *tmp != '/' && *tmp != '\\' && tmp != msg )
    {
        --tmp;
    }

    if ( *tmp != '\0' )
    {
        ++tmp;
    }

    return tmp;
}
#endif
