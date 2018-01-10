/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"
#include "xi_utest_basic_testcase_frame.h"

#include "xively.h"
#include "xi_err.h"
#include "xi_data_desc.h"
#include "xi_macros.h"

#include "xi_bsp_rng.h"
#include "xi_memory_checks.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#endif

XI_TT_TESTGROUP_BEGIN( utest_data_desc )

XI_TT_TESTCASE( utest__xi_data_desc_will_it_fit__valid_data__will_fit, {
    unsigned char buffer[32] = {'\0'};
    xi_data_desc_t test      = {buffer, NULL, 32, 0, 0, XI_MEMORY_TYPE_UNMANAGED};

    tt_want_int_op( xi_data_desc_will_it_fit( &test, 16 ), ==, 1 );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

} )

XI_TT_TESTCASE( utest__xi_data_desc_will_it_fit__valid_data__will_not_fit, {
    unsigned char buffer[32] = {'\0'};
    xi_data_desc_t test      = {buffer, NULL, 32, 0, 0, XI_MEMORY_TYPE_UNMANAGED};

    tt_want_int_op( xi_data_desc_will_it_fit( &test, 33 ), ==, 0 );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

} )

XI_TT_TESTCASE( utest__xi_data_desc_will_it_fit__valid_data__will_fit_border, {
    unsigned char buffer[32] = {'\0'};
    xi_data_desc_t test      = {buffer, NULL, 32, 0, 0, XI_MEMORY_TYPE_UNMANAGED};

    tt_want_int_op( xi_data_desc_will_it_fit( &test, 32 ), ==, 1 );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

} )

XI_TT_TESTCASE( utest__xi_data_desc_will_it_fit__valid_data__will_not_fit_border, {
    unsigned char buffer[32] = {'\0'};
    xi_data_desc_t test      = {buffer, NULL, 32, 0, 0, XI_MEMORY_TYPE_UNMANAGED};

    tt_want_int_op( xi_data_desc_will_it_fit( &test, 33 ), ==, 0 );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
} )

XI_TT_TESTCASE( utest__xi_data_desc_pow2_realloc_strategy__valid_data__will_ret_64, {
    tt_want_int_op( xi_data_desc_pow2_realloc_strategy( 32, 55 ), ==, 64 );
    tt_want_int_op( xi_data_desc_pow2_realloc_strategy( 32, 33 ), ==, 64 );
    tt_want_int_op( xi_data_desc_pow2_realloc_strategy( 32, 63 ), ==, 64 );
    tt_want_int_op( xi_data_desc_pow2_realloc_strategy( 32, 64 ), ==, 64 );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

} )

XI_TT_TESTCASE( utest__xi_data_desc_pow2_realloc_strategy__valid_data__will_ret_128, {
    tt_want_int_op( xi_data_desc_pow2_realloc_strategy( 32, 67 ), ==, 128 );
    tt_want_int_op( xi_data_desc_pow2_realloc_strategy( 32, 90 ), ==, 128 );
    tt_want_int_op( xi_data_desc_pow2_realloc_strategy( 32, 127 ), ==, 128 );
    tt_want_int_op( xi_data_desc_pow2_realloc_strategy( 32, 128 ), ==, 128 );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

} )

XI_TT_TESTCASE( utest__xi_data_desc_pow2_realloc_strategy__valid_data__will_ret_512, {
    tt_want_int_op( xi_data_desc_pow2_realloc_strategy( 2, 257 ), ==, 512 );
    tt_want_int_op( xi_data_desc_pow2_realloc_strategy( 2, 400 ), ==, 512 );
    tt_want_int_op( xi_data_desc_pow2_realloc_strategy( 2, 511 ), ==, 512 );
    tt_want_int_op( xi_data_desc_pow2_realloc_strategy( 2, 512 ), ==, 512 );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

} )

XI_TT_TESTCASE( utest__xi_data_desc_pow2_realloc_strategy__valid_data__will_ret_512_loop,
                {
                    uint32_t i = 257;
                    for ( ; i < 512; ++i )
                    {
                        tt_want_int_op( xi_data_desc_pow2_realloc_strategy( 2, i ), ==,
                                        512 );
                    }

                    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
                } )

XI_TT_TESTCASE( utest__xi_data_desc_realloc__valid_data__data_size_to_128, {
    xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

    xi_state_t ret_state =
        xi_data_desc_realloc( desc, 77, &xi_data_desc_pow2_realloc_strategy );

    tt_want_int_op( ret_state, ==, XI_STATE_OK );
    tt_want_int_op( desc->capacity, ==, 128 );

    xi_free_desc( &desc );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
} )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_make_desc_from_buffer_copy__valid_data__buffer_copied,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_state_t ret_state = XI_STATE_OK;

        const size_t size = 67;

        XI_ALLOC_BUFFER( unsigned char, test_buffer, size, ret_state );

        size_t i = 0;
        for ( ; i < size; ++i )
        {
            test_buffer[i] = xi_bsp_rng_get() % 256;
        }

        xi_data_desc_t* data_desc = xi_make_desc_from_buffer_copy( test_buffer, size );

        tt_ptr_op( data_desc, !=, NULL );
        tt_int_op( data_desc->capacity, ==, size );
        tt_int_op( data_desc->length, ==, size );
        tt_int_op( XI_MEMORY_TYPE_MANAGED, ==, data_desc->memory_type );
        tt_ptr_op( data_desc->data_ptr, !=, test_buffer );
        tt_int_op( memcmp( data_desc->data_ptr, test_buffer, size ), ==, 0 );

        XI_SAFE_FREE( test_buffer );
        xi_free_desc( &data_desc );

        return;
    err_handling:
    end:
        tt_fail();
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_make_desc_from_buffer_share__valid_data__buffer_shared,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_state_t ret_state = XI_STATE_OK;

        const size_t size = 67;

        XI_ALLOC_BUFFER( unsigned char, test_buffer, size, ret_state );

        size_t i = 0;
        for ( ; i < size; ++i )
        {
            test_buffer[i] = xi_bsp_rng_get() % 256;
        }

        xi_data_desc_t* data_desc = xi_make_desc_from_buffer_share( test_buffer, size );

        tt_ptr_op( data_desc, !=, NULL );
        tt_int_op( data_desc->capacity, ==, size );
        tt_int_op( data_desc->length, ==, size );
        tt_int_op( XI_MEMORY_TYPE_UNMANAGED, ==, data_desc->memory_type );
        tt_ptr_op( data_desc->data_ptr, ==, test_buffer );
        tt_int_op( memcmp( data_desc->data_ptr, test_buffer, size ), ==, 0 );

        unsigned char new_value = test_buffer[0] + 1;
        test_buffer[0] += 1;

        tt_int_op( data_desc->data_ptr[0], ==, new_value );

        xi_free_desc( &data_desc );
        XI_SAFE_FREE( test_buffer );

        return;
    err_handling:
    end:
        tt_fail();
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_make_desc_from_string_copy__valid_data__data_copied,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_state_t ret_state      = XI_STATE_OK;
        char* origin_string       = NULL;
        xi_data_desc_t* data_desc = NULL;

        size_t size = 4;
        for ( ; size < 256; size *= 2 )
        {
            XI_ALLOC_BUFFER_AT( char, origin_string, size, ret_state );
            size_t i = 0;
            for ( ; i < size - 1; ++i )
            {
                origin_string[i] = ( xi_bsp_rng_get() % 255 ) + 1;
            }

            tt_int_op( strlen( origin_string ), ==, size - 1 );

            data_desc = xi_make_desc_from_string_copy( origin_string );

            tt_ptr_op( data_desc, !=, NULL );
            tt_int_op( data_desc->capacity, ==, size - 1 );
            tt_int_op( data_desc->length, ==, size - 1 );
            tt_int_op( XI_MEMORY_TYPE_MANAGED, ==, data_desc->memory_type );
            tt_ptr_op( data_desc->data_ptr, !=, origin_string );
            tt_int_op( memcmp( data_desc->data_ptr, origin_string, size - 1 ), ==, 0 );

            xi_free_desc( &data_desc );
            XI_SAFE_FREE( origin_string );
        }

        return;
    err_handling:
    end:
        xi_free_desc( &data_desc );
        XI_SAFE_FREE( origin_string );
        tt_fail();
    } )

XI_TT_TESTCASE( utest__xi_make_desc_from_string_copy__null_data__null_returned, {
    char* origin_string       = NULL;
    xi_data_desc_t* data_desc = NULL;

    data_desc = xi_make_desc_from_string_copy( origin_string );

    tt_ptr_op( data_desc, ==, NULL );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
    return;
end:
    tt_fail();
} )

XI_TT_TESTCASE( utest__xi_make_desc_from_string_share__null_data__null_returned, {
    char* origin_string       = NULL;
    xi_data_desc_t* data_desc = NULL;

    data_desc = xi_make_desc_from_string_share( origin_string );

    tt_ptr_op( data_desc, ==, NULL );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
    return;
end:
    tt_fail();
} )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_make_desc_from_string_share__valid_data__data_shared,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_state_t ret_state = XI_STATE_OK;

        char* origin_string       = NULL;
        xi_data_desc_t* data_desc = NULL;

        size_t size = 4;
        for ( ; size < 256; size *= 2 )
        {
            XI_ALLOC_BUFFER_AT( char, origin_string, size, ret_state );
            size_t i = 0;
            for ( ; i < size - 1; ++i )
            {
                origin_string[i] = ( xi_bsp_rng_get() % 255 ) + 1;
            }

            tt_int_op( strlen( origin_string ), ==, size - 1 );

            data_desc = xi_make_desc_from_string_share( origin_string );

            tt_ptr_op( data_desc, !=, NULL );
            tt_int_op( data_desc->capacity, ==, size - 1 );
            tt_int_op( data_desc->length, ==, size - 1 );
            tt_int_op( XI_MEMORY_TYPE_UNMANAGED, ==, data_desc->memory_type );
            tt_ptr_op( data_desc->data_ptr, ==, origin_string );
            tt_int_op( memcmp( data_desc->data_ptr, origin_string, size - 1 ), ==, 0 );

            origin_string[0] += 1;
            tt_int_op( memcmp( data_desc->data_ptr, origin_string, size - 1 ), ==, 0 );

            xi_free_desc( &data_desc );
            XI_SAFE_FREE( origin_string );
        }

        return;
    err_handling:
    end:
        xi_free_desc( &data_desc );
        XI_SAFE_FREE( origin_string );
        tt_fail();
    } )

XI_TT_TESTCASE( utest__xi_data_desc_realloc__valid_data__data_copied, {
    const char test_string[] = "this is a test string 32434 /asdf/";
    xi_data_desc_t* desc     = xi_make_desc_from_string_copy( test_string );

    xi_state_t ret_state =
        xi_data_desc_realloc( desc, 77, &xi_data_desc_pow2_realloc_strategy );

    tt_want_int_op( ret_state, ==, XI_STATE_OK );
    tt_want_int_op( memcmp( desc->data_ptr, test_string, sizeof( test_string ) - 1 ), ==,
                    0 );

    xi_free_desc( &desc );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
} )

XI_TT_TESTCASE( utest__xi_data_desc_realloc__valid_data__size_not_changed, {
    xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

    xi_state_t ret_state =
        xi_data_desc_realloc( desc, 32, &xi_data_desc_pow2_realloc_strategy );

    tt_want_int_op( ret_state, ==, XI_STATE_OK );
    tt_want_int_op( desc->capacity, ==, 32 );

    xi_free_desc( &desc );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
} )

XI_TT_TESTCASE(
    utest__xi_data_desc_append_data_resize__valid_data__write_string_and_realloc, {
        const char test_string[] = "this is a test string which will test this function";

        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 2 );

        xi_state_t state = xi_data_desc_append_data_resize( desc, test_string,
                                                            sizeof( test_string ) - 1 );

        tt_want_int_op( state, ==, XI_STATE_OK );
        tt_want_int_op( desc->capacity, ==, 64 );
        tt_want_int_op( desc->length, ==, sizeof( test_string ) - 1 );
        tt_want_int_op( memcmp( desc->data_ptr, test_string, sizeof( test_string ) - 1 ),
                        ==, 0 );

        xi_free_desc( &desc );

        tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
    } )

XI_TT_TESTCASE(
    utest__xi_data_desc_append_data_resize__valid_data__write_string_twice_and_realloc, {
        const char test_string[] = "this is a test string which will test this function";
        const size_t test_string_size = sizeof( test_string ) - 1;

        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 2 );

        xi_state_t state =
            xi_data_desc_append_data_resize( desc, test_string, test_string_size );

        tt_want_int_op( state, ==, XI_STATE_OK );

        state = xi_data_desc_append_data_resize( desc, test_string, test_string_size );

        tt_want_int_op( state, ==, XI_STATE_OK );
        tt_want_int_op( desc->capacity, ==, 128 );
        tt_want_int_op( desc->length, ==, test_string_size * 2 );
        tt_want_int_op( memcmp( desc->data_ptr, test_string, test_string_size ), ==, 0 );
        tt_want_int_op(
            memcmp( desc->data_ptr + test_string_size, test_string, test_string_size ),
            ==, 0 );

        xi_free_desc( &desc );

        tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
    } )

XI_TT_TESTCASE(
    utest__xi_data_desc_append_byte__valid_data__write_byte_and_check_if_the_size_is_correct,
    {
        xi_data_desc_t* desc   = xi_make_empty_desc_alloc( 2 );
        xi_state_t local_state = XI_STATE_OK;

        tt_want_int_op( desc->length, ==, 0 );

        local_state = xi_data_desc_append_byte( desc, 'c' );

        tt_want_int_op( local_state, ==, XI_STATE_OK );
        tt_want_int_op( desc->length, ==, 1 );
        tt_want_int_op( desc->data_ptr[0], ==, 'c' );

        local_state = xi_data_desc_append_byte( desc, 'a' );

        tt_want_int_op( local_state, ==, XI_STATE_OK );
        tt_want_int_op( desc->length, ==, 2 );
        tt_want_int_op( desc->data_ptr[0], ==, 'c' );
        tt_want_int_op( desc->data_ptr[1], ==, 'a' );

        local_state = xi_data_desc_append_byte( desc, 'z' );

        tt_want_int_op( local_state, ==, XI_BUFFER_OVERFLOW );
        tt_want_int_op( desc->length, ==, 2 );
        tt_want_int_op( desc->data_ptr[0], ==, 'c' );
        tt_want_int_op( desc->data_ptr[1], ==, 'a' );

        xi_free_desc( &desc );

        tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
    } )

XI_TT_TESTCASE(
    utest__xi_data_desc_append_bytes__valid_data__write_bytes_and_check_if_the_size_is_correct,
    {
        const char data[]      = "123456789";
        xi_data_desc_t* desc   = xi_make_empty_desc_alloc( 18 );
        xi_state_t local_state = XI_STATE_OK;

        tt_want_int_op( desc->length, ==, 0 );

        local_state =
            xi_data_desc_append_bytes( desc, ( const uint8_t* )data, strlen( data ) );

        tt_want_int_op( local_state, ==, XI_STATE_OK );
        tt_want_int_op( desc->length, ==, strlen( data ) );
        tt_want_int_op( memcmp( desc->data_ptr, data, strlen( data ) ), ==, 0 );

        local_state =
            xi_data_desc_append_bytes( desc, ( const uint8_t* )data, strlen( data ) );

        tt_want_int_op( local_state, ==, XI_STATE_OK );
        tt_want_int_op( desc->length, ==, strlen( data ) * 2 );
        tt_want_int_op( memcmp( desc->data_ptr, data, strlen( data ) ), ==, 0 );
        tt_want_int_op( memcmp( desc->data_ptr + strlen( data ), data, strlen( data ) ),
                        ==, 0 );

        local_state =
            xi_data_desc_append_bytes( desc, ( const uint8_t* )data, strlen( data ) );

        tt_want_int_op( local_state, ==, XI_BUFFER_OVERFLOW );
        tt_want_int_op( desc->length, ==, strlen( data ) * 2 );
        tt_want_int_op( memcmp( desc->data_ptr, data, strlen( data ) ), ==, 0 );
        tt_want_int_op( memcmp( desc->data_ptr + strlen( data ), data, strlen( data ) ),
                        ==, 0 );

        xi_free_desc( &desc );

        tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
    } )

XI_TT_TESTCASE(
    utest__xi_data_desc_append_data_desc__valid_data__write_bytes_and_check_if_the_size_is_correct,
    {
        xi_data_desc_t* desc   = xi_make_empty_desc_alloc( 18 );
        xi_data_desc_t* src    = xi_make_desc_from_string_copy( "123456789" );
        xi_state_t local_state = XI_STATE_OK;

        tt_want_int_op( desc->length, ==, 0 );

        local_state = xi_data_desc_append_data( desc, src );

        tt_want_int_op( local_state, ==, XI_STATE_OK );
        tt_want_int_op( desc->length, ==, src->length );
        tt_want_int_op( memcmp( desc->data_ptr, src->data_ptr, src->length ), ==, 0 );

        local_state = xi_data_desc_append_data( desc, src );

        tt_want_int_op( local_state, ==, XI_STATE_OK );
        tt_want_int_op( desc->length, ==, src->length * 2 );
        tt_want_int_op( memcmp( desc->data_ptr, src->data_ptr, src->length ), ==, 0 );
        tt_want_int_op(
            memcmp( desc->data_ptr + src->length, src->data_ptr, src->length ), ==, 0 );

        local_state = xi_data_desc_append_data( desc, src );

        tt_want_int_op( local_state, ==, XI_BUFFER_OVERFLOW );
        tt_want_int_op( desc->length, ==, src->length * 2 );
        tt_want_int_op( memcmp( desc->data_ptr, src->data_ptr, src->length ), ==, 0 );
        tt_want_int_op(
            memcmp( desc->data_ptr + src->length, src->data_ptr, src->length ), ==, 0 );

        xi_free_desc( &src );
        xi_free_desc( &desc );

        tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
    } )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
