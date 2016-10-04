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

typedef xi_time_t( heap_element_generator )( int index );

static xi_time_t index_generator( int index )
{
    return index;
}

static xi_time_t random_generator_0_1000( int index )
{
    XI_UNUSED( index );

    return random() % 1000;
}

static xi_state_t fill_vector_with_heap_elements_using_generator(
    xi_vector_t* vector,
    xi_time_event_t ( *time_events )[TEST_TIME_EVENT_TEST_SIZE],
    xi_time_event_handle_t ( *time_event_handles )[TEST_TIME_EVENT_TEST_SIZE],
    heap_element_generator* generator_fn )
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

        xi_state_t ret_state = fill_vector_with_heap_elements_using_generator(
            vector, &time_events, &time_event_handles, &random_generator_0_1000 );

        tt_assert( XI_STATE_OK == ret_state );

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

        xi_state_t ret_state = fill_vector_with_heap_elements_using_generator(
            vector, &time_events, &time_event_handles, &index_generator );

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

XI_TT_TESTCASE(
    utest__xi_time_event_cancel_all_elements__elements_removed_their_handlers_cleared, {

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
                tt_assert( NULL == time_event_handles[i].position );
            }
        }

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
