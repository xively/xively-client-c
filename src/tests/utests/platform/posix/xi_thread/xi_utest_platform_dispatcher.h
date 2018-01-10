/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"

#include "xi_event_dispatcher_api.h"

#include "xi_critical_section_def.h"

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

xi_state_t register_evtd_handle( xi_event_handle_arg1_t a )
{
    tt_want_int_op( evtd_g_i->cs->cs_state, ==, 0 );

    xi_evtd_instance_t* instance = ( xi_evtd_instance_t* )a;

    evtd_handle_g.handle_type          = XI_EVENT_HANDLE_ARGC1;
    evtd_handle_g.handlers.h1.fn_argc1 = &register_evtd_handle;
    evtd_handle_g.handlers.h1.a1       = a;

    xi_evtd_register_socket_fd( instance, 10, evtd_handle_g );

    tt_want_int_op( evtd_g_i->cs->cs_state, ==, 0 );

    return XI_STATE_OK;
}

xi_state_t stop_evtd_handle( xi_event_handle_arg1_t a )
{
    tt_want_int_op( evtd_g_i->cs->cs_state, ==, 0 );

    xi_evtd_instance_t* instance = ( xi_evtd_instance_t* )a;

    XI_UNUSED( instance );

    return XI_MQTT_BAD_USERNAME_OR_PASSWORD;
}

xi_state_t proc_loop_thread( xi_event_handle_arg1_t a )
{
    tt_want_int_op( evtd_g_i->cs->cs_state, ==, 0 );

    *( ( uint32_t* )a ) -= 1;

    if ( *( ( uint32_t* )a ) > 0 )
    {
        xi_evtd_execute_in( evtd_g_i, evtd_handle_g, 1, NULL );
    }

    tt_want_int_op( evtd_g_i->cs->cs_state, ==, 0 );

    return 0;
}

#endif

XI_TT_TESTCASE( utest__thread_safety_clash__entities_must_not_clash, {
    evtd_g_i = xi_evtd_create_instance();

    uint32_t counter = 10;
    xi_time_t step   = 0;

    evtd_handle_g.handle_type          = XI_EVENT_HANDLE_ARGC1;
    evtd_handle_g.handlers.h1.fn_argc1 = &proc_loop_thread;
    evtd_handle_g.handlers.h1.a1       = ( xi_event_handle_arg1_t )&counter;

    xi_evtd_execute_in( evtd_g_i, evtd_handle_g, 0, NULL );

    xi_event_handle_t evtd_handle = {
        XI_EVENT_HANDLE_ARGC1,
        .handlers.h1 = {&register_evtd_handle, ( xi_event_handle_arg1_t )&evtd_g_i}};
    tt_assert( xi_evtd_register_socket_fd( evtd_g_i, 12, evtd_handle ) != 0 );

    tt_want_int_op( evtd_g_i->cs->cs_state, ==, 0 );
    xi_evtd_continue_when_evt_on_socket( evtd_g_i, XI_EVENT_ERROR, evtd_handle, 12 );
    tt_want_int_op( evtd_g_i->cs->cs_state, ==, 0 );

    while ( evtd_g_i->time_events_container->elem_no > 0 )
    {
        tt_want_int_op( evtd_g_i->cs->cs_state, ==, 0 );
        xi_evtd_continue_when_evt_on_socket( evtd_g_i, XI_EVENT_WANT_READ, evtd_handle,
                                             12 );
        tt_want_int_op( evtd_g_i->cs->cs_state, ==, 0 );

        xi_evtd_step( evtd_g_i, step );
        step += 1;
        tt_assert( counter == 10 - step );
    }

    tt_want_int_op( evtd_g_i->cs->cs_state, ==, 0 );
    tt_assert( counter == 0 );
    tt_assert( step == 10 );
    xi_evtd_unregister_socket_fd( evtd_g_i, 12 );

end:
    xi_evtd_destroy_instance( evtd_g_i );
} )

XI_TT_TESTCASE( utest__thread_safety_clash_time_handler_on_stop__entities_must_not_clash,
                {
                    evtd_g_i = xi_evtd_create_instance();

                    evtd_handle_g.handle_type          = XI_EVENT_HANDLE_ARGC1;
                    evtd_handle_g.handlers.h1.fn_argc1 = &stop_evtd_handle;
                    evtd_handle_g.handlers.h1.a1 = ( xi_event_handle_arg1_t )&evtd_g_i;

                    xi_evtd_execute_in( evtd_g_i, evtd_handle_g, 0, NULL );

                    xi_evtd_step( evtd_g_i, 1 );

                    tt_want_int_op( evtd_g_i->cs->cs_state, ==, 0 );

                    xi_evtd_destroy_instance( evtd_g_i );
                } )

XI_TT_TESTCASE(
    utest__thread_safety_clash_event_handler_on_stop__entities_must_not_clash, {
        evtd_g_i = xi_evtd_create_instance();

        xi_event_handle_t evtd_handle = {
            XI_EVENT_HANDLE_ARGC1,
            .handlers.h1 = {&stop_evtd_handle, ( xi_event_handle_arg1_t )&evtd_g_i}};
        tt_assert( xi_evtd_register_socket_fd( evtd_g_i, 12, evtd_handle ) != 0 );

        tt_want_int_op( evtd_g_i->cs->cs_state, ==, 0 );
        xi_evtd_continue_when_evt_on_socket( evtd_g_i, XI_EVENT_WANT_READ, evtd_handle,
                                             12 );
        tt_want_int_op( evtd_g_i->cs->cs_state, ==, 0 );

        xi_evtd_unregister_socket_fd( evtd_g_i, 12 );
        xi_evtd_destroy_instance( evtd_g_i );

    end:;
    } )
