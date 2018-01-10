/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"

#include "xively_senml.h"
#include "xi_senml_json_serializer.h"
#include "xi_helpers.h"
#include "xi_macros.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

static const char xi_senml_cmp[] =
    "{\"e\":[{\"n\":\"http://test.of.name/"
    "named_measure\",\"v\":22.02},{\"bv\":false,\"t\":-120},{\"sv\":\"Hakuna "
    "Matata!\",\"t\":123}],\"bn\":\"http://base.time.com/the/best/base/time/"
    "\",\"bt\":23,\"bu\":\"V\"}";

#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

XI_TT_TESTGROUP_BEGIN( utest_senml_serialization )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_init__valid_data__buffer_filled_with_proper_beg, {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_init( desc );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "{\"e\":[", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_key_without_coln__valid_data__buffer_filled_with_proper_key,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_key( desc, "key", 0 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "key", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_key_with_coln__valid_data__buffer_filled_with_proper_key,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_key( desc, "key", 1 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, ",key", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_string__valid_data__buffer_filled_with_string_data, {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_string( desc, "a string" );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "\"a string\"", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_float__valid_data__buffer_filled_with_serialized_float,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_float( desc, 1.12f );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "1.12", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_float__valid_data_negative__buffer_filled_with_serialized_float,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_float( desc, -32.12f );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "-32.12", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_int__valid_data__serialized_float_int_the_buffer, {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_int( desc, 43 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "43", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_int__valid_data_negative__serialized_float_int_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_int( desc, -143 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "-143", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_boolean__valid_data_false__serialized_boolean_int_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_boolean( desc, 0 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "false", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_boolean__valid_data_true__serialized_boolean_int_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_boolean( desc, 1 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "true", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_name__valid_data__serialized_name_in_the_buffer, {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_name( desc, "name", 0 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "\"n\":\"name\"", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_name__valid_data_coln__serialized_name_in_the_buffer, {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_name( desc, "name", 12 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, ",\"n\":\"name\"", desc->length ), ==,
                        0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_float_value__valid_data__serialized_value_float_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_float_value( desc, 12.01, 0 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "\"v\":12.01", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_float_value__valid_data_coln__serialized_value_float_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_float_value( desc, 12.01, 1 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, ",\"v\":12.01", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_string_value__valid_data__serialized_value_string_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_string_value( desc, "string_value", 0 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "\"sv\":\"string_value\"", desc->length ),
                        ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_string_value__valid_data_coln__serialized_value_string_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_string_value( desc, "string_value", 1 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op(
            memcmp( desc->data_ptr, ",\"sv\":\"string_value\"", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_boolean_value__valid_data_false__serialized_value_boolean_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_boolean_value( desc, 0, 0 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "\"bv\":false", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_boolean_value__valid_data_true__serialized_value_boolean_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_boolean_value( desc, 1, 0 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "\"bv\":true", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_boolean_value__valid_data_true_coln__serialized_value_boolean_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_boolean_value( desc, 1, 1 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, ",\"bv\":true", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_units__valid_data__serialized_value_units_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_units( desc, "V", 0 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "\"u\":\"V\"", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_units__valid_data_coln__serialized_value_units_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_units( desc, "V", 1 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, ",\"u\":\"V\"", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_time__valid_data__serialized_value_units_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_time( desc, 123, 0 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "\"t\":123", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_time__valid_data_coln__serialized_value_units_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_time( desc, 123, 1 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, ",\"t\":123", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_update_time__valid_data__serialized_value_units_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_update_time( desc, 123, 0 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "\"ut\":123", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_update_time__valid_data_coln__serialized_value_units_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_update_time( desc, 123, 1 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, ",\"ut\":123", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_base_name__valid_data__serialized_value_units_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_base_name( desc, "base_name", 0 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "\"bn\":\"base_name\"", desc->length ),
                        ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_base_name__valid_data_coln__serialized_value_units_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_base_name( desc, "base_name", 1 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, ",\"bn\":\"base_name\"", desc->length ),
                        ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_base_units__valid_data__serialized_value_units_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_base_units( desc, "base_units", 0 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "\"bu\":\"base_units\"", desc->length ),
                        ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_base_units__valid_data_coln__serialized_value_units_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_base_units( desc, "base_units", 1 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, ",\"bu\":\"base_units\"", desc->length ),
                        ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_base_time__valid_data__serialized_value_units_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_base_time( desc, 213, 0 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "\"bt\":213", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_base_time__valid_data_coln__serialized_value_units_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_base_time( desc, 213, 1 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, ",\"bt\":213", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_close__valid_data__serialized_value_units_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_close( desc );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "}", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_close_entries__valid_data__serialized_value_units_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        ret_state = xi_senml_json_serialize_close_entries( desc );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "]", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_value_set__valid_data_float__serialized_value_units_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        xi_senml_value_t value = {{2.3f}, XI_SENML_VALUE_TYPE_FLOAT};

        ret_state = xi_senml_json_serialize_value_set( desc, &value, 0 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "\"v\":2.3", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_value_set__valid_data_string__serialized_value_units_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        xi_senml_value_t value = {{.string_value = "string_value"},
                                  XI_SENML_VALUE_TYPE_STRING};

        ret_state = xi_senml_json_serialize_value_set( desc, &value, 0 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "\"sv\":\"string_value\"", desc->length ),
                        ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_value_set__valid_data_boolean__serialized_value_units_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        xi_senml_value_t value = {{.boolean_value = 0}, XI_SENML_VALUE_TYPE_BOOLEAN};

        ret_state = xi_senml_json_serialize_value_set( desc, &value, 0 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "\"bv\":false", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_value_set__valid_data_boolean_coln__serialized_value_units_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        xi_senml_value_t value = {{.boolean_value = 0}, XI_SENML_VALUE_TYPE_BOOLEAN};

        ret_state = xi_senml_json_serialize_value_set( desc, &value, 1 );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, ",\"bv\":false", desc->length ), ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_entry__valid_data__serialized_value_units_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        xi_senml_entry_t value = {
            "name", "units", {{.boolean_value = 0}, XI_SENML_VALUE_TYPE_BOOLEAN},
            0,      0,       {1, 1, 1, 1, 1},
            0};

        ret_state = xi_senml_json_serialize_entry( desc, &value, 0 );

        // printf( "%s", desc->data_ptr );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, "{\"n\":\"name\",\"bv\":false,"
                                                "\"t\":0,\"u\":\"units\","
                                                "\"ut\":0}",
                                desc->length ),
                        ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE(
    utest__xi_senml_json_serialize_entry__valid_data_coln__serialized_value_units_in_the_buffer,
    {
        xi_data_desc_t* desc = xi_make_empty_desc_alloc( 32 );

        xi_state_t ret_state = XI_STATE_OK;

        xi_senml_entry_t value = {
            "name", "units", {{.boolean_value = 0}, XI_SENML_VALUE_TYPE_BOOLEAN},
            0,      0,       {1, 1, 1, 1, 1},
            0};

        ret_state = xi_senml_json_serialize_entry( desc, &value, 1 );

        // printf( "%s", desc->data_ptr );

        tt_want_int_op( ret_state, ==, XI_STATE_OK );
        tt_want_int_op( memcmp( desc->data_ptr, ",{\"n\":\"name\",\"bv\":false,"
                                                "\"t\":0,\"u\":\"units\","
                                                "\"ut\":0}",
                                desc->length ),
                        ==, 0 );

        xi_free_desc( &desc );
    } )

XI_TT_TESTCASE( utest__xi_senml_json_serialize__valid_data__create_buffer_with_json_repr,
                {
                    xi_senml_t* structure = 0;
                    xi_state_t state      = XI_STATE_OK;

                    XI_UNUSED( state );

                    XI_CREATE_SENML_STRUCT(
                        state, structure,
                        XI_SENML_BASE_NAME( "http://base.time.com/the/best/base/time/" ),
                        XI_SENML_BASE_UNITS( "V" ), XI_SENML_BASE_TIME( 23 ) );

                    XI_ADD_SENML_ENTRY(
                        state, structure,
                        XI_SENML_ENTRY_NAME( "http://test.of.name/named_measure" ),
                        XI_SENML_ENTRY_FLOAT_VALUE( 22.02 ) );

                    XI_ADD_SENML_ENTRY( state, structure, XI_SENML_ENTRY_TIME( -120 ),
                                        XI_SENML_ENTRY_BOOLEAN_VALUE( 0 ) );

                    XI_ADD_SENML_ENTRY( state, structure, XI_SENML_ENTRY_TIME( 123 ),
                                        XI_SENML_ENTRY_STRING_VALUE( "Hakuna Matata!" ) );

                    uint8_t* buff = NULL;
                    uint32_t size = 0;

                    state = xi_senml_serialize( structure, &buff, &size );

                    tt_want_int_op( memcmp( xi_senml_cmp, buff,
                                            XI_MIN( sizeof( xi_senml_cmp ) - 1, size ) ),
                                    ==, 0 );

                    xi_senml_destroy( &structure );
                    xi_senml_free_buffer( &buff );

                    return;
                } )

XI_TT_TESTGROUP_END

#pragma GCC diagnostic pop

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
