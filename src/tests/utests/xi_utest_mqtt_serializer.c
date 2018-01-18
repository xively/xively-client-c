/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"

#include "xively.h"
#include "xi_err.h"
#include "xi_helpers.h"
#include "xi_globals.h"
#include "xi_mqtt_serialiser.h"

#include "xi_memory_checks.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

typedef struct utest_mqtt_size_expectations_s
{
    size_t message_buffer_length;
    size_t remaining_bytes_length;
    uint8_t* reference_buffer;
} utest_mqtt_expectations_t;

typedef struct utest_mqtt_test_case_s
{
    xi_mqtt_message_t msg;
    utest_mqtt_expectations_t test_expectations;
} utest_mqtt_test_case_t;

static char topic_name[] = "xi/blue/v1/de289e01-cc13-11e4-a698-0a1f2727d969/d/"
                           "48d0cf57-cc13-11e4-a698-0a1f2727d969/channel_0/"
                           "dummyname";
static char content[] = "4 13 4 00 00 00 012";

// reference buffer generated via node.js mqtt serialiser for above data
static uint8_t reference_message_content[] = {
    0x30, 0x7f, 0x0,  0x6a, 0x78, 0x69, 0x2f, 0x62, 0x6c, 0x75, 0x65, 0x2f, 0x76,
    0x31, 0x2f, 0x64, 0x65, 0x32, 0x38, 0x39, 0x65, 0x30, 0x31, 0x2d, 0x63, 0x63,
    0x31, 0x33, 0x2d, 0x31, 0x31, 0x65, 0x34, 0x2d, 0x61, 0x36, 0x39, 0x38, 0x2d,
    0x30, 0x61, 0x31, 0x66, 0x32, 0x37, 0x32, 0x37, 0x64, 0x39, 0x36, 0x39, 0x2f,
    0x64, 0x2f, 0x34, 0x38, 0x64, 0x30, 0x63, 0x66, 0x35, 0x37, 0x2d, 0x63, 0x63,
    0x31, 0x33, 0x2d, 0x31, 0x31, 0x65, 0x34, 0x2d, 0x61, 0x36, 0x39, 0x38, 0x2d,
    0x30, 0x61, 0x31, 0x66, 0x32, 0x37, 0x32, 0x37, 0x64, 0x39, 0x36, 0x39, 0x2f,
    0x63, 0x68, 0x61, 0x6e, 0x6e, 0x65, 0x6c, 0x5f, 0x30, 0x2f, 0x64, 0x75, 0x6d,
    0x6d, 0x79, 0x6e, 0x61, 0x6d, 0x65, 0x34, 0x20, 0x31, 0x33, 0x20, 0x34, 0x20,
    0x30, 0x30, 0x20, 0x30, 0x30, 0x20, 0x30, 0x30, 0x20, 0x30, 0x31, 0x32};

#define make_static_desc( data, size )                                                   \
    {                                                                                    \
        ( uint8_t* )data, NULL, size, size, 0, XI_MEMORY_TYPE_UNMANAGED                  \
    }

xi_data_desc_t topic_name_desc = make_static_desc( topic_name, sizeof( topic_name ) - 1 );
xi_data_desc_t content_desc    = make_static_desc( content, sizeof( content ) - 1 );

utest_mqtt_test_case_t array_of_test_case[] = {
    {// test case 0
     {.publish = {.common = {{.common_bits = {0, 0, 0, XI_MQTT_TYPE_PUBLISH}}, 0},
                  &topic_name_desc,
                  0,
                  &content_desc}},
     {129, 127, ( uint8_t* )&reference_message_content}}};

void utest__serialize_publish__valid_data_border_case__size_is_correct_impl( void )
{
    xi_state_t local_state = XI_STATE_OK;
    XI_UNUSED( local_state );

    size_t i = 0;
    for ( ; i < XI_ARRAYSIZE( array_of_test_case ); ++i )
    {
        utest_mqtt_test_case_t* currrent_test_case = &array_of_test_case[i];

        //
        xi_state_t local_state = XI_STATE_OK;
        xi_mqtt_serialiser_t serializer;
        xi_mqtt_serialiser_init( &serializer );

        //
        xi_mqtt_message_t* msg = &currrent_test_case->msg;

        size_t message_len, remaining_len, payload_size = 0;
        local_state = xi_mqtt_serialiser_size( &message_len, &remaining_len,
                                               &payload_size, NULL, msg );

        tt_int_op( message_len, ==,
                   currrent_test_case->test_expectations.message_buffer_length );

        tt_int_op( remaining_len, ==,
                   currrent_test_case->test_expectations.remaining_bytes_length );

        xi_data_desc_t* buffer = xi_make_empty_desc_alloc( message_len - payload_size );
        XI_CHECK_MEMORY( buffer, local_state );

        xi_mqtt_serialiser_rc_t rc =
            xi_mqtt_serialiser_write( NULL, msg, buffer, message_len, remaining_len );

        tt_int_op( buffer->length, ==, message_len - payload_size );
        tt_int_op( rc, ==, XI_MQTT_SERIALISER_RC_SUCCESS );
        tt_int_op( memcmp( buffer->data_ptr,
                           currrent_test_case->test_expectations.reference_buffer,
                           buffer->length ),
                   ==, 0 );

        xi_free_desc( &buffer );
    }

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

    return;
end:
err_handling:;
}

#endif

XI_TT_TESTGROUP_BEGIN( utest_mqtt_serializer )

XI_TT_TESTCASE( utest__serialize_publish__valid_data_border_case__size_is_correct, {
    utest__serialize_publish__valid_data_border_case__size_is_correct_impl();
} )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
