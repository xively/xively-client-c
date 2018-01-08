/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"
#include "xi_utest_basic_testcase_frame.h"
#include "xi_memory_checks.h"
#include "xi_bsp_rng.h"

#include "xi_handle.h"
#include "xi_vector.h"
#include "xi_time_event.h"
#include "xively.h"

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#define TEST_TIME_EVENT_TEST_SIZE 64

typedef xi_time_t( time_event_container_element_generator )( int index );

static xi_time_t index_generator( int index )
{
    return index;
}

static xi_time_t random_generator_0_1000( int index )
{
    XI_UNUSED( index );

    return xi_bsp_rng_get() % 1000;
}

static xi_state_t fill_vector_with_heap_elements_using_generator(
    xi_vector_t* vector,
    xi_time_event_t ( *time_events )[TEST_TIME_EVENT_TEST_SIZE],
    xi_time_event_handle_t ( *time_event_handles )[TEST_TIME_EVENT_TEST_SIZE],
    time_event_container_element_generator* generator_fn )
{
    xi_state_t state = XI_STATE_OK;

    int i = 0;
    for ( ; i < TEST_TIME_EVENT_TEST_SIZE; ++i )
    {
        time_events[0][i].time_of_execution = generator_fn( i );

        xi_state_t ret_state =
            xi_time_event_add( vector, &time_events[0][i], &time_event_handles[0][i] );

        XI_CHECK_STATE( ret_state );

        tt_assert( ret_state == XI_STATE_OK );
        tt_assert( time_event_handles[0][i].ptr_to_position != NULL );
    }

end:
err_handling:
    return state;
}

#endif

XI_TT_TESTGROUP_BEGIN( utest_time_event )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_time_event_execute_handle_in__single_time_event_added,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {

        xi_vector_t* vector                      = xi_vector_create();
        xi_time_event_t time_event               = xi_make_empty_time_event();
        xi_time_event_handle_t time_event_handle = xi_make_empty_time_event_handle();

        xi_state_t ret_state =
            xi_time_event_add( vector, &time_event, &time_event_handle );

        tt_assert( ret_state == XI_STATE_OK );
        tt_assert( time_event_handle.ptr_to_position != NULL );

        xi_vector_destroy( vector );
    end:;
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_time_event_execute_handle_in__add_TEST_TIME_EVENT_TEST_SIZE_random_time_events,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_bsp_rng_init();

        xi_vector_t* vector = xi_vector_create();

        xi_time_event_handle_t time_event_handles[TEST_TIME_EVENT_TEST_SIZE] = {
            xi_make_empty_time_event_handle()};
        xi_time_event_t time_events[TEST_TIME_EVENT_TEST_SIZE] = {
            xi_make_empty_time_event()};

        xi_state_t ret_state = fill_vector_with_heap_elements_using_generator(
            vector, &time_events, &time_event_handles, &random_generator_0_1000 );

        tt_assert( XI_STATE_OK == ret_state );

        /* counter for tracking the number of elements */
        int no_elements = 0;
        /* for tracking the order of the container */
        int last_element_value = 0;

        /* at this point we should have a heap constructed, we can test if taking elements
         * from the top we will receive them in a sorted order */
        do
        {
            xi_time_event_t* time_event = xi_time_event_get_top( vector );
            tt_assert( time_event->time_of_execution >= last_element_value );
            last_element_value = time_event->time_of_execution;
            ++no_elements;
        } while ( vector->elem_no != 0 );

        /* and we can check if all of them has been received */
        tt_assert( no_elements == TEST_TIME_EVENT_TEST_SIZE );

        xi_vector_destroy( vector );
    end:
        xi_bsp_rng_shutdown();
    } )


XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_time_event_restart_first_element__event_key_and_position_changed_positive,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_time_t original_position = 0;
        for ( ; original_position < TEST_TIME_EVENT_TEST_SIZE; ++original_position )
        {
            xi_time_event_handle_t time_event_handles[TEST_TIME_EVENT_TEST_SIZE] = {
                xi_make_empty_time_event_handle()};
            xi_time_event_t time_events[TEST_TIME_EVENT_TEST_SIZE] = {
                xi_make_empty_time_event()};

            xi_vector_t* vector = xi_vector_create();

            xi_state_t ret_state = fill_vector_with_heap_elements_using_generator(
                vector, &time_events, &time_event_handles, &index_generator );

            int i = 0;
            for ( ; i < TEST_TIME_EVENT_TEST_SIZE; ++i )
            {
                tt_assert( *time_event_handles[i].ptr_to_position == i );
            }

            tt_assert( XI_STATE_OK == ret_state );

            const xi_time_t new_test_time = TEST_TIME_EVENT_TEST_SIZE + 12;

            ret_state = xi_time_event_restart(
                vector, &time_event_handles[original_position], new_test_time );

            tt_assert( XI_STATE_OK == ret_state );

            xi_time_event_t* time_event =
                ( xi_time_event_t* )vector->array[TEST_TIME_EVENT_TEST_SIZE - 1]
                    .selector_t.ptr_value;

            tt_assert( time_event->time_of_execution == new_test_time );
            tt_assert( time_event->position ==
                       *time_event_handles[original_position].ptr_to_position );

            xi_vector_destroy( vector );
        }
    end:;
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_time_event_restart_first_element__event_key_and_position_changed_negative,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_time_t original_position = 0;
        for ( ; original_position < TEST_TIME_EVENT_TEST_SIZE; ++original_position )
        {
            xi_time_event_handle_t time_event_handles[TEST_TIME_EVENT_TEST_SIZE] = {
                xi_make_empty_time_event_handle()};
            xi_time_event_t time_events[TEST_TIME_EVENT_TEST_SIZE] = {
                xi_make_empty_time_event()};

            xi_vector_t* vector = xi_vector_create();

            xi_state_t ret_state = fill_vector_with_heap_elements_using_generator(
                vector, &time_events, &time_event_handles, &index_generator );

            int i = 0;
            for ( ; i < TEST_TIME_EVENT_TEST_SIZE; ++i )
            {
                tt_assert( *time_event_handles[i].ptr_to_position == i );
            }

            tt_assert( XI_STATE_OK == ret_state );

            const xi_time_t new_test_time = -1;

            ret_state = xi_time_event_restart(
                vector, &time_event_handles[original_position], new_test_time );

            tt_assert( XI_STATE_OK == ret_state );

            xi_time_event_t* time_event =
                ( xi_time_event_t* )vector->array[0].selector_t.ptr_value;

            tt_assert( time_event->time_of_execution == new_test_time );
            tt_assert( time_event->position ==
                       *time_event_handles[original_position].ptr_to_position );

            xi_vector_destroy( vector );
        }
    end:;
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_time_event_cancel_all_elements__elements_removed_their_handlers_cleared,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {

        xi_vector_t* vector = xi_vector_create();

        xi_time_event_handle_t time_event_handles[TEST_TIME_EVENT_TEST_SIZE] = {
            xi_make_empty_time_event_handle()};
        xi_time_event_t time_events[TEST_TIME_EVENT_TEST_SIZE] = {
            xi_make_empty_time_event()};

        xi_state_t ret_state = fill_vector_with_heap_elements_using_generator(
            vector, &time_events, &time_event_handles, &index_generator );

        tt_assert( XI_STATE_OK == ret_state );

        {
            size_t i = 0;
            for ( ; i < TEST_TIME_EVENT_TEST_SIZE; ++i )
            {
                xi_time_event_t* cancelled_time_event = NULL;
                const xi_state_t local_state          = xi_time_event_cancel(
                    vector, &time_event_handles[i], &cancelled_time_event );

                tt_assert( XI_STATE_OK == local_state );
            }
        }

        /* vector should be empty */
        tt_assert( 0 == vector->elem_no );

        {
            size_t i = 0;
            for ( ; i < TEST_TIME_EVENT_TEST_SIZE; ++i )
            {
                tt_assert( NULL == time_event_handles[i].ptr_to_position );
            }
        }

        xi_vector_destroy( vector );
    end:;
    } )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
