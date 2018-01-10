/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"
#include "xi_utest_basic_testcase_frame.h"

#include "xively_senml.h"
#include "xi_macros.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

XI_TT_TESTGROUP_BEGIN( utest_senml )

XI_TT_TESTCASE( utest_xi_senml_create__empty_data__create_empty_structure, {
    xi_senml_t* structure = 0;
    xi_state_t state      = XI_STATE_OK;

    XI_CREATE_SENML_EMPTY_STRUCT( state, structure );

    tt_want_ptr_op( structure, !=, 0 );
    tt_want_ptr_op( structure->base_name, ==, 0 );
    tt_want_int_op( structure->base_time, ==, 0 );
    tt_want_ptr_op( structure->base_units, ==, 0 );
    tt_want_ptr_op( structure->entries_list, ==, 0 );

    tt_want_int_op( structure->set.base_time_set, ==, 0 );
    tt_want_int_op( structure->set.base_name_set, ==, 0 );
    tt_want_int_op( structure->set.base_units_set, ==, 0 );

    tt_want_int_op( state, ==, XI_STATE_OK );

    xi_senml_destroy( &structure );

    return;

    xi_senml_destroy( &structure );
    tt_abort_printf( ( "this test must not fail!" ) );
end:
    return;
} )

XI_TT_TESTCASE( utest_xi_senml_create__valid_data__create_structure_with_base_name, {
    xi_senml_t* structure  = 0;
    xi_state_t state       = XI_STATE_OK;
    const char test_name[] = "http://test.values.com/test_measures/";

    XI_CREATE_SENML_STRUCT( state, structure, XI_SENML_BASE_NAME( ( char* )test_name ) );
    tt_want_ptr_op( structure, !=, 0 );
    tt_want_ptr_op( structure->base_name, !=, 0 );
    tt_want_int_op( memcmp( structure->base_name, test_name, sizeof( test_name ) ), ==,
                    0 );
    tt_want_int_op( structure->base_time, ==, 0 );
    tt_want_ptr_op( structure->base_units, ==, 0 );
    tt_want_ptr_op( structure->entries_list, ==, 0 );

    tt_want_int_op( structure->set.base_time_set, ==, 0 );
    tt_want_int_op( structure->set.base_name_set, ==, 1 );
    tt_want_int_op( structure->set.base_units_set, ==, 0 );

    tt_want_int_op( state, ==, XI_STATE_OK );

    xi_senml_destroy( &structure );
    return;

    xi_senml_destroy( &structure );
    tt_abort_printf( ( "this test must not fail!" ) );

end:
    return;
} )

XI_TT_TESTCASE( utest_xi_senml_create__valid_data__create_structure_with_base_units, {
    xi_senml_t* structure        = 0;
    xi_state_t state             = XI_STATE_OK;
    const char test_units_name[] = "Volts";

    XI_CREATE_SENML_STRUCT( state, structure,
                            XI_SENML_BASE_UNITS( ( char* )test_units_name ) );

    tt_want_ptr_op( structure, !=, 0 );
    tt_want_ptr_op( structure->base_name, ==, 0 );
    tt_want_int_op( structure->base_time, ==, 0 );
    tt_want_int_op(
        memcmp( structure->base_units, test_units_name, sizeof( test_units_name ) ), ==,
        0 );
    tt_want_ptr_op( structure->entries_list, ==, 0 );

    tt_want_int_op( structure->set.base_time_set, ==, 0 );
    tt_want_int_op( structure->set.base_name_set, ==, 0 );
    tt_want_int_op( structure->set.base_units_set, ==, 1 );

    tt_want_int_op( state, ==, XI_STATE_OK );

    xi_senml_destroy( &structure );
    return;

    xi_senml_destroy( &structure );
    tt_abort_printf( ( "this test must not fail!" ) );
end:
    return;
} )

XI_TT_TESTCASE( utest_xi_senml_create__valid_data__create_structure_with_time, {
    xi_senml_t* structure = 0;
    xi_state_t state      = XI_STATE_OK;
    uint32_t base_time    = 1234;

    XI_CREATE_SENML_STRUCT( state, structure, XI_SENML_BASE_TIME( base_time ) );

    tt_want_ptr_op( structure, !=, 0 );
    tt_want_ptr_op( structure->base_name, ==, 0 );
    tt_want_int_op( structure->base_time, ==, base_time );
    tt_want_int_op( structure->base_units, ==, 0 );
    tt_want_ptr_op( structure->entries_list, ==, 0 );

    tt_want_int_op( structure->set.base_time_set, ==, 1 );
    tt_want_int_op( structure->set.base_name_set, ==, 0 );
    tt_want_int_op( structure->set.base_units_set, ==, 0 );

    tt_want_int_op( state, ==, XI_STATE_OK );

    xi_senml_destroy( &structure );
    return;

    xi_senml_destroy( &structure );
    tt_abort_printf( ( "this test must not fail!" ) );
end:
    return;
} )

XI_TT_TESTCASE( utest_xi_senml_create__valid_data_create_structure_with_all_fields, {
    xi_senml_t* structure = 0;
    xi_state_t state      = XI_STATE_OK;

    const char test_name[]       = "http://test.values.com/test_measures/";
    const char test_units_name[] = "Volts";
    uint32_t base_time           = 1234;

    XI_CREATE_SENML_STRUCT( state, structure, XI_SENML_BASE_TIME( base_time ),
                            XI_SENML_BASE_NAME( ( char* )test_name ),
                            XI_SENML_BASE_UNITS( ( char* )test_units_name ) );

    tt_want_ptr_op( structure, !=, 0 );
    tt_want_int_op( structure->base_time, ==, base_time );
    tt_want_int_op(
        memcmp( structure->base_units, test_units_name, sizeof( test_units_name ) ), ==,
        0 );
    tt_want_int_op( memcmp( structure->base_name, test_name, sizeof( test_name ) ), ==,
                    0 );
    tt_want_ptr_op( structure->entries_list, ==, 0 );
    tt_want_int_op( state, ==, XI_STATE_OK );

    xi_senml_destroy( &structure );
    return;

    xi_senml_destroy( &structure );
    tt_abort_printf( ( "this test must not fail!" ) );
end:
    return;
} )

XI_TT_TESTCASE( utest_xi_senml_create_empty__invalid_data__set_state_to_invalid_parameter,
                {
                    xi_senml_t* structure = ( xi_senml_t* )( intptr_t )0x1FF11F1F;
                    xi_state_t state      = XI_STATE_OK;

                    XI_CREATE_SENML_EMPTY_STRUCT( state, structure );

                    tt_want_int_op( state, ==, XI_INVALID_PARAMETER );
                    return;
                } )

XI_TT_TESTCASE(
    utest_xi_senml_create_with_base_name_invalid_data__set_state_to_invalid_parameter, {
        xi_senml_t* structure = 0;
        xi_state_t state      = XI_STATE_OK;

        XI_CREATE_SENML_STRUCT( state, structure, XI_SENML_BASE_NAME( 0 ) );

        xi_senml_destroy( &structure );
        tt_want_int_op( state, ==, XI_INVALID_PARAMETER );

        return;
    } )

XI_TT_TESTCASE(
    utest_xi_senml_create_with_base_units_invalid_data_set_state_to_invalid_parameter, {
        xi_senml_t* structure = 0;
        xi_state_t state      = XI_STATE_OK;

        XI_CREATE_SENML_STRUCT( state, structure, XI_SENML_BASE_UNITS( 0 ) );

        xi_senml_destroy( &structure );
        tt_want_int_op( state, ==, XI_INVALID_PARAMETER );

        return;
    } )

XI_TT_TESTCASE(
    utest_xi_senml_entry_create_float_value__valid_data__create_empty_structure_with_one_value_entry,
    {
        xi_senml_t* structure = 0;
        xi_state_t state      = XI_STATE_OK;

        XI_UNUSED( state );

        XI_CREATE_SENML_EMPTY_STRUCT( state, structure );
        XI_ADD_SENML_ENTRY( state, structure, XI_SENML_ENTRY_FLOAT_VALUE( 3.2 ) );

        tt_want_ptr_op( structure->entries_list, !=, 0 );
        xi_senml_destroy( &structure );

        return;

        xi_senml_destroy( &structure );
        tt_abort_printf( ( "this test must not fail!" ) );
    end:
        return;
    } )

#if 0
XI_TT_TESTCASE( utest_xi_senml_entry_create_empty__valid_data__set_status_to_invalid_parameter,
{
    xi_senml_t* structure = 0;
    xi_state_t state = XI_STATE_OK;

    XI_CREATE_SENML_EMPTY_STRUCT( state, structure );
    XI_ADD_SENML_ENTRY( state, structure );

    xi_senml_destroy( &structure );
    tt_abort_printf( ( "this test must fail!" ) );

    return;

    tt_want_int_op( state, ==, XI_INVALID_PARAMETER );
    xi_senml_destroy( &structure );
end:
    return;
} )
#endif

XI_TT_TESTCASE(
    utest_xi_senml_entry_create_string_value__valid_data_create_empty_structure_with_string_value_entry,
    {
        xi_senml_t* structure   = 0;
        xi_state_t state        = XI_STATE_OK;
        const char test_value[] = "simple test value stored as a string";

        XI_UNUSED( state );

        XI_CREATE_SENML_EMPTY_STRUCT( state, structure );
        XI_ADD_SENML_ENTRY( state, structure,
                            XI_SENML_ENTRY_STRING_VALUE( ( char* )test_value ) );

        tt_want_ptr_op( structure->entries_list, !=, 0 );
        tt_want_int_op( structure->entries_list->set.value_set, ==, 1 );
        tt_want_int_op( memcmp( structure->entries_list->value_cnt.value.string_value,
                                test_value, sizeof( test_value ) ),
                        ==, 0 );

        xi_senml_destroy( &structure );
        return;

        xi_senml_destroy( &structure );
        tt_abort_printf( ( "this test must not fail!" ) );
    end:
        return;
    } )

XI_TT_TESTCASE(
    utest_xi_senml_entry_create_boolean_value__valid_data_create_empty_structure_with_boolean_value_entry,
    {
        xi_senml_t* structure = 0;
        xi_state_t state      = XI_STATE_OK;
        uint8_t value         = 1;

        XI_UNUSED( state );

        XI_CREATE_SENML_EMPTY_STRUCT( state, structure );
        XI_ADD_SENML_ENTRY( state, structure, XI_SENML_ENTRY_BOOLEAN_VALUE( value ) );

        tt_want_ptr_op( structure->entries_list, !=, 0 );
        tt_want_int_op( structure->entries_list->set.value_set, ==, 1 );
        tt_want_int_op( structure->entries_list->value_cnt.value.boolean_value, ==,
                        value );

        xi_senml_destroy( &structure );
        return;

        xi_senml_destroy( &structure );
        tt_abort_printf( ( "this test must not fail!" ) );
    end:
        return;
    } )

XI_TT_TESTCASE(
    utest_xi_senml_entry_create_time__valid_data_create_empty_structure_with_time_entry, {
        xi_senml_t* structure = 0;
        xi_state_t state      = XI_STATE_OK;
        uint32_t time         = 1123;

        XI_UNUSED( state );

        XI_CREATE_SENML_EMPTY_STRUCT( state, structure );
        XI_ADD_SENML_ENTRY( state, structure, XI_SENML_ENTRY_FLOAT_VALUE( 1.23 ),
                            XI_SENML_ENTRY_TIME( time ) );

        tt_want_ptr_op( structure->entries_list, !=, 0 );
        tt_want_int_op( structure->entries_list->set.time_set, ==, 1 );
        tt_want_int_op( structure->entries_list->time, ==, time );

        xi_senml_destroy( &structure );
        return;

        xi_senml_destroy( &structure );
        tt_abort_printf( ( "this test must not fail!" ) );
    end:
        return;
    } )

XI_TT_TESTCASE(
    utest_xi_senml_entry_create_updata_time__valid_data_create_empty_structure_with_update_time_entry,
    {
        xi_senml_t* structure = 0;
        xi_state_t state      = XI_STATE_OK;
        uint32_t update_time  = 1123;

        XI_UNUSED( state );

        XI_CREATE_SENML_EMPTY_STRUCT( state, structure );
        XI_ADD_SENML_ENTRY( state, structure, XI_SENML_ENTRY_FLOAT_VALUE( 1.23 ),
                            XI_SENML_ENTRY_UPDATE_TIME( update_time ) );

        tt_want_ptr_op( structure->entries_list, !=, 0 );
        tt_want_int_op( structure->entries_list->set.update_time_set, ==, 1 );
        tt_want_int_op( structure->entries_list->update_time, ==, update_time );

        xi_senml_destroy( &structure );
        return;

        xi_senml_destroy( &structure );
        tt_abort_printf( ( "this test must not fail!" ) );
    end:
        return;
    } )


XI_TT_TESTCASE_WITH_SETUP(
    utest_xi_senml_entry_create__update_name_field_2x__no_memory_leak_expected,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_senml_t* structure = 0;
        xi_state_t state      = XI_STATE_OK;

        XI_UNUSED( state );

        XI_CREATE_SENML_EMPTY_STRUCT( state, structure );
        XI_ADD_SENML_ENTRY( state, structure, XI_SENML_ENTRY_NAME( "tiger" ),
                            XI_SENML_ENTRY_NAME( "skunk" ) );

        tt_want_ptr_op( structure->entries_list, !=, NULL );
        tt_want_int_op( structure->entries_list->set.name_set, ==, 1 );
        tt_want_str_op( structure->entries_list->name, ==, "skunk" );

        xi_senml_destroy( &structure );

        return;
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest_xi_senml_entry_create__update_units_field_3x__no_memory_leak_expected,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_senml_t* structure = 0;
        xi_state_t state      = XI_STATE_OK;

        XI_UNUSED( state );

        XI_CREATE_SENML_EMPTY_STRUCT( state, structure );
        XI_ADD_SENML_ENTRY( state, structure, XI_SENML_ENTRY_UNITS( "celsius" ),
                            XI_SENML_ENTRY_UNITS( "elephant ???" ),
                            XI_SENML_ENTRY_UNITS( "fahrenheit" ) );

        tt_want_ptr_op( structure->entries_list, !=, NULL );
        tt_want_int_op( structure->entries_list->set.units_set, ==, 1 );
        tt_want_str_op( structure->entries_list->units, ==, "fahrenheit" );

        xi_senml_destroy( &structure );

        return;
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest_xi_senml_entry_create__update_string_value_3x__no_memory_leak_expected,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_senml_t* structure = 0;
        xi_state_t state      = XI_STATE_OK;

        XI_UNUSED( state );

        XI_CREATE_SENML_EMPTY_STRUCT( state, structure );
        XI_ADD_SENML_ENTRY( state, structure, XI_SENML_ENTRY_STRING_VALUE( "flow" ),
                            XI_SENML_ENTRY_STRING_VALUE( "rope" ),
                            XI_SENML_ENTRY_STRING_VALUE( "cukroscsibecombcsont" ) );

        tt_want_ptr_op( structure->entries_list, !=, NULL );
        tt_want_int_op( structure->entries_list->set.value_set, ==, 1 );
        tt_want_str_op( structure->entries_list->value_cnt.value.string_value, ==,
                        "cukroscsibecombcsont" );

        xi_senml_destroy( &structure );

        return;
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest_xi_senml_entry_create__update_string_time_string__no_memory_leak_expected,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_senml_t* structure = 0;
        xi_state_t state      = XI_STATE_OK;

        XI_UNUSED( state );

        XI_CREATE_SENML_EMPTY_STRUCT( state, structure );
        XI_ADD_SENML_ENTRY( state, structure, XI_SENML_ENTRY_STRING_VALUE( "flow" ),
                            XI_SENML_ENTRY_TIME( 111 ),
                            XI_SENML_ENTRY_STRING_VALUE( "rope" ) );

        tt_want_ptr_op( structure->entries_list, !=, NULL );
        tt_want_int_op( structure->entries_list->set.value_set, ==, 1 );
        tt_want_str_op( structure->entries_list->value_cnt.value.string_value, ==,
                        "rope" );

        xi_senml_destroy( &structure );

        return;
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest_xi_senml_entry_create__update_string_and_units_value_more_times__no_memory_leak_expected,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_senml_t* structure = 0;
        xi_state_t state      = XI_STATE_OK;

        XI_UNUSED( state );

        XI_CREATE_SENML_EMPTY_STRUCT( state, structure );
        XI_ADD_SENML_ENTRY( state, structure, XI_SENML_ENTRY_STRING_VALUE( "flow" ),
                            XI_SENML_ENTRY_STRING_VALUE( "rope" ),
                            XI_SENML_ENTRY_STRING_VALUE( "cukroscsibecombcsont" ),
                            XI_SENML_ENTRY_UNITS( "elephant ???" ),
                            XI_SENML_ENTRY_UNITS( "fahrenheit" ) );

        tt_want_ptr_op( structure->entries_list, !=, NULL );
        tt_want_int_op( structure->entries_list->set.value_set, ==, 1 );
        tt_want_str_op( structure->entries_list->value_cnt.value.string_value, ==,
                        "cukroscsibecombcsont" );
        tt_want_int_op( structure->entries_list->set.units_set, ==, 1 );
        tt_want_str_op( structure->entries_list->units, ==, "fahrenheit" );

        xi_senml_destroy( &structure );

        return;
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest_xi_senml_entry_create__update_base_name_2x__no_memory_leak_expected,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_senml_t* structure = 0;
        xi_state_t state      = XI_STATE_OK;

        XI_UNUSED( state );

        XI_CREATE_SENML_STRUCT( state, structure, XI_SENML_BASE_NAME( "toothpaste" ),
                                XI_SENML_BASE_UNITS( "zzz" ),
                                XI_SENML_BASE_NAME( "shoelaces" ) );

        tt_want_int_op( structure->set.base_name_set, ==, 1 );
        tt_want_str_op( structure->base_name, ==, "shoelaces" );

        xi_senml_destroy( &structure );

        return;
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest_xi_senml_entry_create__update_base_units_2x__no_memory_leak_expected,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_senml_t* structure = 0;
        xi_state_t state      = XI_STATE_OK;

        XI_UNUSED( state );

        XI_CREATE_SENML_STRUCT( state, structure, XI_SENML_BASE_UNITS( "aaa" ),
                                XI_SENML_BASE_NAME( "toothpaste" ),
                                XI_SENML_BASE_UNITS( "zzz" ) );

        tt_want_int_op( structure->set.base_units_set, ==, 1 );
        tt_want_str_op( structure->base_units, ==, "zzz" );

        xi_senml_destroy( &structure );

        return;
    } )

XI_TT_TESTGROUP_END

#pragma GCC diagnostic pop

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
