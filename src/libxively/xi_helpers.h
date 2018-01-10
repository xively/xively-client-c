/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_HELPERS_H__
#define __XI_HELPERS_H__

#include <stdlib.h>
#include <limits.h>

#include "xi_common.h"
#include "xi_mqtt_message.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XI_MAX8_t ( 0xFF )
#define XI_MAX16_t ( 0xFFFF )
#define XI_MAX32_t ( 0xFFFFFFFF )

/* Our Message Payloads could be either character arrays or binary arrays.
 * This function will ready the payload as a character array and return
 * a null-terminated copy of that data.  You must free this data when you're
 * done with it.
 *
 * Returns NULL if msg is NULL or string allocation failed. */
char* xi_parse_message_payload_as_string( const xi_mqtt_message_t* msg );

/* Avoid using `strdup()` which can cause some problems with `free()`,
 * because of buggy implementations of `realloc()`. */
char* xi_str_dup( const char* s );

char* xi_str_cat( const char* s1, const char* s2 );
char* xi_str_cat_three( const char* s1, const char* s2, const char* s3 );

int xi_str_copy_untiln( char* dst, size_t dst_size, const char* src, char delim );

void xi_str_reposition_after_first_n_char( char ch, size_t num, const char** input_str );

char* xi_replace_with( char p, char r, char* buffer, size_t max_chars );

uint32_t xi_highest_bit_filter( uint32_t value );

/* if you need debugging rockets here it is - you can later on use addr2line -e
 * bin/linux/tests/tools/xi_libxively_driver -f address in order to see which line of code
 * it was*/
#if 0
void get_stack_trace();
#endif

#ifdef __cplusplus
}
#endif

#endif /* __XI_HELPERS_H__ */
