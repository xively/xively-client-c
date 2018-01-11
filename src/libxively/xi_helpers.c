/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <string.h>

#include "xi_debug.h"
#include "xi_helpers.h"
#include "xi_allocator.h"
#include "xi_macros.h"

char* xi_parse_message_payload_as_string( const xi_mqtt_message_t* msg )
{
    if ( NULL == msg )
    {
        xi_debug_logger( "message null returning" );
        return NULL;
    }

    char* payload = ( char* )xi_alloc( msg->publish.content->length + 1 );

    if ( NULL == payload )
    {
        xi_debug_logger( "allocated string null, returning" );
        return NULL;
    }

    memcpy( payload, msg->publish.content->data_ptr, msg->publish.content->length );

    payload[msg->publish.content->length] = '\0';

    return payload;
}

char* xi_str_dup( const char* s )
{
    /* PRECONDITIONS */
    assert( s != 0 );

    const size_t len = strlen( s );
    char* ret        = xi_alloc( len + 1 );
    if ( ret == 0 )
    {
        return 0;
    }
    memcpy( ret, s, len + 1 );
    return ret;
}

char* xi_str_cat( const char* s1, const char* s2 )
{
    assert( 0 != s1 );
    assert( 0 != s2 );

    size_t len1 = strlen( s1 );
    size_t len2 = strlen( s2 );
    char* ret   = xi_alloc( len1 + len2 + 1 );
    if ( ret == 0 )
    {
        return 0;
    }

    memcpy( ret, s1, len1 );
    memcpy( ret + len1, s2, len2 + 1 );

    return ret;
}

char* xi_str_cat_three( const char* s1, const char* s2, const char* s3 )
{
    assert( 0 != s1 );
    assert( 0 != s2 );
    assert( 0 != s3 );

    size_t len1 = strlen( s1 );
    size_t len2 = strlen( s2 );
    size_t len3 = strlen( s3 );

    char* ret = xi_alloc( len1 + len2 + len3 + 1 );
    if ( ret == 0 )
    {
        return 0;
    }

    memcpy( ret, s1, len1 );
    memcpy( ret + len1, s2, len2 );
    memcpy( ret + len1 + len2, s3, len3 + 1 );

    return ret;
}

int xi_str_copy_untiln( char* dst, size_t dst_size, const char* src, char delim )
{
    /* PRECONDITIONS */
    assert( dst != 0 );
    assert( dst_size > 1 );
    assert( src != 0 );

    size_t counter   = 0;
    size_t real_size = dst_size - 1;

    while ( *src != delim && counter < real_size && *src != '\0' )
    {
        *dst++ = *src++;
        counter++;
    }

    *dst = '\0';
    return counter;
}

void xi_str_reposition_after_first_n_char( char ch, size_t num, const char** input_str )
{
    if ( NULL == input_str || NULL == *input_str )
    {
        return;
    }

    for ( ; '\0' != **input_str && 0 < num; ++*input_str )
    {
        if ( ch == **input_str )
        {
            --num;
        }
    }
}

char* xi_replace_with( char p, char r, char* buffer, size_t max_chars )
{
    char* c = buffer;

    while ( *c != '\0' && max_chars-- )
    {
        if ( *c == p )
        {
            *c = r;
        }
        c++;
    }

    return c;
}

/**
 * @brief leave_heighest_bit
 *
 * function leavs the heighest bit set to 1
 *
 * @param value
 * @return value with the heighest bit set
 */
uint32_t xi_highest_bit_filter( uint32_t value )
{
    value |= ( value >> 1 );
    value |= ( value >> 2 );
    value |= ( value >> 4 );
    value |= ( value >> 8 );
    value |= ( value >> 16 );

    return value & ~( value >> 1 );
}

#if 0

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void get_stack_trace()
{
    int j, nptrs;
#define SIZE 100
    void *buffer[100];
    char **strings;

    nptrs = backtrace(buffer, SIZE);
    printf( "backtrace() returned %d addresses\n", nptrs);
    fflush( stdout );

    /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
       would produce similar output to the following: */

    strings = backtrace_symbols(buffer, nptrs);
    if (strings == NULL) {
        perror("backtrace_symbols");
        exit(EXIT_FAILURE);
    }

    for (j = 0; j < nptrs; j++)
    {
        printf( "%s\n", strings[j]);
        fflush( stdout );
    }

   free(strings);
}
#endif
