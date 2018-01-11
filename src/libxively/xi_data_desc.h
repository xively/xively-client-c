/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_DATA_DESC_H__
#define __XI_DATA_DESC_H__

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include "xi_err.h"
#include "xi_memory_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct data_desc_s
{
    uint8_t* data_ptr;
    struct data_desc_s* __next;
    uint32_t capacity;
    uint32_t length;
    uint32_t curr_pos;
    xi_memory_type_t memory_type;
} xi_data_desc_t;

typedef uint32_t( xi_data_desc_realloc_strategy_t )( uint32_t, uint32_t );

extern xi_data_desc_t* xi_make_empty_desc_alloc( size_t capacity );

extern xi_data_desc_t*
xi_make_desc_from_buffer_copy( unsigned const char* buffer, size_t len );

extern xi_data_desc_t*
xi_make_desc_from_buffer_share( unsigned char* buffer, size_t len );

extern xi_data_desc_t* xi_make_desc_from_string_copy( const char* str );

extern xi_data_desc_t* xi_make_desc_from_string_share( const char* str );

extern xi_data_desc_t* xi_make_desc_from_float_copy( const float value );

extern void xi_free_desc( xi_data_desc_t** desc );

extern uint8_t xi_data_desc_will_it_fit( const xi_data_desc_t* const, size_t len );

uint32_t xi_data_desc_pow2_realloc_strategy( uint32_t original, uint32_t desired );

extern xi_state_t xi_data_desc_realloc( xi_data_desc_t* desc,
                                        uint32_t new_capacity,
                                        xi_data_desc_realloc_strategy_t* strategy );

extern xi_state_t xi_data_desc_assure_buf_len( xi_data_desc_t* out, size_t len );

extern xi_state_t xi_data_desc_append_data_resize( xi_data_desc_t* out,
                                                   const char* const data,
                                                   const size_t len );

extern xi_state_t
xi_data_desc_append_bytes( xi_data_desc_t* out, const uint8_t* bytes, const size_t len );

extern xi_state_t xi_data_desc_append_byte( xi_data_desc_t* out, const uint8_t byte );

extern xi_state_t
xi_data_desc_append_data( xi_data_desc_t* out, const xi_data_desc_t* in );

#if XI_DEBUG_OUTPUT
extern void xi_debug_data_desc_dump( const xi_data_desc_t* buffer );
extern void xi_debug_data_desc_dump_ascii( const xi_data_desc_t* buffer );
extern void xi_debug_data_desc_dump_hex( const xi_data_desc_t* buffer );
#else
#define xi_debug_data_desc_dump( ... )
#define xi_debug_data_desc_dump_ascii( ... )
#define xi_debug_data_desc_dump_hex( ... )
#endif

#ifdef __cplusplus
}
#endif

#endif /* __XI_DATA_DESC_H__ */
