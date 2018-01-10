/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_thread_posix_workerthread.h"
#include "xi_list.h"

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#include "../../../xi_utest_thread_util_actions.h"

void xi_utest_local__add_handler__workerthread( xi_evtd_instance_t* evtd,
                                                xi_event_handle_func_argc1 fn,
                                                xi_event_handle_arg1_t* arg1,
                                                size_t add_handler_multiplicity )
{
    if ( evtd == NULL )
        return;

    xi_event_handle_t evtd_handle = {
        XI_EVENT_HANDLE_ARGC1, .handlers.h1 = {fn, ( xi_event_handle_arg1_t )arg1}, 0};

    size_t counter_added = 0;
    for ( ; counter_added < add_handler_multiplicity; ++counter_added )
    {
        xi_evtd_execute( evtd, evtd_handle );
    }
}

#endif // XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

XI_TT_TESTCASE(
    utest__posix__xi_workerthread_create_instance__no_input__fields_are_initialized, {
        xi_evtd_instance_t* evtd                   = xi_evtd_create_instance();
        xi_evtd_instance_t* test_cases_evtd_ptrs[] = {( xi_evtd_instance_t* )NULL, evtd};

        uint8_t counter_testcase = 0;
        for ( ; counter_testcase < XI_ARRAYSIZE( test_cases_evtd_ptrs );
              ++counter_testcase )
        {
            xi_workerthread_t* new_workerthread_instance =
                xi_workerthread_create_instance( test_cases_evtd_ptrs[counter_testcase] );

            tt_ptr_op( NULL, !=, new_workerthread_instance );
            tt_ptr_op( NULL, !=, new_workerthread_instance->thread_evtd );
            tt_ptr_op( test_cases_evtd_ptrs[counter_testcase], ==,
                       new_workerthread_instance->thread_evtd_secondary );

            xi_workerthread_destroy_instance( &new_workerthread_instance );

            tt_ptr_op( NULL, ==, new_workerthread_instance );
        }

    end:
        xi_evtd_destroy_instance( evtd );

        return;
    } )

XI_TT_TESTCASE(
    utest__posix__xi_workerthread__add_onetype_handler_multiple_times__all_handlers_are_executed,
    {
        const size_t nb_handler_additions[] = {1, 2, 23, 124, 0};

        xi_init_critical_section( &xi_uteset_local_action_store_cs );

        size_t id_handler_adds = 0;
        for ( ; id_handler_adds < XI_ARRAYSIZE( nb_handler_additions ) - 4;
              ++id_handler_adds )
        {
            xi_workerthread_t* workerthread = xi_workerthread_create_instance( NULL );

            // const uint8_t sync_point_reached =
            // xi_workerthread_wait_sync_point(workerthread);
            // tt_want_int_op(1, ==, sync_point_reached);

            uint32_t value_shared_between_threads = 111;

            xi_utest_local__add_handler__workerthread(
                workerthread->thread_evtd, &xi_utest_local_action_increase_by_one,
                ( xi_event_handle_arg1_t )&value_shared_between_threads,
                nb_handler_additions[id_handler_adds] );

            xi_workerthread_destroy_instance( &workerthread );

#if 0
		printf("current number of handler additions: %lu, result = %u\n"
			, nb_handler_additions[id_handler_adds]
			, value_shared_between_threads);
#endif
            tt_want_int_op( 111 + nb_handler_additions[id_handler_adds], ==,
                            value_shared_between_threads );
        }

        xi_destroy_critical_section( &xi_uteset_local_action_store_cs );
    } )

XI_TT_TESTCASE(
    utest__posix__xi_workerthread__add_twotypes_handler_multiple_times__all_handlers_are_executed,
    {
        const size_t nb_handler_additions_A[] = {67, 4, 3, 7, 1, 0};
        const size_t nb_handler_additions_B[] = {7, 4, 22,
                                                 0, 6, 7}; // keep arrays same sized!

        xi_init_critical_section( &xi_uteset_local_action_store_cs );

        size_t id_handler_adds = 0;
        for ( ; id_handler_adds < XI_ARRAYSIZE( nb_handler_additions_A ) - 5;
              ++id_handler_adds )
        {
            xi_workerthread_t* workerthread = xi_workerthread_create_instance( NULL );

            // const uint8_t sync_point_reached =
            // xi_workerthread_wait_sync_point(workerthread);
            // tt_want_int_op(1, ==, sync_point_reached);

            uint32_t value_shared_between_threads = 77777;

            xi_utest_local__add_handler__workerthread(
                workerthread->thread_evtd, &xi_utest_local_action_increase_by_one,
                ( xi_event_handle_arg1_t )&value_shared_between_threads,
                nb_handler_additions_A[id_handler_adds] );

            xi_utest_local__add_handler__workerthread(
                workerthread->thread_evtd, &xi_utest_local_action_decrease_by_11,
                ( xi_event_handle_arg1_t )&value_shared_between_threads,
                nb_handler_additions_B[id_handler_adds] );

            xi_workerthread_destroy_instance( &workerthread );

#if 0
		printf("current number of handler additions: %lu - %lu, result = %u\n"
			, nb_handler_additions_A[id_handler_adds]
			, nb_handler_additions_B[id_handler_adds]
			, value_shared_between_threads);
#endif

            tt_want_int_op( 77777 + nb_handler_additions_A[id_handler_adds] -
                                11 * nb_handler_additions_B[id_handler_adds],
                            ==, value_shared_between_threads );
        }

        xi_destroy_critical_section( &xi_uteset_local_action_store_cs );
    } )

XI_TT_TESTCASE(
    utest__posix__xi_workerthread__usageof_secondary_evtd__one_or_two_handlers_are_executed,
    {
        const size_t nb_handler_additions[] = {0, 1, 2, 5, 11};

        xi_init_critical_section( &xi_uteset_local_action_store_cs );

        size_t id_handler_adds = 0;
        for ( ; id_handler_adds <
                ( xi_test_load_level ? XI_ARRAYSIZE( nb_handler_additions ) : 3 );
              ++id_handler_adds )
        {
            xi_evtd_instance_t* evtd        = xi_evtd_create_instance();
            xi_workerthread_t* workerthread = xi_workerthread_create_instance( evtd );

            // const uint8_t sync_point_reached =
            // xi_workerthread_wait_sync_point(workerthread);
            // tt_want_int_op(1, ==, sync_point_reached);

            uint32_t value_shared_between_threads = 1111;

            xi_utest_local__add_handler__workerthread(
                workerthread->thread_evtd_secondary,
                &xi_utest_local_action_decrease_by_11,
                ( xi_event_handle_arg1_t )&value_shared_between_threads,
                nb_handler_additions[id_handler_adds] );

            while ( !XI_LIST_EMPTY( xi_event_handle_t,
                                    workerthread->thread_evtd_secondary->call_queue ) )
            {
                XI_TIME_MILLISLEEP( 10, deltatime );
            }

            xi_workerthread_destroy_instance( &workerthread );
            xi_evtd_destroy_instance( evtd );

#if 0
		printf("current number of handler additions: %lu, result = %u\n"
			, nb_handler_additions[id_handler_adds]
			, value_shared_between_threads);
#endif
            tt_want_int_op( 1111 - nb_handler_additions[id_handler_adds] * 11, ==,
                            value_shared_between_threads );
        }

        xi_destroy_critical_section( &xi_uteset_local_action_store_cs );
    } )

XI_TT_TESTCASE(
    utest__posix__xi_workerthread__usageof_both_evtds__handlers_are_executed_properly, {
        const size_t nb_handler_additions[] = {0, 2, 5, 9};

        xi_init_critical_section( &xi_uteset_local_action_store_cs );

        size_t id_handler_adds = 0;
        for ( ; id_handler_adds <
                ( xi_test_load_level ? XI_ARRAYSIZE( nb_handler_additions ) : 2 );
              ++id_handler_adds )
        {
            xi_evtd_instance_t* evtd        = xi_evtd_create_instance();
            xi_workerthread_t* workerthread = xi_workerthread_create_instance( evtd );

            // const uint8_t sync_point_reached =
            // xi_workerthread_wait_sync_point(workerthread);
            // tt_want_int_op(1, ==, sync_point_reached);

            uint32_t value_shared_between_threads = 1111;

            xi_utest_local__add_handler__workerthread(
                workerthread->thread_evtd_secondary,
                &xi_utest_local_action_decrease_by_11,
                ( xi_event_handle_arg1_t )&value_shared_between_threads,
                nb_handler_additions[id_handler_adds] );

            xi_utest_local__add_handler__workerthread(
                workerthread->thread_evtd, &xi_utest_local_action_increase_by_one,
                ( xi_event_handle_arg1_t )&value_shared_between_threads,
                nb_handler_additions[id_handler_adds] );

            while ( !XI_LIST_EMPTY( xi_event_handle_t,
                                    workerthread->thread_evtd_secondary->call_queue ) )
            {
                XI_TIME_MILLISLEEP( 10, deltatime );
            }

            xi_workerthread_destroy_instance( &workerthread );
            xi_evtd_destroy_instance( evtd );

#if 0
		printf("current number of handler additions: %lu, result = %u\n"
			, nb_handler_additions[id_handler_adds]
			, value_shared_between_threads);
#endif
            tt_want_int_op( 1111 + ( 1 - 11 ) * nb_handler_additions[id_handler_adds], ==,
                            value_shared_between_threads );
        }

        xi_destroy_critical_section( &xi_uteset_local_action_store_cs );
    } )
