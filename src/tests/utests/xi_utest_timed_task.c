/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"
#include "xi_memory_checks.h"

#include "xi_timed_task.h"
#include "xi_vector.h"

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

xi_context_handle_t xi_utest_timed_task_last_context_handle       = -1;
xi_timed_task_handle_t xi_utest_timed_task_last_timed_task_handle = -1;
void* xi_utest_timed_task_user_data                               = NULL;

void xi_utest_timed_task_reset_globals()
{
    xi_utest_timed_task_last_context_handle    = -1;
    xi_utest_timed_task_last_timed_task_handle = -1;
    xi_utest_timed_task_user_data              = NULL;
}

void xi_utest_timed_task_callback( const xi_context_handle_t context_handle,
                                   const xi_timed_task_handle_t timed_task_handle,
                                   void* user_data )
{
    xi_utest_timed_task_last_context_handle    = context_handle;
    xi_utest_timed_task_last_timed_task_handle = timed_task_handle;
    xi_utest_timed_task_user_data              = user_data;
}

void xi_utest_timed_task_callback_remove_timed_task(
    const xi_context_handle_t context_handle,
    const xi_timed_task_handle_t timed_task_handle,
    void* user_data )
{
    xi_timed_task_container_t* container = ( xi_timed_task_container_t* )user_data;
    xi_remove_timed_task( container, timed_task_handle );
    xi_utest_timed_task_last_context_handle    = context_handle;
    xi_utest_timed_task_last_timed_task_handle = timed_task_handle;
    xi_utest_timed_task_user_data              = user_data;
}


#endif

XI_TT_TESTGROUP_BEGIN( utest_timed_task )

// ------------------------------ SINGLE TIMED TASK TESTS ------------------------------
// //

XI_TT_TESTCASE( utest__xi_make_timed_task_container__ctor_dtor, {
    xi_timed_task_container_t* container = xi_make_timed_task_container();
    tt_want_ptr_op( NULL, !=, container );
    xi_destroy_timed_task_container( container );
    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
end:;
} )

XI_TT_TESTCASE( utest__add_timed_task__returns_valid_handle, {
    xi_evtd_instance_t* dispatcher       = xi_evtd_create_instance();
    xi_timed_task_container_t* container = xi_make_timed_task_container();

    xi_context_handle_t context_handle = 1;

    xi_timed_task_handle_t task_handle =
        xi_add_timed_task( container, dispatcher, context_handle,
                           &xi_utest_timed_task_callback, 0, 0, NULL );
    tt_want_int_op( task_handle, >=, 0 );

    xi_remove_timed_task( container, task_handle );
    xi_destroy_timed_task_container( container );
    xi_evtd_destroy_instance( dispatcher );
    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
end:;
} )

XI_TT_TESTCASE( utest__add_timed_task__callback_called, {
    xi_utest_timed_task_reset_globals();
    xi_evtd_instance_t* dispatcher       = xi_evtd_create_instance();
    xi_timed_task_container_t* container = xi_make_timed_task_container();

    xi_context_handle_t context_handle = 1;
    void* user_data                    = ( void* )( intptr_t )2;

    xi_timed_task_handle_t task_handle =
        xi_add_timed_task( container, dispatcher, context_handle,
                           &xi_utest_timed_task_callback, 0, 0, user_data );

    tt_want_int_op( context_handle, !=, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle, !=, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data, !=, xi_utest_timed_task_user_data );

    xi_evtd_step( dispatcher, 1 );

    tt_want_int_op( context_handle, ==, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle, ==, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data, ==, xi_utest_timed_task_user_data );

    xi_destroy_timed_task_container( container );
    xi_evtd_destroy_instance( dispatcher );
    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
end:;
} )

XI_TT_TESTCASE( utest__remove_timed_task__called_after_fired_callback_wont_break, {
    xi_utest_timed_task_reset_globals();
    xi_evtd_instance_t* dispatcher       = xi_evtd_create_instance();
    xi_timed_task_container_t* container = xi_make_timed_task_container();

    xi_context_handle_t context_handle = 1;
    void* user_data                    = ( void* )( intptr_t )2;

    xi_timed_task_handle_t task_handle =
        xi_add_timed_task( container, dispatcher, context_handle,
                           &xi_utest_timed_task_callback, 0, 0, user_data );

    tt_want_int_op( context_handle, !=, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle, !=, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data, !=, xi_utest_timed_task_user_data );

    xi_evtd_step( dispatcher, 1 );
    xi_remove_timed_task( container, task_handle );

    tt_want_int_op( context_handle, ==, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle, ==, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data, ==, xi_utest_timed_task_user_data );

    xi_destroy_timed_task_container( container );
    xi_evtd_destroy_instance( dispatcher );
    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
end:;
} )


XI_TT_TESTCASE( utest__remove_timed_task__wont_call_callback, {
    xi_utest_timed_task_reset_globals();
    xi_evtd_instance_t* dispatcher       = xi_evtd_create_instance();
    xi_timed_task_container_t* container = xi_make_timed_task_container();

    xi_context_handle_t context_handle = 1;
    void* user_data                    = ( void* )( intptr_t )2;

    xi_timed_task_handle_t task_handle =
        xi_add_timed_task( container, dispatcher, context_handle,
                           &xi_utest_timed_task_callback, 0, 0, user_data );

    tt_want_int_op( context_handle, !=, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle, !=, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data, !=, xi_utest_timed_task_user_data );

    xi_remove_timed_task( container, task_handle );
    xi_evtd_step( dispatcher, 1 );

    tt_want_int_op( context_handle, !=, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle, !=, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data, !=, xi_utest_timed_task_user_data );

    xi_destroy_timed_task_container( container );
    xi_evtd_destroy_instance( dispatcher );
    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
end:;
} )

XI_TT_TESTCASE( utest__remove_timed_task__one_invocation_only_removes_one_task, {
    xi_utest_timed_task_reset_globals();
    xi_evtd_instance_t* dispatcher       = xi_evtd_create_instance();
    xi_timed_task_container_t* container = xi_make_timed_task_container();

    xi_context_handle_t context_handle_1 = 1;
    void* user_data_1                    = ( void* )( intptr_t )2;
    xi_timed_task_handle_t task_handle_1 =
        xi_add_timed_task( container, dispatcher, context_handle_1,
                           &xi_utest_timed_task_callback, 0, 0, user_data_1 );

    xi_context_handle_t context_handle_2 = 3;
    void* user_data_2                    = ( void* )( intptr_t )4;
    xi_timed_task_handle_t task_handle_2 =
        xi_add_timed_task( container, dispatcher, context_handle_2,
                           &xi_utest_timed_task_callback, 0, 0, user_data_2 );

    tt_want_int_op( context_handle_1, !=, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle_1, !=, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data_1, !=, xi_utest_timed_task_user_data );
    tt_want_int_op( context_handle_2, !=, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle_2, !=, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data_2, !=, xi_utest_timed_task_user_data );

    xi_remove_timed_task( container, task_handle_1 );
    xi_evtd_step( dispatcher, 1 );

    tt_want_int_op( context_handle_2, ==, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle_2, ==, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data_2, ==, xi_utest_timed_task_user_data );

    xi_destroy_timed_task_container( container );
    xi_evtd_destroy_instance( dispatcher );
    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
end:;
} )

// ------------------------------ RECURRING TIMED TASK TESTS
// ------------------------------ //

XI_TT_TESTCASE( utest__recurring_timed_task__check_recurring_callbacks_by_1_timehop, {
    xi_utest_timed_task_reset_globals();
    xi_evtd_instance_t* dispatcher       = xi_evtd_create_instance();
    xi_timed_task_container_t* container = xi_make_timed_task_container();

    xi_context_handle_t context_handle = 1;

    void* user_data = ( void* )( intptr_t )0x4242;
    xi_timed_task_handle_t task_handle =
        xi_add_timed_task( container, dispatcher, context_handle,
                           &xi_utest_timed_task_callback, 1, 1, user_data );

    tt_want_int_op( context_handle, !=, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle, !=, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data, !=, xi_utest_timed_task_user_data );

    xi_evtd_step( dispatcher, 0 );

    tt_want_int_op( context_handle, !=, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle, !=, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data, !=, xi_utest_timed_task_user_data );

    xi_evtd_step( dispatcher, 1 );

    tt_want_int_op( context_handle, ==, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle, ==, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data, ==, xi_utest_timed_task_user_data );

    xi_utest_timed_task_reset_globals();

    tt_want_int_op( context_handle, !=, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle, !=, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data, !=, xi_utest_timed_task_user_data );

    xi_evtd_step( dispatcher, 2 );

    tt_want_int_op( context_handle, ==, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle, ==, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data, ==, xi_utest_timed_task_user_data );

    xi_remove_timed_task( container, task_handle );
    xi_destroy_timed_task_container( container );
    xi_evtd_destroy_instance( dispatcher );
    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
end:;
} )

XI_TT_TESTCASE( utest__recurring_timed_task__check_recurring_callbacks_by_2_timehop, {
    xi_utest_timed_task_reset_globals();
    xi_evtd_instance_t* dispatcher       = xi_evtd_create_instance();
    xi_timed_task_container_t* container = xi_make_timed_task_container();

    xi_context_handle_t context_handle = 1;

    void* user_data = ( void* )( intptr_t )0x4242;
    xi_timed_task_handle_t task_handle =
        xi_add_timed_task( container, dispatcher, context_handle,
                           &xi_utest_timed_task_callback, 2, 1, user_data );

    tt_want_int_op( context_handle, !=, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle, !=, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data, !=, xi_utest_timed_task_user_data );

    xi_evtd_step( dispatcher, 0 );

    tt_want_int_op( context_handle, !=, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle, !=, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data, !=, xi_utest_timed_task_user_data );

    xi_evtd_step( dispatcher, 1 );

    tt_want_int_op( context_handle, !=, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle, !=, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data, !=, xi_utest_timed_task_user_data );

    xi_evtd_step( dispatcher, 2 );

    tt_want_int_op( context_handle, ==, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle, ==, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data, ==, xi_utest_timed_task_user_data );

    xi_utest_timed_task_reset_globals();

    tt_want_int_op( context_handle, !=, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle, !=, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data, !=, xi_utest_timed_task_user_data );

    xi_evtd_step( dispatcher, 3 );

    tt_want_int_op( context_handle, !=, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle, !=, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data, !=, xi_utest_timed_task_user_data );

    xi_evtd_step( dispatcher, 4 );

    tt_want_int_op( context_handle, ==, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle, ==, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data, ==, xi_utest_timed_task_user_data );

    xi_remove_timed_task( container, task_handle );
    xi_destroy_timed_task_container( container );
    xi_evtd_destroy_instance( dispatcher );
    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
end:;
} )

XI_TT_TESTCASE( utest__remove_recurring_timed_task__wont_call_callback_after_remove, {
    xi_utest_timed_task_reset_globals();
    xi_evtd_instance_t* dispatcher       = xi_evtd_create_instance();
    xi_timed_task_container_t* container = xi_make_timed_task_container();

    xi_context_handle_t context_handle = 1;

    void* user_data = ( void* )( intptr_t )0x4242;
    xi_timed_task_handle_t task_handle =
        xi_add_timed_task( container, dispatcher, context_handle,
                           &xi_utest_timed_task_callback, 1, 1, user_data );

    xi_evtd_step( dispatcher, 0 );

    tt_want_int_op( context_handle, !=, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle, !=, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data, !=, xi_utest_timed_task_user_data );

    xi_evtd_step( dispatcher, 1 );

    tt_want_int_op( context_handle, ==, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle, ==, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data, ==, xi_utest_timed_task_user_data );

    xi_utest_timed_task_reset_globals();

    tt_want_int_op( context_handle, !=, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle, !=, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data, !=, xi_utest_timed_task_user_data );

    xi_evtd_step( dispatcher, 2 );

    tt_want_int_op( context_handle, ==, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle, ==, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data, ==, xi_utest_timed_task_user_data );

    xi_utest_timed_task_reset_globals();

    tt_want_int_op( context_handle, !=, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle, !=, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data, !=, xi_utest_timed_task_user_data );

    xi_remove_timed_task( container, task_handle );
    xi_evtd_step( dispatcher, 3 );

    tt_want_int_op( context_handle, !=, xi_utest_timed_task_last_context_handle );
    tt_want_int_op( task_handle, !=, xi_utest_timed_task_last_timed_task_handle );
    tt_want_ptr_op( user_data, !=, xi_utest_timed_task_user_data );

    xi_remove_timed_task( container, task_handle );
    xi_destroy_timed_task_container( container );
    xi_evtd_destroy_instance( dispatcher );
    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
end:;
} )

XI_TT_TESTCASE(
    utest__remove_timed_task_inside_the_callback__wont_call_callback_after_remove, {
        xi_utest_timed_task_reset_globals();
        xi_evtd_instance_t* dispatcher       = xi_evtd_create_instance();
        xi_timed_task_container_t* container = xi_make_timed_task_container();

        xi_context_handle_t context_handle = 1;

        void* user_data                    = ( void* )container;
        xi_timed_task_handle_t task_handle = xi_add_timed_task(
            container, dispatcher, context_handle,
            &xi_utest_timed_task_callback_remove_timed_task, 1, 1, user_data );

        xi_evtd_step( dispatcher, 0 );

        tt_want_int_op( context_handle, !=, xi_utest_timed_task_last_context_handle );
        tt_want_int_op( task_handle, !=, xi_utest_timed_task_last_timed_task_handle );
        tt_want_ptr_op( user_data, !=, xi_utest_timed_task_user_data );

        xi_evtd_step( dispatcher, 1 );

        tt_want_int_op( context_handle, ==, xi_utest_timed_task_last_context_handle );
        tt_want_int_op( task_handle, ==, xi_utest_timed_task_last_timed_task_handle );
        tt_want_ptr_op( user_data, ==, xi_utest_timed_task_user_data );

        xi_utest_timed_task_reset_globals();

        tt_want_int_op( context_handle, !=, xi_utest_timed_task_last_context_handle );
        tt_want_int_op( task_handle, !=, xi_utest_timed_task_last_timed_task_handle );
        tt_want_ptr_op( user_data, !=, xi_utest_timed_task_user_data );

        xi_evtd_step( dispatcher, 2 );

        tt_want_int_op( context_handle, !=, xi_utest_timed_task_last_context_handle );
        tt_want_int_op( task_handle, !=, xi_utest_timed_task_last_timed_task_handle );
        tt_want_ptr_op( user_data, !=, xi_utest_timed_task_user_data );

        xi_remove_timed_task( container, task_handle );
        xi_destroy_timed_task_container( container );
        xi_evtd_destroy_instance( dispatcher );
        tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
    end:;
    } )


XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
