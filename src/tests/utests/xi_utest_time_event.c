#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"
#include "xi_memory_checks.h"

#include "xi_handle.h"
#include "xi_vector.h"
#include "xi_time_event.h"
#include "xively.h"

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#define TEST_TIME_EVENT_TEST_SIZE 64
void test_time_event()
{
}

static xi_state_t fill_vector_with_heap_elements_linear_order(
    xi_vector_t* vector,
    xi_time_event_t ( *time_events )[TEST_TIME_EVENT_TEST_SIZE],
    xi_time_event_handle_t ( *time_event_handles )[TEST_TIME_EVENT_TEST_SIZE] )
{
    xi_state_t state = XI_STATE_OK;

    int i = 0;
    for ( ; i < TEST_TIME_EVENT_TEST_SIZE; ++i )
    {
        time_events[0][i].time_of_execution = i;

        xi_state_t ret_state =
            xi_time_event_add( vector, &time_events[0][i], &time_event_handles[0][i] );

        XI_CHECK_STATE( ret_state );

        tt_assert( ret_state == XI_STATE_OK );
        tt_assert( time_event_handles[0][i].position != NULL );
        tt_assert( *( time_event_handles[0][i].position ) == i );
    }

end:
err_handling:
    return state;
}

#endif

XI_TT_TESTGROUP_BEGIN( utest_time_event )

XI_TT_TESTCASE( utest__xi_time_event_execute_handle_in__time_event_added, {

    xi_vector_t* vector                      = xi_vector_create();
    xi_time_event_t time_event               = xi_make_empty_time_event();
    xi_time_event_handle_t time_event_handle = xi_make_empty_time_event_handle();

    xi_state_t ret_state = xi_time_event_add( vector, &time_event, &time_event_handle );

    tt_assert( ret_state == XI_STATE_OK );
    tt_assert( time_event_handle.position != NULL );

    xi_vector_destroy( vector );

    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
end:;

} )

XI_TT_TESTCASE(
    utest__xi_time_event_execute_handle_in__add_TEST_TIME_EVENT_TEST_SIZE_random_time_events,
    {
        xi_vector_t* vector = xi_vector_create();

        xi_time_event_handle_t time_event_handles[TEST_TIME_EVENT_TEST_SIZE] = {
            xi_make_empty_time_event_handle()};
        xi_time_event_t time_events[TEST_TIME_EVENT_TEST_SIZE] = {
            xi_make_empty_time_event()};

        int i = 0;
        for ( ; i < TEST_TIME_EVENT_TEST_SIZE; ++i )
        {
            time_events[i].time_of_execution = ( xi_time_t )rand() % 1000;

            xi_state_t ret_state =
                xi_time_event_add( vector, &time_events[i], &time_event_handles[i] );

            tt_assert( ret_state == XI_STATE_OK );
            tt_assert( time_event_handles[i].position != NULL );
        }

        /* counter for tracking the number of elements */
        int no_elements = 0;
        /* for tracking the order of the heap */
        int last_element_value = 0;

        /* at this point we should have a heap constructed, we can test if taking elements
         * from the top we will receive them in a sorted order */
        do
        {
            xi_time_event_t* time_event = xi_time_event_get_top( vector );
            tt_assert( time_event->time_of_execution >= last_element_value );
            last_element_value = time_event->time_of_execution;
            ++no_elements;
            /* we have to relelase the memory */
        } while ( vector->elem_no != 0 );

        /* and we can check if all of them has been received */
        tt_assert( no_elements == TEST_TIME_EVENT_TEST_SIZE );

        xi_vector_destroy( vector );

        tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
    end:;
    } )


XI_TT_TESTCASE(
    utest__xi_time_event_restart_first_element__event_key_and_position_changed, {
        xi_vector_t* vector = xi_vector_create();

        xi_time_event_handle_t time_event_handles[TEST_TIME_EVENT_TEST_SIZE] = {
            xi_make_empty_time_event_handle()};
        xi_time_event_t time_events[TEST_TIME_EVENT_TEST_SIZE] = {
            xi_make_empty_time_event()};

        xi_state_t ret_state = fill_vector_with_heap_elements_linear_order(
            vector, &time_events, &time_event_handles );

        tt_assert( XI_STATE_OK == ret_state );

        const xi_time_t new_test_time = TEST_TIME_EVENT_TEST_SIZE + 12;

        ret_state =
            xi_time_event_restart( vector, &time_event_handles[0], new_test_time );

        tt_assert( XI_STATE_OK == ret_state );

        xi_time_event_t* time_event =
            ( xi_time_event_t* )vector->array[TEST_TIME_EVENT_TEST_SIZE - 1]
                .selector_t.ptr_value;

        tt_assert( time_event->time_of_execution == new_test_time );
        tt_assert( time_event->position == *time_event_handles[0].position );

        xi_vector_destroy( vector );
        tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
    end:;
    } )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
