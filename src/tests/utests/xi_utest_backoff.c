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
#include "xi_helpers.h"
#include "xi_backoff_status_api.h"
#include "xi_backoff_lut_config.h"
#include "xi_globals.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

static const xi_vector_elem_t xi_utest_backoff_lut_test_1[] = {
    XI_VEC_ELEM( XI_VEC_VALUE_UI32( 1 ) )};

static const xi_vector_elem_t xi_utest_decay_lut_test_1[] = {
    XI_VEC_ELEM( XI_VEC_VALUE_UI32( 123 ) )};

static const xi_vector_elem_t xi_utest_backoff_lut_test_2[] = {
    XI_VEC_ELEM( XI_VEC_VALUE_UI32( 2 ) ), XI_VEC_ELEM( XI_VEC_VALUE_UI32( 4 ) ),
    XI_VEC_ELEM( XI_VEC_VALUE_UI32( 8 ) ), XI_VEC_ELEM( XI_VEC_VALUE_UI32( 16 ) )};

static const xi_vector_elem_t xi_utest_decay_lut_test_2[] = {
    XI_VEC_ELEM( XI_VEC_VALUE_UI32( 2 ) ), XI_VEC_ELEM( XI_VEC_VALUE_UI32( 4 ) ),
    XI_VEC_ELEM( XI_VEC_VALUE_UI32( 8 ) ), XI_VEC_ELEM( XI_VEC_VALUE_UI32( 30 ) )};

typedef struct xi_utest_backoff_data_test_case_s
{
    const xi_vector_elem_t* backoff_data;
    const xi_vector_elem_t* decay_data;
    size_t data_len;
} xi_utest_backoff_data_test_case_t;

static const xi_utest_backoff_data_test_case_t xi_utest_backoff_test_cases[] = {
    {XI_BACKOFF_LUT, XI_DECAY_LUT, XI_ARRAYSIZE( XI_BACKOFF_LUT )},
    {xi_utest_backoff_lut_test_1, xi_utest_decay_lut_test_1,
     XI_ARRAYSIZE( xi_utest_backoff_lut_test_1 )},
    {xi_utest_backoff_lut_test_2, xi_utest_decay_lut_test_2,
     XI_ARRAYSIZE( xi_utest_backoff_lut_test_2 )}};

void xi__utest__reset_backoff_penalty()
{
    xi_globals.backoff_status.backoff_lut_i = 0;
    xi_cancel_backoff_event();
}

#endif

XI_TT_TESTGROUP_BEGIN( utest_backoff )

XI_TT_TESTCASE( utest__backoff_and_decay_lut_sizes__no_data__must_be_equal, {
    tt_int_op( XI_ARRAYSIZE( XI_BACKOFF_LUT ), ==, XI_ARRAYSIZE( XI_DECAY_LUT ) );

    tt_int_op( XI_ARRAYSIZE( xi_utest_backoff_lut_test_1 ), ==,
               XI_ARRAYSIZE( xi_utest_decay_lut_test_1 ) );

    tt_int_op( XI_ARRAYSIZE( xi_utest_backoff_lut_test_2 ), ==,
               XI_ARRAYSIZE( xi_utest_decay_lut_test_2 ) );
end:;
} )

XI_TT_TESTCASE(
    utest__xi_backoff_configure_using_data__valid_data__must_contain_proper_data, {
        size_t itests = XI_ARRAYSIZE( xi_utest_backoff_test_cases );

        size_t i = 0;
        for ( ; i < itests; ++i )
        {
            const xi_utest_backoff_data_test_case_t* curr_test_case =
                ( xi_utest_backoff_data_test_case_t* )&xi_utest_backoff_test_cases[i];

            xi_backoff_configure_using_data(
                ( xi_vector_elem_t* )curr_test_case->backoff_data,
                ( xi_vector_elem_t* )curr_test_case->decay_data, curr_test_case->data_len,
                XI_MEMORY_TYPE_UNMANAGED );

            tt_ptr_op( xi_globals.backoff_status.backoff_lut->array, !=, NULL );
            tt_ptr_op( xi_globals.backoff_status.backoff_lut->array, ==,
                       curr_test_case->backoff_data );

            tt_ptr_op( xi_globals.backoff_status.decay_lut->array, !=, NULL );
            tt_ptr_op( xi_globals.backoff_status.decay_lut->array, ==,
                       curr_test_case->decay_data );
        }

    end:
        xi_backoff_release();
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_inc_backoff_penalty__no_data__use_proper_progression,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_context_handle_t xi_context_handle = xi_create_context();
        if ( XI_INVALID_CONTEXT_HANDLE >= xi_context_handle )
        {
            tt_fail_msg( "Failed to create default context!" );
            return;
        }

        size_t itests = XI_ARRAYSIZE( xi_utest_backoff_test_cases );

        size_t i = 0;
        for ( ; i < itests; ++i )
        {
            const xi_utest_backoff_data_test_case_t* curr_test_case =
                ( xi_utest_backoff_data_test_case_t* )&xi_utest_backoff_test_cases[i];

            xi_backoff_configure_using_data(
                ( xi_vector_elem_t* )curr_test_case->backoff_data,
                ( xi_vector_elem_t* )curr_test_case->decay_data, curr_test_case->data_len,
                XI_MEMORY_TYPE_UNMANAGED );

            tt_want_int_op( xi_globals.backoff_status.next_update.ptr_to_position, ==,
                            0 );
            xi_globals.backoff_status.backoff_lut_i = 0;

            int i = 0;
            for ( ; i < xi_globals.backoff_status.backoff_lut->elem_no - 1; ++i )
            {
                uint32_t prev_backoff_lut_i = xi_globals.backoff_status.backoff_lut_i;
                xi_inc_backoff_penalty();

                if ( curr_test_case->data_len > 1 )
                {
                    tt_want_int_op( xi_globals.backoff_status.backoff_lut_i, >, 0 );
                }

                tt_want_int_op( prev_backoff_lut_i, <,
                                xi_globals.backoff_status.backoff_lut_i );
            }

            uint32_t prev_backoff_lut_i = xi_globals.backoff_status.backoff_lut_i;

            xi_inc_backoff_penalty();

            if ( curr_test_case->data_len > 1 )
            {
                tt_want_int_op( xi_globals.backoff_status.backoff_lut_i, >, 0 );
            }

            tt_want_int_op( prev_backoff_lut_i, ==,
                            xi_globals.backoff_status.backoff_lut_i );

            xi_globals.backoff_status.next_update.ptr_to_position = 0;
        }

        xi_delete_context( xi_context_handle );
        xi_backoff_release();
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_dec_backoff_penalty__no_data__use_proper_equation,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_context_handle_t xi_context_handle = xi_create_context();
        if ( XI_INVALID_CONTEXT_HANDLE >= xi_context_handle )
        {
            tt_fail_msg( "Failed to create default context!" );
            return;
        }

        size_t itests = XI_ARRAYSIZE( xi_utest_backoff_test_cases );

        size_t i = 0;
        for ( ; i < itests; ++i )
        {
            const xi_utest_backoff_data_test_case_t* curr_test_case =
                ( xi_utest_backoff_data_test_case_t* )&xi_utest_backoff_test_cases[i];

            xi_backoff_configure_using_data(
                ( xi_vector_elem_t* )curr_test_case->backoff_data,
                ( xi_vector_elem_t* )curr_test_case->decay_data, curr_test_case->data_len,
                XI_MEMORY_TYPE_UNMANAGED );

            size_t last_index = XI_ARRAYSIZE( xi_utest_backoff_lut_test_2 ) - 1;
            xi_globals.backoff_status.backoff_lut_i = last_index;

            int backoff_i = last_index;

            for ( ; backoff_i > 0; --backoff_i )
            {
                uint32_t prev_backoff_i = xi_globals.backoff_status.backoff_lut_i;

                xi_dec_backoff_penalty();

                tt_want_int_op( xi_globals.backoff_status.backoff_lut_i, ==,
                                prev_backoff_i - 1 );
            }

            tt_want_int_op( xi_globals.backoff_status.backoff_lut_i, ==, 0 );
        }

        xi_delete_context( xi_context_handle );
        xi_backoff_release();
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_get_backoff_penalty__check_return_value_random_range,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_context_handle_t xi_context_handle = xi_create_context();
        if ( XI_INVALID_CONTEXT_HANDLE >= xi_context_handle )
        {
            tt_fail_msg( "Failed to create default context!" );
            return;
        }

        size_t itests = XI_ARRAYSIZE( xi_utest_backoff_test_cases );

        size_t i = 0;
        for ( ; i < itests; ++i )
        {
            const xi_utest_backoff_data_test_case_t* curr_test_case =
                ( xi_utest_backoff_data_test_case_t* )&xi_utest_backoff_test_cases[i];

            xi_backoff_configure_using_data(
                ( xi_vector_elem_t* )curr_test_case->backoff_data,
                ( xi_vector_elem_t* )curr_test_case->decay_data, curr_test_case->data_len,
                XI_MEMORY_TYPE_UNMANAGED );

            size_t j = 0;
            for ( ; j < curr_test_case->data_len; ++j )
            {
                xi_globals.backoff_status.backoff_lut_i = j;

                const xi_backoff_lut_index_t lower_index =
                    XI_MAX( xi_globals.backoff_status.backoff_lut_i - 1, 0 );

                const int32_t orig_value =
                    xi_globals.backoff_status.backoff_lut
                        ->array[xi_globals.backoff_status.backoff_lut_i]
                        .selector_t.ui32_value;

                const uint32_t rand_range =
                    xi_globals.backoff_status.backoff_lut->array[lower_index]
                        .selector_t.ui32_value;

                const int32_t half_range = XI_MAX( rand_range / 2, 1 );

                int jj = 0;
                for ( ; jj < 1000; ++jj )
                {
                    const int32_t penalty = xi_get_backoff_penalty();

                    tt_int_op( penalty, <=, orig_value + half_range );
                    tt_int_op( penalty, >=, orig_value - half_range );
                }
            }
        }

    end:
        xi_delete_context( xi_context_handle );
        xi_backoff_release();
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_cancel_backoff_event__no_data__backoff_status_clean,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_context_handle_t xi_context_handle = xi_create_context();
        if ( XI_INVALID_CONTEXT_HANDLE >= xi_context_handle )
        {
            tt_fail_msg( "Failed to create default context!" );
            return;
        }

        size_t itests = XI_ARRAYSIZE( xi_utest_backoff_test_cases );

        size_t i = 0;
        for ( ; i < itests; ++i )
        {
            const xi_utest_backoff_data_test_case_t* curr_test_case =
                ( xi_utest_backoff_data_test_case_t* )&xi_utest_backoff_test_cases[i];

            xi_backoff_configure_using_data(
                ( xi_vector_elem_t* )curr_test_case->backoff_data,
                ( xi_vector_elem_t* )curr_test_case->decay_data, curr_test_case->data_len,
                XI_MEMORY_TYPE_UNMANAGED );

            xi_globals.backoff_status.backoff_lut_i = 5;

            xi_inc_backoff_penalty();

            xi_cancel_backoff_event();

            tt_want_ptr_op( xi_globals.backoff_status.next_update.ptr_to_position, ==,
                            0 );
        }

        xi_delete_context( xi_context_handle );
        xi_backoff_release();
    } )

XI_TT_TESTCASE( utest__xi_backoff_classify_state__xi_state_for_none__backoff_class_none, {
    xi_state_t states[] = {XI_STATE_OK, XI_STATE_WRITTEN};

    size_t i = 0;
    for ( ; i < XI_ARRAYSIZE( states ); ++i )
    {
        tt_want_int_op( xi_backoff_classify_state( states[i] ), ==,
                        XI_BACKOFF_CLASS_NONE );
    }
} )

XI_TT_TESTCASE(
    utest__xi_backoff_classify_state__xi_state_for_terminal__backoff_class_terminal, {
        xi_state_t states[] = {XI_CONNECTION_RESET_BY_PEER_ERROR,
                               XI_MQTT_UNACCEPTABLE_PROTOCOL_VERSION,
                               XI_MQTT_IDENTIFIER_REJECTED,
                               XI_MQTT_BAD_USERNAME_OR_PASSWORD, XI_MQTT_NOT_AUTHORIZED};

        size_t i = 0;
        for ( ; i < XI_ARRAYSIZE( states ); ++i )
        {
            tt_want_int_op( xi_backoff_classify_state( states[i] ), ==,
                            XI_BACKOFF_CLASS_TERMINAL );
        }
    } )

XI_TT_TESTCASE(
    utest__xi_backoff_classify_state__xi_state_for_recoverable__backoff_class_recoverable,
    {
        xi_state_t states[] = {XI_MQTT_MESSAGE_CLASS_UNKNOWN_ERROR,
                               XI_TLS_CONNECT_ERROR,
                               XI_TLS_INITALIZATION_ERROR,
                               XI_STATE_TIMEOUT,
                               XI_SOCKET_WRITE_ERROR,
                               XI_SOCKET_READ_ERROR};

        unsigned long i = 0;
        for ( ; i < XI_ARRAYSIZE( states ); ++i )
        {
            tt_want_int_op( xi_backoff_classify_state( states[i] ), ==,
                            XI_BACKOFF_CLASS_RECOVERABLE );
        }
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_update_backoff_penalty__xi_state_terminal__increase_penalty,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_context_handle_t xi_context_handle = xi_create_context();
        if ( XI_INVALID_CONTEXT_HANDLE >= xi_context_handle )
        {
            tt_fail_msg( "Failed to create default context!" );
            return;
        }

        size_t itests = XI_ARRAYSIZE( xi_utest_backoff_test_cases );

        size_t i = 0;
        for ( ; i < itests; ++i )
        {
            const xi_utest_backoff_data_test_case_t* curr_test_case =
                ( xi_utest_backoff_data_test_case_t* )&xi_utest_backoff_test_cases[i];

            xi_backoff_configure_using_data(
                ( xi_vector_elem_t* )curr_test_case->backoff_data,
                ( xi_vector_elem_t* )curr_test_case->decay_data, curr_test_case->data_len,
                XI_MEMORY_TYPE_UNMANAGED );

            xi_backoff_lut_index_t curr_index = xi_globals.backoff_status.backoff_lut_i;

            xi_update_backoff_penalty( XI_MQTT_BAD_USERNAME_OR_PASSWORD );

            tt_want_int_op( xi_globals.backoff_status.backoff_class, ==,
                            XI_BACKOFF_CLASS_TERMINAL );

            if ( curr_test_case->data_len > 1 )
            {
                tt_want_int_op( xi_globals.backoff_status.backoff_lut_i, >, curr_index );
            }
            else
            {
                tt_want_int_op( xi_globals.backoff_status.backoff_lut_i, ==, curr_index );
                tt_want_int_op( xi_globals.backoff_status.backoff_lut_i, ==, 0 );
            }

            tt_want_ptr_op( xi_globals.backoff_status.next_update.ptr_to_position, !=,
                            0 );

            xi__utest__reset_backoff_penalty();
        }

        xi_delete_context( xi_context_handle );
        xi_backoff_release();
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_update_backoff_penalty__xi_state_recoverable__increase_penalty,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_context_handle_t xi_context_handle = xi_create_context();
        if ( XI_INVALID_CONTEXT_HANDLE >= xi_context_handle )
        {
            tt_fail_msg( "Failed to create default context!" );
            return;
        }

        size_t itests = XI_ARRAYSIZE( xi_utest_backoff_test_cases );

        size_t i = 0;
        for ( ; i < itests; ++i )
        {
            const xi_utest_backoff_data_test_case_t* curr_test_case =
                ( xi_utest_backoff_data_test_case_t* )&xi_utest_backoff_test_cases[i];

            xi_backoff_configure_using_data(
                ( xi_vector_elem_t* )curr_test_case->backoff_data,
                ( xi_vector_elem_t* )curr_test_case->decay_data, curr_test_case->data_len,
                XI_MEMORY_TYPE_UNMANAGED );

            xi_backoff_lut_index_t curr_index = xi_globals.backoff_status.backoff_lut_i;

            xi_update_backoff_penalty( XI_SOCKET_READ_ERROR );

            tt_want_int_op( xi_globals.backoff_status.backoff_class, ==,
                            XI_BACKOFF_CLASS_RECOVERABLE );

            if ( curr_test_case->data_len > 1 )
            {
                tt_want_int_op( xi_globals.backoff_status.backoff_lut_i, >, curr_index );
            }
            else
            {
                tt_want_int_op( xi_globals.backoff_status.backoff_lut_i, ==, curr_index );
                tt_want_int_op( xi_globals.backoff_status.backoff_lut_i, ==, 0 );
            }

            tt_want_ptr_op( xi_globals.backoff_status.next_update.ptr_to_position, !=,
                            0 );

            xi__utest__reset_backoff_penalty();
        }

        xi_delete_context( xi_context_handle );
        xi_backoff_release();
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_update_backoff_penalty__xi_state_none__none_penalty,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_context_handle_t xi_context_handle = xi_create_context();
        if ( XI_INVALID_CONTEXT_HANDLE >= xi_context_handle )
        {
            tt_fail_msg( "Failed to create default context!" );
            return;
        }

        size_t itests = XI_ARRAYSIZE( xi_utest_backoff_test_cases );

        size_t i = 0;
        for ( ; i < itests; ++i )
        {
            const xi_utest_backoff_data_test_case_t* curr_test_case =
                ( xi_utest_backoff_data_test_case_t* )&xi_utest_backoff_test_cases[i];

            xi_backoff_configure_using_data(
                ( xi_vector_elem_t* )curr_test_case->backoff_data,
                ( xi_vector_elem_t* )curr_test_case->decay_data, curr_test_case->data_len,
                XI_MEMORY_TYPE_UNMANAGED );

            xi_backoff_lut_index_t curr_index = xi_globals.backoff_status.backoff_lut_i;

            xi_update_backoff_penalty( XI_STATE_OK );

            tt_want_int_op( xi_globals.backoff_status.backoff_class, ==,
                            XI_BACKOFF_CLASS_NONE );
            tt_want_int_op( xi_globals.backoff_status.backoff_lut_i, ==, curr_index );
            tt_want_ptr_op( xi_globals.backoff_status.next_update.ptr_to_position, ==,
                            0 );

            xi__utest__reset_backoff_penalty();
        }

        xi_delete_context( xi_context_handle );
        xi_backoff_release();
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_restart_update_time__no_data__update_event_registered,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_context_handle_t xi_context_handle = xi_create_context();
        if ( XI_INVALID_CONTEXT_HANDLE >= xi_context_handle )
        {
            tt_fail_msg( "Failed to create default context!" );
            return;
        }

        xi_evtd_instance_t* event_dispatcher = xi_globals.evtd_instance;

        size_t itests = XI_ARRAYSIZE( xi_utest_backoff_test_cases );

        size_t i = 0;
        for ( ; i < itests; ++i )
        {
            const xi_utest_backoff_data_test_case_t* curr_test_case =
                ( xi_utest_backoff_data_test_case_t* )&xi_utest_backoff_test_cases[i];

            xi_backoff_configure_using_data(
                ( xi_vector_elem_t* )curr_test_case->backoff_data,
                ( xi_vector_elem_t* )curr_test_case->decay_data, curr_test_case->data_len,
                XI_MEMORY_TYPE_UNMANAGED );

            tt_want_ptr_op( xi_globals.backoff_status.next_update.ptr_to_position, ==,
                            0 );

            xi_restart_update_time( event_dispatcher );

            tt_want_ptr_op( xi_globals.backoff_status.next_update.ptr_to_position, !=,
                            0 );

            xi_time_event_handle_t* backoff_handler =
                &xi_globals.backoff_status.next_update;

            xi_restart_update_time( event_dispatcher );

            tt_want_ptr_op( xi_globals.backoff_status.next_update.ptr_to_position, ==,
                            backoff_handler->ptr_to_position );
            tt_want_ptr_op( xi_globals.backoff_status.next_update.ptr_to_position, !=,
                            NULL );

            xi__utest__reset_backoff_penalty();
        }

        xi_delete_context( xi_context_handle );
        xi_backoff_release();
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_update_backoff_penalty_time__no_data__update_update_time_and_no_backoff_penalty,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_context_handle_t xi_context_handle = xi_create_context();
        if ( XI_INVALID_CONTEXT_HANDLE >= xi_context_handle )
        {
            tt_fail_msg( "Failed to create default context!" );
            return;
        }

        xi_evtd_instance_t* event_dispatcher = xi_globals.evtd_instance;
        tt_int_op( xi_globals.backoff_status.backoff_lut_i, ==, 0 );

        size_t itests = XI_ARRAYSIZE( xi_utest_backoff_test_cases );

        size_t i = 0;
        for ( ; i < itests; ++i )
        {
            const xi_utest_backoff_data_test_case_t* curr_test_case =
                ( xi_utest_backoff_data_test_case_t* )&xi_utest_backoff_test_cases[i];

            xi_backoff_configure_using_data(
                ( xi_vector_elem_t* )curr_test_case->backoff_data,
                ( xi_vector_elem_t* )curr_test_case->decay_data, curr_test_case->data_len,
                XI_MEMORY_TYPE_UNMANAGED );

            xi_inc_backoff_penalty();

            xi_globals.backoff_status.backoff_class = XI_BACKOFF_CLASS_TERMINAL;

            xi_vector_index_type_t* backoff_time_event_position =
                xi_globals.backoff_status.next_update.ptr_to_position;

            xi_backoff_lut_index_t curr_index = xi_globals.backoff_status.backoff_lut_i;

            xi_evtd_step( event_dispatcher,
                          event_dispatcher->current_step +
                              xi_globals.backoff_status.decay_lut->array[curr_index]
                                  .selector_t.ui32_value +
                              1 );

            tt_int_op( xi_globals.backoff_status.backoff_lut_i, ==, curr_index );

            if ( curr_test_case->data_len > 1 )
            {
                tt_ptr_op( xi_globals.backoff_status.next_update.ptr_to_position, !=,
                           NULL );
            }
            else
            {
                tt_ptr_op( xi_globals.backoff_status.next_update.ptr_to_position, ==,
                           NULL );
            }

            tt_ptr_op( &xi_globals.backoff_status.next_update.ptr_to_position, !=,
                       backoff_time_event_position );

            xi__utest__reset_backoff_penalty();
        }

    end:
        xi__utest__reset_backoff_penalty();
        xi_delete_context( xi_context_handle );
        xi_backoff_release();
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_update_backoff_penalty_time__no_data__update_update_time_and_backoff_penalty,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_context_handle_t xi_context_handle = xi_create_context();
        if ( XI_INVALID_CONTEXT_HANDLE >= xi_context_handle )
        {
            tt_fail_msg( "Failed to create default context!" );
            return;
        }

        xi_evtd_instance_t* event_dispatcher = xi_globals.evtd_instance;
        size_t itests = XI_ARRAYSIZE( xi_utest_backoff_test_cases );

        size_t i = 0;
        for ( ; i < itests; ++i )
        {
            const xi_utest_backoff_data_test_case_t* curr_test_case =
                ( xi_utest_backoff_data_test_case_t* )&xi_utest_backoff_test_cases[i];

            xi_backoff_configure_using_data(
                ( xi_vector_elem_t* )curr_test_case->backoff_data,
                ( xi_vector_elem_t* )curr_test_case->decay_data, curr_test_case->data_len,
                XI_MEMORY_TYPE_UNMANAGED );

            xi_inc_backoff_penalty();

            xi_globals.backoff_status.backoff_class = XI_BACKOFF_CLASS_NONE;

            xi_vector_index_type_t* backoff_time_event_position =
                xi_globals.backoff_status.next_update.ptr_to_position;

            xi_backoff_lut_index_t curr_index = xi_globals.backoff_status.backoff_lut_i;

            xi_evtd_step( event_dispatcher,
                          event_dispatcher->current_step +
                              xi_globals.backoff_status.decay_lut->array[curr_index]
                                  .selector_t.ui32_value +
                              1 );

            if ( curr_test_case->data_len > 1 )
            {
                tt_int_op( xi_globals.backoff_status.backoff_lut_i, <, curr_index );
            }
            else
            {
                tt_int_op( xi_globals.backoff_status.backoff_lut_i, ==, 0 );
            }

            tt_ptr_op( xi_globals.backoff_status.next_update.ptr_to_position, ==, NULL );
            tt_ptr_op( xi_globals.backoff_status.next_update.ptr_to_position, !=,
                       backoff_time_event_position );

            xi__utest__reset_backoff_penalty();
        }

    end:
        xi__utest__reset_backoff_penalty();
        xi_delete_context( xi_context_handle );
        xi_backoff_release();
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_context__no_data__after_restarting_xi_context_penalty_must_not_be_changed,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        // initialize xi library.
        xi_context_handle_t xi_context_handle = xi_create_context();
        if ( XI_INVALID_CONTEXT_HANDLE >= xi_context_handle )
        {
            tt_fail_msg( "Failed to create default context!" );
            return;
        }

        xi_inc_backoff_penalty();

        xi_backoff_lut_index_t curr_index = xi_globals.backoff_status.backoff_lut_i;

        xi_delete_context( xi_context_handle );

        xi_context_handle = xi_create_context();
        if ( XI_INVALID_CONTEXT_HANDLE >= xi_context_handle )
        {
            tt_fail_msg( "Failed to create default context!" );
            return;
        }

        tt_want_int_op( curr_index, ==, xi_globals.backoff_status.backoff_lut_i );
        tt_want_int_op( xi_globals.backoff_status.next_update.ptr_to_position, ==, NULL );

        xi_delete_context( xi_context_handle );

    } )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
