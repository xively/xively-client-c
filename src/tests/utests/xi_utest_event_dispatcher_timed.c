/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"

#include "xi_vector.h"
#include "xi_io_timeouts.h"
#include "xi_memory_checks.h"
#include "xi_event_dispatcher_api.h"

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

void xi_utest_local__add_handler__evtd( xi_evtd_instance_t* evtd,
                                        xi_event_handle_func_argc4 fn,
                                        xi_event_handle_arg1_t arg1,
                                        xi_event_handle_arg2_t arg2,
                                        xi_event_handle_arg3_t arg3,
                                        xi_event_handle_arg4_t arg4 )
{
    if ( evtd == NULL )
        return;

    xi_event_handle_t evtd_handle = {XI_EVENT_HANDLE_ARGC4,
                                     .handlers.h4 = {fn, arg1, arg2, arg3, arg4}, 0};

    xi_evtd_execute_in( evtd, evtd_handle, 1, NULL );
}

xi_state_t xi_utest_local_action__timed( xi_event_handle_arg1_t target_evtd,
                                         xi_event_handle_arg2_t exec_counter,
                                         xi_event_handle_arg3_t state,
                                         xi_event_handle_arg4_t seconds_since_epoch )
{
    if ( target_evtd == NULL || exec_counter == NULL )
        return XI_STATE_OK;
    {
        uint32_t* counter = ( uint32_t* )exec_counter;

        printf( "%s, counter = %u, time = %ld\n", __func__, ( unsigned int )*counter,
                time( 0 ) );

        if ( 0 < *counter )
        {
            --( *counter );

            tt_want_int_op( ( *( time_t* )seconds_since_epoch ) + 1, ==, time( 0 ) );

            *( time_t* )seconds_since_epoch = time( 0 );

            xi_utest_local__add_handler__evtd( ( xi_evtd_instance_t* )target_evtd,
                                               &xi_utest_local_action__timed, target_evtd,
                                               exec_counter, state, seconds_since_epoch );
        }
    }

    return XI_STATE_OK;
}

xi_state_t xi_utest_local_action__io_timed( void* arg1,
                                            void* vector,
                                            xi_state_t state,
                                            void* execution_counter )
{
    XI_UNUSED( state );

    xi_io_timeout_t* timeout_element = ( xi_io_timeout_t* )arg1;
    xi_time_event_handle_t* event    = &timeout_element->timeout;

    // increase execution count
    *( ( int* )execution_counter ) += 1;

    // remove from vector
    xi_io_timeouts_remove( event, vector );

    XI_SAFE_FREE( timeout_element );

    return XI_STATE_OK;
}

#endif // XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

XI_TT_TESTGROUP_BEGIN( utest_event_dispatcher_timed )

// skipped, because it takes to much time, enable it temporarily for aimed platform tests
SKIP_XI_TT_TESTCASE(
    utest__xi_evtd_execute_in__time_function_misbehaviour_upon_core_switch__test_for_multicore_archs__time_is_measured_same_on_different_cores,
    {
        // this test is for testing the function time( 0 ) whether its result depends
        // on which core it is called.
        xi_evtd_instance_t* evtd = xi_evtd_create_instance();

        uint32_t exec_counter      = 5;
        time_t seconds_since_epoch = time( 0 ) - 1;
        xi_state_t state           = XI_STATE_OK;

        xi_utest_local__add_handler__evtd( evtd, &xi_utest_local_action__timed, evtd,
                                           &exec_counter, state, &seconds_since_epoch );

        while ( 0 < exec_counter )
        {
            // OS might switch core of execution between two calls on function 'time'
            xi_evtd_step( evtd, time( 0 ) );
        }

        // end:
        xi_evtd_destroy_instance( evtd );
    } )

XI_TT_TESTCASE(
    utest__xi_evtd_io_execute_in__delayed_execution__events_should_be_executed, {
        // this test checks if event dispatcher executes all scheduled handles

        xi_evtd_instance_t* evtd = xi_evtd_create_instance();

        int index                                 = 0;
        uint32_t time_counter                     = 0;
        uint32_t execution_counter                = 0;
        xi_state_t state                          = XI_STATE_OK;
        xi_vector_t* vector                       = xi_vector_create();
        xi_vector_t* test_vector                  = xi_vector_create();
        xi_time_event_handle_t* time_event_handle = NULL;

        // schedule 50 handles
        for ( index = 0; index < 50; index++ )
        {
            XI_ALLOC( xi_io_timeout_t, timeout_element, state );

            time_event_handle = &timeout_element->timeout;

            xi_event_handle_t evtd_handle =
                xi_make_handle( &xi_utest_local_action__io_timed, timeout_element, vector,
                                XI_STATE_OK, &execution_counter );

            // schedule handle execution
            xi_io_timeouts_create( evtd, evtd_handle, 10, vector, time_event_handle );

            // store event in the test vector
            xi_vector_push( test_vector, XI_VEC_CONST_VALUE_PARAM(
                                             XI_VEC_VALUE_PTR( timeout_element ) ) );
        }

        // check if vector contains all events
        tt_assert( vector->elem_no == 50 );

        // check if elements are valid
        for ( index = 0; index < vector->elem_no; ++index )
        {
            xi_vector_elem_t element           = vector->array[index];
            xi_vector_elem_t test_element      = test_vector->array[index];
            xi_io_timeout_t* timeout           = test_element.selector_t.ptr_value;
            xi_time_event_handle_t* event      = element.selector_t.ptr_value;
            xi_time_event_handle_t* test_event = &timeout->timeout;
            tt_assert( event == test_event );
        }

        // simulate time stepping
        while ( time_counter < 11 )
        {
            xi_evtd_step( evtd, time_counter );
            time_counter += 1;
        }

        // check if all events are executed
        tt_assert( execution_counter == 50 );

        // check if all executed events are removed from vector
        tt_assert( vector->elem_no == 0 );

        goto end;

    err_handling:
        tt_abort_msg( "test should not fail" );

    end:
        xi_vector_destroy( vector );
        xi_vector_destroy( test_vector );
        xi_evtd_destroy_instance( evtd );

        tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
    } )

XI_TT_TESTCASE( utest__xi_evtd_io_cancel__delayed_execution__events_should_be_cancelled, {
    // this test checks if event dispatcher cancels scheduled handles

    xi_evtd_instance_t* evtd = xi_evtd_create_instance();

    int index                                 = 0;
    uint32_t time_counter                     = 0;
    uint32_t execution_counter                = 0;
    xi_state_t state                          = XI_STATE_OK;
    xi_vector_t* vector                       = xi_vector_create();
    xi_vector_t* test_vector                  = xi_vector_create();
    xi_time_event_handle_t* time_event_handle = NULL;

    // schedule 50 handles
    for ( index = 0; index < 50; index++ )
    {
        XI_ALLOC( xi_io_timeout_t, timeout_element, state );

        time_event_handle = &timeout_element->timeout;

        xi_event_handle_t evtd_handle =
            xi_make_handle( &xi_utest_local_action__io_timed, timeout_element, vector,
                            XI_STATE_OK, &execution_counter );

        // schedule handle execution
        xi_io_timeouts_create( evtd, evtd_handle, 10, vector, time_event_handle );

        // store event in the test vector
        xi_vector_push( test_vector,
                        XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( timeout_element ) ) );
    }

    // simulate time stepping
    while ( time_counter < 11 )
    {
        // cancel 25 elements
        if ( time_counter == 5 )
        {
            for ( index = 0; index < 25; ++index )
            {
                xi_vector_elem_t element         = test_vector->array[index];
                xi_io_timeout_t* timeout_element = element.selector_t.ptr_value;
                xi_time_event_handle_t* event    = &timeout_element->timeout;
                xi_io_timeouts_cancel( evtd, event, vector );
                XI_SAFE_FREE( timeout_element );
            }
        }
        xi_evtd_step( evtd, time_counter );
        time_counter += 1;
    }

    // checking if remaining events are executed
    tt_assert( execution_counter == 25 );

    // check if vector contains no remaining events, they are both executed or cancelled
    tt_assert( vector->elem_no == 0 );

    goto end;

err_handling:
    tt_abort_msg( "test should not fail" );

end:
    xi_vector_destroy( vector );
    xi_vector_destroy( test_vector );
    xi_evtd_destroy_instance( evtd );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
} )

XI_TT_TESTCASE(
    utest__xi_evtd_io_restart__delayed_execution__events_should_be_restarted, {
        // this test checks if event dispatcher reschedules handles

        xi_evtd_instance_t* evtd = xi_evtd_create_instance();

        int index                                 = 0;
        uint32_t time_counter                     = 0;
        uint32_t execution_counter                = 0;
        xi_state_t state                          = XI_STATE_OK;
        xi_vector_t* vector                       = xi_vector_create();
        xi_vector_t* test_vector                  = xi_vector_create();
        xi_time_event_handle_t* time_event_handle = NULL;

        // schedule 50 handles
        for ( index = 0; index < 50; index++ )
        {
            XI_ALLOC( xi_io_timeout_t, timeout_element, state );

            time_event_handle = &timeout_element->timeout;

            xi_event_handle_t evtd_handle =
                xi_make_handle( &xi_utest_local_action__io_timed, timeout_element, vector,
                                XI_STATE_OK, &execution_counter );

            // schedule handle execution
            xi_io_timeouts_create( evtd, evtd_handle, 10, vector, time_event_handle );

            // store first 25 events in the test vector
            if ( index < 25 )
            {
                xi_vector_push( test_vector, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR(
                                                 time_event_handle ) ) );
            }
        }

        // simulate time stepping
        while ( time_counter < 20 )
        {
            // restart first 25 elements
            if ( time_counter == 6 )
            {
                xi_io_timeouts_restart( evtd, 10, test_vector );
            }
            // check if last 25 was executed
            if ( time_counter == 11 )
            {
                tt_assert( execution_counter == 25 );
            }
            // check if first 25 was executed after restart
            if ( time_counter == 16 )
            {
                tt_assert( execution_counter == 50 );
            }
            xi_evtd_step( evtd, time_counter );

            time_counter += 1;
        }

        goto end;

    err_handling:
        tt_abort_msg( "test should not fail" );

    end:
        xi_vector_destroy( vector );
        xi_vector_destroy( test_vector );
        xi_evtd_destroy_instance( evtd );

        tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
    } )


XI_TT_TESTCASE( utest__xi_evtd_io_remove__delayed_execution__events_should_be_removed, {
    // this test checks if event dispatcher removes events from events vector

    xi_evtd_instance_t* evtd = xi_evtd_create_instance();

    int index                                 = 0;
    uint32_t execution_counter                = 0;
    xi_state_t state                          = XI_STATE_OK;
    xi_vector_t* vector                       = xi_vector_create();
    xi_vector_t* test_vector                  = xi_vector_create();
    xi_vector_t* element_vector               = xi_vector_create();
    xi_time_event_handle_t* time_event_handle = NULL;

    // schedule 50 handles
    for ( index = 0; index < 50; index++ )
    {
        XI_ALLOC( xi_io_timeout_t, timeout_element, state );

        time_event_handle = &timeout_element->timeout;

        xi_event_handle_t evtd_handle =
            xi_make_handle( &xi_utest_local_action__io_timed, timeout_element, vector,
                            XI_STATE_OK, &execution_counter );

        // schedule handle execution
        xi_io_timeouts_create( evtd, evtd_handle, 10, vector, time_event_handle );

        // store first 25 events in the test vector
        if ( index < 25 )
        {
            xi_vector_push( test_vector, XI_VEC_CONST_VALUE_PARAM(
                                             XI_VEC_VALUE_PTR( time_event_handle ) ) );
        }

        // store event in the test vector
        xi_vector_push( element_vector,
                        XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( timeout_element ) ) );
    }

    // removing events
    for ( index = 0; index < 25; index++ )
    {
        xi_vector_elem_t element = test_vector->array[index];
        time_event_handle        = element.selector_t.ptr_value;

        xi_io_timeouts_remove( time_event_handle, vector );
    }

    // check if last25 was removed
    tt_assert( vector->elem_no == 25 );

    // cleanup
    for ( index = 0; index < 50; ++index )
    {
        xi_vector_elem_t element         = element_vector->array[index];
        xi_io_timeout_t* timeout_element = element.selector_t.ptr_value;
        xi_time_event_handle_t* event    = &timeout_element->timeout;
        xi_io_timeouts_cancel( evtd, event, vector );
        XI_SAFE_FREE( timeout_element );
    }

    goto end;

err_handling:
    tt_abort_msg( "test should not fail" );

end:
    xi_vector_destroy( vector );
    xi_vector_destroy( test_vector );
    xi_vector_destroy( element_vector );
    xi_evtd_destroy_instance( evtd );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
} )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
