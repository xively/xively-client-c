/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_data_desc.h"
#include "xi_allocator.h"
#include "xi_macros.h"
#include "xi_helpers.h"

xi_data_desc_t* xi_make_empty_desc_alloc( size_t capacity )
{
    assert( capacity > 0 );

    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_data_desc_t, data_desc, state );

    XI_ALLOC_BUFFER_AT( unsigned char, data_desc->data_ptr, capacity, state );

    data_desc->length      = 0;
    data_desc->capacity    = capacity;
    data_desc->memory_type = XI_MEMORY_TYPE_MANAGED;

    return data_desc;

err_handling:
    xi_free_desc( &data_desc );
    return 0;
}

xi_data_desc_t* xi_make_desc_from_buffer_copy( unsigned const char* buffer, size_t len )
{
    assert( buffer != 0 );

    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_data_desc_t, data_desc, state );
    XI_ALLOC_BUFFER_AT( unsigned char, data_desc->data_ptr, len, state );

    memcpy( data_desc->data_ptr, buffer, len );

    data_desc->capacity    = len;
    data_desc->length      = data_desc->capacity;
    data_desc->memory_type = XI_MEMORY_TYPE_MANAGED;

    return data_desc;

err_handling:
    xi_free_desc( &data_desc );
    return 0;
}

xi_data_desc_t* xi_make_desc_from_buffer_share( unsigned char* buffer, size_t len )
{
    assert( buffer != 0 );

    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_data_desc_t, data_desc, state );

    data_desc->capacity    = len;
    data_desc->length      = data_desc->capacity;
    data_desc->data_ptr    = buffer;
    data_desc->memory_type = XI_MEMORY_TYPE_UNMANAGED;

    return data_desc;

err_handling:
    xi_free_desc( &data_desc );
    return 0;
}

xi_data_desc_t* xi_make_desc_from_string_copy( const char* str )
{
    if ( NULL == str )
    {
        return NULL;
    }

    xi_state_t state = XI_STATE_OK;
    const size_t len = strlen( str );

    XI_ALLOC( xi_data_desc_t, data_desc, state );

    XI_ALLOC_BUFFER_AT( unsigned char, data_desc->data_ptr, len, state );
    memcpy( data_desc->data_ptr, str, len );

    data_desc->capacity    = len;
    data_desc->length      = data_desc->capacity;
    data_desc->memory_type = XI_MEMORY_TYPE_MANAGED;

    return data_desc;

err_handling:
    xi_free_desc( &data_desc );
    return 0;
}

xi_data_desc_t* xi_make_desc_from_string_share( const char* str )
{
    if ( NULL == str )
    {
        return NULL;
    }

    xi_state_t state = XI_STATE_OK;

    const size_t len = strlen( str );

    XI_ALLOC( xi_data_desc_t, data_desc, state );

    data_desc->data_ptr    = ( uint8_t* )str;
    data_desc->capacity    = len;
    data_desc->length      = data_desc->capacity;
    data_desc->memory_type = XI_MEMORY_TYPE_UNMANAGED;

    return data_desc;

err_handling:
    xi_free_desc( &data_desc );
    return 0;
}

xi_data_desc_t* xi_make_desc_from_float_copy( const float value )
{
    assert( sizeof( float ) == 4 );

    const char* acc = ( const char* )&value;

    xi_state_t state = XI_STATE_OK;
    size_t i         = 0;

    XI_ALLOC( xi_data_desc_t, data_desc, state );
    XI_ALLOC_BUFFER_AT( unsigned char, data_desc->data_ptr, 4, state );

    for ( i = 0; i < 4; ++i )
    {
        data_desc->data_ptr[i] = acc[i];
    }

    data_desc->capacity    = 4;
    data_desc->length      = 4;
    data_desc->memory_type = XI_MEMORY_TYPE_MANAGED;

    return data_desc;

err_handling:
    xi_free_desc( &data_desc );
    return 0;
}

void xi_free_desc( xi_data_desc_t** desc )
{
    if ( desc != NULL && *desc != NULL )
    {
        /* PRE-CONDITION */
        assert( ( *desc )->memory_type != XI_MEMORY_TYPE_UNKNOWN );

        if ( XI_MEMORY_TYPE_MANAGED == ( *desc )->memory_type )
        {
            XI_SAFE_FREE( ( *desc )->data_ptr );
        }

        XI_SAFE_FREE( ( *desc ) );
    }
}

uint8_t xi_data_desc_will_it_fit( const xi_data_desc_t* const desc, size_t len )
{
    assert( desc );

    if ( desc->capacity < ( desc->length + len ) )
    {
        return 0;
    }

    return 1;
}

/**
 * @brief xi_data_desc_find_pow2_mult
 *
 * Little explanation for this function:
 * what it does it calculates the new capacity for original value so that it
 * becomes new required memory limit - which is calculated from the desire
 * parameter.
 *
 * @param original size of already allocated memory
 * @param desire number of bytes that are required
 * @return multiplier for original value
 */
static uint32_t xi_data_desc_find_pow2_capacity( uint32_t original, uint32_t desire )
{
    if ( desire <= original )
        return original;

    uint32_t tmp = xi_highest_bit_filter( desire );
    if ( tmp < desire )
    {
        tmp <<= 1;
    }

    return tmp;
}

/**
 * @brief xi_data_desc_pow2_realloc_strategy
 *
 * this is some kind of equivalent of std::allocator which can be defined
 * to handle different allocation strategy like pooling.
 *
 * @param original
 * @param desired
 * @return
 */
uint32_t xi_data_desc_pow2_realloc_strategy( uint32_t original, uint32_t desired )
{
    uint32_t new_cap = xi_data_desc_find_pow2_capacity( original, desired );

    return new_cap;
}

xi_state_t xi_data_desc_realloc( xi_data_desc_t* desc,
                                 uint32_t new_capacity,
                                 xi_data_desc_realloc_strategy_t* strategy )
{
    /* PRECONDITION */
    assert( desc->memory_type != XI_MEMORY_TYPE_UNKNOWN );

    if ( desc == NULL || strategy == NULL )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t ret_state = XI_STATE_OK;

    unsigned char* old = desc->data_ptr;

    uint32_t calulated_capacity = ( *strategy )( desc->capacity, new_capacity );

    XI_ALLOC_BUFFER( unsigned char, new_buf, calulated_capacity, ret_state );

    memcpy( new_buf, old, desc->capacity );

    desc->data_ptr = new_buf;
    desc->capacity = calulated_capacity;

    if ( XI_MEMORY_TYPE_MANAGED == desc->memory_type )
    {
        XI_SAFE_FREE( old );
    }

    desc->memory_type = XI_MEMORY_TYPE_MANAGED;

err_handling:
    return ret_state;
}

xi_state_t xi_data_desc_assure_buf_len( xi_data_desc_t* out, size_t len )
{
    if ( xi_data_desc_will_it_fit( out, len ) == 0 )
    {
        return xi_data_desc_realloc( out, out->length + len,
                                     &xi_data_desc_pow2_realloc_strategy );
    }

    return XI_STATE_OK;
}

xi_state_t xi_data_desc_append_data_resize( xi_data_desc_t* out,
                                            const char* const data,
                                            const size_t len )
{
    if ( out == NULL || data == NULL )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t ret_state = XI_STATE_OK;

    XI_CHECK_STATE( ret_state = xi_data_desc_assure_buf_len( out, len ) );

    memcpy( out->data_ptr + out->length, data, len );
    out->length += len;

err_handling:
    return ret_state;
}

xi_state_t
xi_data_desc_append_bytes( xi_data_desc_t* out, const uint8_t* bytes, const size_t len )
{
    if ( out == NULL || bytes == NULL )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t local_state = XI_BUFFER_OVERFLOW;

    if ( xi_data_desc_will_it_fit( out, len ) == 1 )
    {
        memcpy( out->data_ptr + out->length, bytes, len );
        out->length += len;
        local_state = XI_STATE_OK;
    }

    return local_state;
}

xi_state_t xi_data_desc_append_byte( xi_data_desc_t* out, const uint8_t byte )
{
    if ( out == NULL )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t local_state = XI_BUFFER_OVERFLOW;

    if ( xi_data_desc_will_it_fit( out, 1 ) == 1 )
    {
        out->data_ptr[out->length] = byte;
        out->length += 1;
        local_state = XI_STATE_OK;
    }

    return local_state;
}

extern xi_state_t
xi_data_desc_append_data( xi_data_desc_t* out, const xi_data_desc_t* in )
{
    if ( out == NULL || in == NULL )
    {
        return XI_INVALID_PARAMETER;
    }

    return xi_data_desc_append_bytes( out, in->data_ptr, in->length );
}

#if XI_DEBUG_OUTPUT
void xi_debug_data_desc_dump( const xi_data_desc_t* buffer )
{
    if ( buffer == NULL )
    {
        xi_debug_printf( "NULL" );
        return;
    }

    xi_debug_printf( "[%" SCNu32 "] ", buffer->length );

    char hex = 0;
    size_t i = 0;
    for ( ; i < buffer->length; ++i )
    {
        if ( ( uint8_t )buffer->data_ptr[i] < 0x20 ||
             ( uint8_t )buffer->data_ptr[i] > 0x7e )
        {
            hex = 1;
            break;
        }
    }

    if ( hex )
    {
        xi_debug_data_desc_dump_hex( buffer );
    }
    else
    {
        xi_debug_data_desc_dump_ascii( buffer );
    }
}

void xi_debug_data_desc_dump_ascii( const xi_data_desc_t* buffer )
{
    if ( buffer == NULL )
    {
        xi_debug_printf( "NULL" );
        return;
    }

    size_t i = 0;
    for ( ; i < buffer->length; ++i )
    {
        xi_debug_printf( "%c", buffer->data_ptr[i] );
    }
}

void xi_debug_data_desc_dump_hex( const xi_data_desc_t* buffer )
{
    if ( buffer == NULL )
    {
        xi_debug_printf( "NULL" );
        return;
    }

    size_t i = 0;
    for ( ; i < buffer->length; ++i )
    {
        xi_debug_printf( "%02x ", buffer->data_ptr[i] );
    }
}
#endif
