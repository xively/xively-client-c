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
#include <limits.h>

#include "xi_memory_checks.h"
#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"

#include "xi_event_dispatcher_api.h"

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

static uint32_t g_cont0_test = 0;

xi_state_t continuation( void )
{
    g_cont0_test = 127;
    return 0;
}

xi_state_t continuation1( xi_event_handle_arg1_t a )
{
    *( ( uint32_t* )a ) = 127;
    return 0;
}

xi_state_t continuation1_1( xi_event_handle_arg1_t a )
{
    *( ( uint32_t* )a ) += 1;
    return 0;
}

xi_state_t continuation1_3( xi_event_handle_arg1_t a )
{
    *( ( uint32_t* )a ) += 3;
    return 0;
}

xi_state_t continuation1_5( xi_event_handle_arg1_t a )
{
    *( ( uint32_t* )a ) += 5;
    return 0;
}

static xi_evtd_instance_t* evtd_g_i = 0;
static xi_event_handle_t evtd_handle_g;

xi_state_t proc_loop( xi_event_handle_arg1_t a )
{
    *( ( uint32_t* )a ) -= 1;

    if ( *( ( uint32_t* )a ) > 0 )
    {
        xi_evtd_execute_in( evtd_g_i, evtd_handle_g, 1, NULL );
    }
    return 0;
}

void test_time_overflow_function( void )
{
    evtd_g_i = xi_evtd_create_instance();

    tt_assert( NULL != evtd_g_i );

    /* counter that will be used by this test */
    uint32_t counter = 0;

    const xi_time_t time_max  = LONG_MAX;
    const xi_time_t time_hole = 10;

    printf( "%ld\n", time_max );

    evtd_g_i->current_step = time_max - time_hole;

    /* now let's add some events to be executed */
    /* we will add some events before the MAX( xi_time_t ) */
    {
        xi_time_t i = 0;
        for ( ; i < time_hole; ++i )
        {
            const xi_state_t ret_state = xi_evtd_execute_in(
                evtd_g_i, xi_make_handle( &continuation1_1, &counter ), i, NULL );

            xi_debug_format( "ret_state = %d", ret_state );

            tt_int_op( XI_STATE_OK, ==, ret_state );
        }
    }

    /* and some after the MIN( xi_time_t ) in order to check the reaction on time
     * overlap */
    {
        xi_time_t i = 0;
        for ( ; i < time_hole; ++i )
        {
            const xi_state_t ret_state = xi_evtd_execute_in(
                evtd_g_i, xi_make_handle( &continuation1_1, &counter ), time_hole + i,
                NULL );
            tt_int_op( XI_STATE_OK, ==, ret_state );
        }
    }

    /* let's execute first half */
    xi_evtd_step( evtd_g_i, time_max - ( time_hole / 2 ) );

    /* expectation is that only the half of the events added before the MAX( xi_time_t )
     * will be executed */
    tt_int_op( counter, ==, ( time_hole / 2 ) );

    xi_evtd_destroy_instance( evtd_g_i );

    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );

    return;
end:;
}

#endif

/*-----------------------------------------------------------------------*/
// DISPATCHER TESTS
/*-----------------------------------------------------------------------*/
XI_TT_TESTGROUP_BEGIN( utest_event_dispatcher )

#ifdef XI_MODULE_THREAD_ENABLED
#include "xi_utest_platform_dispatcher.h"
#endif

XI_TT_TESTCASE( utest__continuation0, {

    xi_evtd_instance_t* evtd_i = xi_evtd_create_instance();

    {
        xi_event_handle_t evtd_handle = {XI_EVENT_HANDLE_ARGC0,
                                         .handlers.h0 = {&continuation}};
        xi_evtd_execute_handle( &evtd_handle );
    }

    tt_assert( g_cont0_test == 127 );

end:
    xi_evtd_destroy_instance( evtd_i );
} )

XI_TT_TESTCASE( utest__continuation1, {
    xi_evtd_instance_t* evtd_i = xi_evtd_create_instance();

    uint32_t counter = 0;

    {
        xi_event_handle_t evtd_handle = {
            XI_EVENT_HANDLE_ARGC1,
            .handlers.h1 = {&continuation1, ( xi_event_handle_arg1_t )&counter}};
        xi_evtd_execute_handle( &evtd_handle );
    }

    tt_assert( counter == 127 );

end:
    xi_evtd_destroy_instance( evtd_i );
} )

XI_TT_TESTCASE( utest__handler_processing_loop, {
    evtd_g_i = xi_evtd_create_instance();

    uint32_t counter = 10;
    xi_time_t step   = 0;


    evtd_handle_g.handle_type          = XI_EVENT_HANDLE_ARGC1;
    evtd_handle_g.handlers.h1.fn_argc1 = &proc_loop;
    evtd_handle_g.handlers.h1.a1       = ( xi_event_handle_arg1_t )&counter;

    xi_evtd_execute_in( evtd_g_i, evtd_handle_g, 0, NULL );

    while ( evtd_g_i->time_events_container->elem_no > 0 )
    {
        xi_evtd_step( evtd_g_i, step );
        step += 1;
        tt_assert( counter == 10u - step );
    }

    tt_assert( counter == 0 );
    tt_assert( step == 10 );

end:
    xi_evtd_destroy_instance( evtd_g_i );

} )

XI_TT_TESTCASE( utest__register_fd, {
    evtd_g_i = xi_evtd_create_instance();

    xi_event_handle_t handle = xi_make_empty_handle();

    tt_assert( xi_evtd_register_socket_fd( evtd_g_i, 15, handle ) != 0 );
    {
        xi_evtd_fd_tuple_t* tmp =
            ( xi_evtd_fd_tuple_t* )evtd_g_i->handles_and_socket_fd->array[0]
                .selector_t.ptr_value;
        tt_assert( tmp->fd == 15 );
        tt_assert( tmp->event_type == XI_EVENT_WANT_READ );
    }

    tt_assert( xi_evtd_register_socket_fd( evtd_g_i, 14, handle ) );
    {
        xi_evtd_fd_tuple_t* tmp =
            ( xi_evtd_fd_tuple_t* )evtd_g_i->handles_and_socket_fd->array[1]
                .selector_t.ptr_value;
        tt_assert( tmp->fd == 14 );
        tt_assert( tmp->event_type == XI_EVENT_WANT_READ );
    }

    tt_assert( xi_evtd_register_socket_fd( evtd_g_i, 12, handle ) );
    {
        xi_evtd_fd_tuple_t* tmp =
            ( xi_evtd_fd_tuple_t* )evtd_g_i->handles_and_socket_fd->array[2]
                .selector_t.ptr_value;
        tt_assert( tmp->fd == 12 );
        tt_assert( tmp->event_type == XI_EVENT_WANT_READ );
    }


    xi_evtd_unregister_socket_fd( evtd_g_i, 12 );
    xi_evtd_unregister_socket_fd( evtd_g_i, 15 );
    xi_evtd_unregister_socket_fd( evtd_g_i, 14 );

end:
    xi_evtd_destroy_instance( evtd_g_i );
} )

XI_TT_TESTCASE( utest__evtd_updates, {
    uint32_t counter = 0;

    evtd_g_i = xi_evtd_create_instance();

    {
        xi_event_handle_t evtd_handle = {
            XI_EVENT_HANDLE_ARGC1,
            .handlers.h1 = {&continuation1_1, ( xi_event_handle_arg1_t )&counter}};

        tt_assert( xi_evtd_register_socket_fd( evtd_g_i, 15, evtd_handle ) != 0 );
        tt_assert( xi_evtd_register_socket_fd( evtd_g_i, 14, evtd_handle ) != 0 );
        tt_assert( xi_evtd_register_socket_fd( evtd_g_i, 12, evtd_handle ) != 0 );
    }

    {
        xi_event_handle_t evtd_handle = {
            XI_EVENT_HANDLE_ARGC1,
            .handlers.h1 = {&continuation1_1, ( xi_event_handle_arg1_t )&counter}};
        xi_evtd_continue_when_evt_on_socket( evtd_g_i, XI_EVENT_WANT_READ, evtd_handle,
                                             15 );

        xi_evtd_fd_tuple_t* tmp =
            ( xi_evtd_fd_tuple_t* )evtd_g_i->handles_and_socket_fd->array[0]
                .selector_t.ptr_value;

        tt_assert( tmp->event_type == XI_EVENT_WANT_READ );
        tt_assert( tmp->handle.handle_type == XI_EVENT_HANDLE_ARGC1 );
        tt_assert( tmp->handle.handlers.h1.a1 == ( xi_event_handle_arg1_t )&counter );
        tt_assert( tmp->handle.handlers.h1.fn_argc1 == &continuation1_1 );
    }

    {
        xi_event_handle_t evtd_handle = {
            XI_EVENT_HANDLE_ARGC1,
            .handlers.h1 = {&continuation1_3, ( xi_event_handle_arg1_t )&counter}};
        xi_evtd_continue_when_evt_on_socket( evtd_g_i, XI_EVENT_WANT_WRITE, evtd_handle,
                                             14 );

        xi_evtd_fd_tuple_t* tmp =
            ( xi_evtd_fd_tuple_t* )evtd_g_i->handles_and_socket_fd->array[1]
                .selector_t.ptr_value;

        tt_assert( tmp->event_type == XI_EVENT_WANT_WRITE );
        tt_assert( tmp->handle.handle_type == XI_EVENT_HANDLE_ARGC1 );
        tt_assert( tmp->handle.handlers.h1.a1 == ( xi_event_handle_arg1_t )&counter );
        tt_assert( tmp->handle.handlers.h1.fn_argc1 == &continuation1_3 );
    }

    {
        xi_event_handle_t evtd_handle = {
            XI_EVENT_HANDLE_ARGC1,
            .handlers.h1 = {&continuation1_5, ( xi_event_handle_arg1_t )&counter}};
        xi_evtd_continue_when_evt_on_socket( evtd_g_i, XI_EVENT_ERROR, evtd_handle, 12 );

        xi_evtd_fd_tuple_t* tmp =
            ( xi_evtd_fd_tuple_t* )evtd_g_i->handles_and_socket_fd->array[2]
                .selector_t.ptr_value;

        tt_assert( tmp->event_type == XI_EVENT_ERROR );
        tt_assert( tmp->handle.handle_type == XI_EVENT_HANDLE_ARGC1 );
        tt_assert( tmp->handle.handlers.h1.a1 == ( xi_event_handle_arg1_t )&counter );
        tt_assert( tmp->handle.handlers.h1.fn_argc1 == &continuation1_5 );
    }


    tt_assert( counter == 0 );
    xi_evtd_update_event_on_socket( evtd_g_i, 12 );
    tt_assert( counter == 5 );

    counter = 0;

    {
        xi_event_handle_t evtd_handle = {
            XI_EVENT_HANDLE_ARGC1,
            .handlers.h1 = {&continuation1_5, ( xi_event_handle_arg1_t )&counter}};
        xi_evtd_continue_when_evt_on_socket( evtd_g_i, XI_EVENT_ERROR, evtd_handle, 12 );
    }

    tt_assert( counter == 0 );
    xi_evtd_update_event_on_socket( evtd_g_i, 12 );
    xi_evtd_update_event_on_socket( evtd_g_i, 15 );
    tt_assert( counter == 6 );

    counter = 0;

    {
        xi_event_handle_t evtd_handle = {
            XI_EVENT_HANDLE_ARGC1,
            .handlers.h1 = {&continuation1_5, ( xi_event_handle_arg1_t )&counter}};
        xi_evtd_continue_when_evt_on_socket( evtd_g_i, XI_EVENT_ERROR, evtd_handle, 12 );
    }
    {
        xi_event_handle_t evtd_handle = {
            XI_EVENT_HANDLE_ARGC1,
            .handlers.h1 = {&continuation1_3, ( xi_event_handle_arg1_t )&counter}};
        xi_evtd_continue_when_evt_on_socket( evtd_g_i, XI_EVENT_WANT_WRITE, evtd_handle,
                                             14 );
    }

    tt_assert( counter == 0 );
    xi_evtd_update_event_on_socket( evtd_g_i, 12 );
    xi_evtd_update_event_on_socket( evtd_g_i, 15 );
    xi_evtd_update_event_on_socket( evtd_g_i, 14 );
    tt_assert( counter == 9 );

    xi_evtd_unregister_socket_fd( evtd_g_i, 12 );
    xi_evtd_unregister_socket_fd( evtd_g_i, 15 );
    xi_evtd_unregister_socket_fd( evtd_g_i, 14 );

end:
    xi_evtd_destroy_instance( evtd_g_i );
} )

/* skipped because this feature is not yet implemented */
SKIP_XI_TT_TESTCASE(
    utest__xi_evtd__events_to_call_added__overlap_timer__proper_events_executed,
    { test_time_overflow_function(); } )

#undef TEST_DATA_SIZE

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
