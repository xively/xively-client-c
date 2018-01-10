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
#include "xi_macros.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
const char* timeseries_topic  = "test-topic";
const int max_csv_string_size = 1024;

#endif

XI_TT_TESTGROUP_BEGIN( utest_publish )

XI_TT_TESTCASE_WITH_SETUP( utest__xi_publish__formatted_timeseries__no_correct_values,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               xi_state_t state       = XI_STATE_OK;
                               uint32_t time_value    = 12345;
                               char* category         = NULL;
                               char* string_value     = NULL;
                               float* ptr_float_value = NULL;

                               /* Call xi_connect_with_callback after creating a context
                                */
                               xi_context_handle_t xi_context = XI_INVALID_CONTEXT_HANDLE;
                               xi_context                     = xi_create_context();
                               tt_assert( XI_INVALID_CONTEXT_HANDLE < xi_context );

                               state = xi_publish_formatted_timeseries(
                                   xi_context, timeseries_topic, &time_value, category,
                                   string_value, ptr_float_value,
                                   XI_MQTT_QOS_AT_LEAST_ONCE, NULL /* CALLBACK */
                                   ,
                                   NULL /* USER DATA */ );


                               tt_int_op( state, ==, XI_INVALID_PARAMETER );

                           end:
                               xi_delete_context( xi_context );
                           } )


XI_TT_TESTCASE_WITH_SETUP( utest__xi_publish__formatted_timeseries__category_too_long,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               xi_state_t state    = XI_STATE_OK;
                               char* category      = NULL;
                               char* string_value  = "string_value";
                               float float_value   = 123.0f;
                               uint32_t time_value = 12345;

                               /* Call xi_connect_with_callback after creating a context
                                */
                               xi_context_handle_t xi_context = XI_INVALID_CONTEXT_HANDLE;
                               xi_context                     = xi_create_context();
                               tt_assert( XI_INVALID_CONTEXT_HANDLE < xi_context );

                               XI_ALLOC_BUFFER_AT( char, category,
                                                   max_csv_string_size + 1, state );
                               memset( category, 'a', max_csv_string_size + 1 );
                               category[max_csv_string_size] = '\0';

                               state = xi_publish_formatted_timeseries(
                                   xi_context, timeseries_topic, &time_value, category,
                                   string_value, &float_value, XI_MQTT_QOS_AT_LEAST_ONCE,
                                   NULL /* CALLBACK */
                                   ,
                                   NULL /* USER DATA */ );

                               tt_int_op( state, ==, XI_SERIALIZATION_ERROR );

                           err_handling:
                           end:
                               XI_SAFE_FREE( category );
                               xi_delete_context( xi_context );
                           } )

XI_TT_TESTCASE_WITH_SETUP( utest__xi_publish__formatted_timeseries__string_value_too_long,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               xi_state_t state    = XI_STATE_OK;
                               char* category      = NULL;
                               char* string_value  = NULL;
                               float float_value   = 123.0f;
                               uint32_t time_value = 12345;

                               /* Call xi_connect_with_callback after creating a context
                                */
                               xi_context_handle_t xi_context = XI_INVALID_CONTEXT_HANDLE;
                               xi_context                     = xi_create_context();
                               tt_assert( XI_INVALID_CONTEXT_HANDLE < xi_context );

                               XI_ALLOC_BUFFER_AT( char, string_value,
                                                   max_csv_string_size + 1, state );
                               memset( string_value, 'a', max_csv_string_size + 1 );
                               string_value[max_csv_string_size] = '\0';

                               state = xi_publish_formatted_timeseries(
                                   xi_context, timeseries_topic, &time_value, category,
                                   string_value, &float_value, XI_MQTT_QOS_AT_LEAST_ONCE,
                                   NULL /* CALLBACK */
                                   ,
                                   NULL /* USER DATA */ );

                               tt_int_op( state, ==, XI_SERIALIZATION_ERROR );

                           err_handling:
                           end:
                               XI_SAFE_FREE( string_value );
                               xi_delete_context( xi_context );
                           } )


XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_publish__formatted_timeseries__string_value_contains_comma,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_state_t state    = XI_STATE_OK;
        char* category      = NULL;
        char* string_value  = "string,value";
        float float_value   = 123.0f;
        uint32_t time_value = 12345;

        /* Call xi_connect_with_callback after creating a context */
        xi_context_handle_t xi_context = XI_INVALID_CONTEXT_HANDLE;
        xi_context                     = xi_create_context();
        tt_assert( XI_INVALID_CONTEXT_HANDLE < xi_context );

        state = xi_publish_formatted_timeseries(
            xi_context, timeseries_topic, &time_value, category, string_value,
            &float_value, XI_MQTT_QOS_AT_LEAST_ONCE, NULL /* CALLBACK */
            ,
            NULL /* USER DATA */ );

        tt_int_op( state, ==, XI_SERIALIZATION_ERROR );

    end:
        xi_delete_context( xi_context );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_publish__formatted_timeseries__string_value_contains_newline,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_state_t state    = XI_STATE_OK;
        char* category      = NULL;
        char* string_value  = "stringvalue\n";
        float float_value   = 123.0f;
        uint32_t time_value = 12345;

        /* Call xi_connect_with_callback after creating a context */
        xi_context_handle_t xi_context = XI_INVALID_CONTEXT_HANDLE;
        xi_context                     = xi_create_context();
        tt_assert( XI_INVALID_CONTEXT_HANDLE < xi_context );

        state = xi_publish_formatted_timeseries(
            xi_context, timeseries_topic, &time_value, category, string_value,
            &float_value, XI_MQTT_QOS_AT_LEAST_ONCE, NULL /* CALLBACK */
            ,
            NULL /* USER DATA */ );

        tt_int_op( state, ==, XI_SERIALIZATION_ERROR );

    end:
        xi_delete_context( xi_context );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_publish__formatted_timeseries__string_value_contains_carriage_return,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_state_t state    = XI_STATE_OK;
        char* category      = NULL;
        char* string_value  = "stringvalue\r";
        float float_value   = 123.0f;
        uint32_t time_value = 12345;

        /* Call xi_connect_with_callback after creating a context */
        xi_context_handle_t xi_context = XI_INVALID_CONTEXT_HANDLE;
        xi_context                     = xi_create_context();
        tt_assert( XI_INVALID_CONTEXT_HANDLE < xi_context );

        state = xi_publish_formatted_timeseries(
            xi_context, timeseries_topic, &time_value, category, string_value,
            &float_value, XI_MQTT_QOS_AT_LEAST_ONCE, NULL /* CALLBACK */
            ,
            NULL /* USER DATA */ );

        tt_int_op( state, ==, XI_SERIALIZATION_ERROR );

    end:
        xi_delete_context( xi_context );
    } )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
