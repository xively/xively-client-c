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
#include "xi_globals.h"
#include "xi_mqtt_logic_layer_subscribe_command.h"
#include "xi_mqtt_logic_layer_data.h"
#include "xi_mqtt_message.h"
#include "xi_handle.h"
#include "xi_memory_checks.h"
#include "xi_user_sub_call_wrapper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

static uint8_t global_value_to_test = 0;

static xi_state_t successful_subscribe_handler( xi_context_handle_t in_context_handle,
                                                xi_sub_call_type_t call_type,
                                                const xi_sub_call_params_t* const params,
                                                xi_state_t state,
                                                void* user_data )
{
    XI_UNUSED( in_context_handle );
    XI_UNUSED( params );
    XI_UNUSED( state );
    XI_UNUSED( user_data );

    tt_want_int_op( call_type, ==, XI_SUB_CALL_SUBACK );
    tt_want_int_op( state, ==, XI_MQTT_SUBSCRIPTION_SUCCESSFULL );
    global_value_to_test = 1;

    return XI_STATE_OK;
}

static xi_state_t failed_subscribe_handler( xi_context_handle_t in_context_handle,
                                            xi_sub_call_type_t call_type,
                                            const xi_sub_call_params_t* const params,
                                            xi_state_t state,
                                            void* user_data )
{
    XI_UNUSED( in_context_handle );
    XI_UNUSED( params );
    XI_UNUSED( state );
    XI_UNUSED( user_data );

    tt_want_int_op( call_type, ==, XI_SUB_CALL_SUBACK );
    tt_want_int_op( state, ==, XI_MQTT_SUBSCRIPTION_FAILED );
    global_value_to_test = 1;

    return XI_STATE_OK;
}

#endif

XI_TT_TESTGROUP_BEGIN( utest_mqtt_logic_layer_subscribe )

XI_TT_TESTCASE( utest__cmp_topics__valid_data__cmp_topics_should_return_0, {
    xi_event_handle_t handle = xi_make_empty_handle();

    char* string1 = "test_string";
    char* string2 = "test_string";

    size_t string_len = strlen( string1 );

    xi_mqtt_task_specific_data_t spd = {.subscribe = {string2, handle, 0}};
    xi_data_desc_t data = {
        ( unsigned char* )string1, NULL, string_len, string_len, string_len,
        XI_MEMORY_TYPE_UNMANAGED};

    union xi_vector_selector_u a = {&spd};
    union xi_vector_selector_u b = {&data};

    tt_want_int_op( cmp_topics( &a, &b ), ==, 0 );
} )

XI_TT_TESTCASE( utest__cmp_topics__valid_data__cmp_topics_should_return_1, {
    xi_event_handle_t handle = xi_make_empty_handle();

    char* string1 = "test_string1";
    char* string2 = "test_string2";

    size_t string_len = strlen( string1 );

    xi_mqtt_task_specific_data_t spd = {.subscribe = {string2, handle, 0}};
    xi_data_desc_t data = {
        ( unsigned char* )string1, NULL, string_len, string_len, string_len,
        XI_MEMORY_TYPE_UNMANAGED};

    union xi_vector_selector_u a = {&spd};
    union xi_vector_selector_u b = {&data};

    tt_want_int_op( cmp_topics( &a, &b ), ==, 1 );
} )

XI_TT_TESTCASE_WITH_SETUP(
    utest__do_mqtt_subscribe__valid_data__subscription_handler_registered_with_success,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_state_t local_state = XI_STATE_OK;

        xi_context_handle_t xi_context_handle = xi_create_context();
        if ( XI_INVALID_CONTEXT_HANDLE >= xi_context_handle )
        {
            tt_fail_msg( "Failed to create context!" );
            return;
        }

        xi_mqtt_logic_task_t* task = 0;
        xi_mqtt_message_t* msg     = 0;

        // take the pointer to context
        xi_context_t* xi_context =
            xi_object_for_handle( xi_globals.context_handles_vector, xi_context_handle );
        tt_assert( NULL != xi_context );

        // set the task data
        XI_ALLOC_AT( xi_mqtt_logic_task_t, task, local_state );

        task->cs = 111; // this is very hakish since it depends on the code
        // so most probably this test will fail everytime we change anything in
        // tested function which is not too good at least you know what to check
        // if the test fails

        xi_evtd_execute_in(
            xi_globals.evtd_instance,
            xi_make_handle( &do_mqtt_subscribe, 0, &task, XI_STATE_TIMEOUT, 0 ), 10,
            &task->timeout );

        task->data.mqtt_settings.scenario = XI_MQTT_SUBSCRIBE;
        task->data.mqtt_settings.qos      = XI_MQTT_QOS_AT_LEAST_ONCE;

        XI_ALLOC_AT( xi_mqtt_task_specific_data_t, task->data.data_u, local_state );
        xi_mqtt_task_specific_data_t* data_u = task->data.data_u;

        task->data.data_u->subscribe.handler = xi_make_threaded_handle(
            XI_THREADID_THREAD_0, &xi_user_sub_call_wrapper, xi_context, NULL,
            XI_STATE_OK, ( void* )&successful_subscribe_handler, ( void* )NULL,
            ( void* )task->data.data_u );

        // set the msg data
        XI_ALLOC_AT( xi_mqtt_message_t, msg, local_state );

        msg->common.common_u.common_bits.type = XI_MQTT_TYPE_SUBACK;
        XI_ALLOC_AT( xi_mqtt_topicpair_t, msg->suback.topics, local_state );
        msg->suback.topics->xi_mqtt_topic_pair_payload_u.status = XI_MQTT_QOS_0_GRANTED;

        // set the layer data
        xi_mqtt_logic_layer_data_t logic_layer_data;
        memset( &logic_layer_data, 0, sizeof( xi_mqtt_logic_layer_data_t ) );

        logic_layer_data.q12_tasks_queue = task;

        logic_layer_data.handlers_for_topics = xi_vector_create();

        xi_layer_t* layer = xi_context->layer_chain.bottom;
        layer->user_data  = &logic_layer_data;

        // data prepared time to call the function
        tt_want_int_op(
            do_mqtt_subscribe( &layer->layer_connection, task, XI_STATE_OK, msg ), ==,
            XI_STATE_OK );
        tt_want_int_op( logic_layer_data.handlers_for_topics->elem_no, ==, 1 );
        tt_want_ptr_op(
            logic_layer_data.handlers_for_topics->array[0].selector_t.ptr_value, ==,
            data_u );

        // make the handler to be called
        xi_evtd_step( xi_globals.evtd_instance, 20 );

        tt_want_int_op( global_value_to_test, ==, 1 );
        global_value_to_test = 0;

        XI_SAFE_FREE( data_u );
        xi_vector_del( logic_layer_data.handlers_for_topics, 0 );

        logic_layer_data.handlers_for_topics =
            xi_vector_destroy( logic_layer_data.handlers_for_topics );

        xi_delete_context( xi_context_handle );

        return;

    err_handling:
        tt_abort_msg( "test should not fail" );
    end:
        if ( task != 0 )
            xi_mqtt_logic_free_task( &task );
        xi_mqtt_message_free( &msg );
        xi_delete_context( xi_context_handle );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__do_mqtt_subscribe__valid_data__subscription_handler_registered_with_failure,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_state_t local_state = XI_STATE_OK;

        xi_context_handle_t xi_context_handle = xi_create_context();
        if ( XI_INVALID_CONTEXT_HANDLE >= xi_context_handle )
        {
            tt_fail_msg( "Failed to create default context!" );
            return;
        }

        xi_mqtt_logic_task_t* task = 0;
        xi_mqtt_message_t* msg     = 0;

        // take the pointer to context
        xi_context_t* xi_context =
            xi_object_for_handle( xi_globals.context_handles_vector, xi_context_handle );
        tt_assert( NULL != xi_context );

        // set the task data
        XI_ALLOC_AT( xi_mqtt_logic_task_t, task, local_state );

        task->cs = 111; // this is very hakish since it depends on the code
        // so most probably this test will fail everytime we change anything in
        // tested function which is not too good at least you know what to check
        // if the test fails

        xi_evtd_execute_in(
            xi_globals.evtd_instance,
            xi_make_handle( &do_mqtt_subscribe, 0, &task, XI_STATE_TIMEOUT, 0 ), 10,
            &task->timeout );

        task->data.mqtt_settings.scenario = XI_MQTT_SUBSCRIBE;
        task->data.mqtt_settings.qos      = XI_MQTT_QOS_AT_LEAST_ONCE;

        XI_ALLOC_AT( xi_mqtt_task_specific_data_t, task->data.data_u, local_state );

        task->data.data_u->subscribe.handler = xi_make_threaded_handle(
            XI_THREADID_THREAD_0, &xi_user_sub_call_wrapper, xi_context, NULL,
            XI_STATE_OK, ( void* )&failed_subscribe_handler, ( void* )NULL,
            ( void* )task->data.data_u );

        // set the msg data
        XI_ALLOC_AT( xi_mqtt_message_t, msg, local_state );

        msg->common.common_u.common_bits.type = XI_MQTT_TYPE_SUBACK;
        XI_ALLOC_AT( xi_mqtt_topicpair_t, msg->suback.topics, local_state );
        msg->suback.topics->xi_mqtt_topic_pair_payload_u.status = XI_MQTT_SUBACK_FAILED;

        // set the layer data
        xi_mqtt_logic_layer_data_t logic_layer_data;
        memset( &logic_layer_data, 0, sizeof( xi_mqtt_logic_layer_data_t ) );

        logic_layer_data.q12_tasks_queue = task;

        logic_layer_data.handlers_for_topics = xi_vector_create();

        xi_layer_t* layer = xi_context->layer_chain.bottom;
        layer->user_data  = &logic_layer_data;

        // data prepared time to call the function

        tt_want_int_op(
            do_mqtt_subscribe( &layer->layer_connection, task, XI_STATE_OK, msg ), ==,
            XI_STATE_OK );
        tt_want_int_op( logic_layer_data.handlers_for_topics->elem_no, ==, 0 );

        // make the handler to be called
        xi_evtd_step( xi_globals.evtd_instance, 20 );

        tt_want_int_op( global_value_to_test, ==, 1 );
        global_value_to_test = 0;

        xi_delete_context( xi_context_handle );
        logic_layer_data.handlers_for_topics =
            xi_vector_destroy( logic_layer_data.handlers_for_topics );

        return;

    err_handling:
        tt_abort_msg( "test should not fail" );
    end:
        if ( task != 0 )
            xi_mqtt_logic_free_task( &task );
        xi_mqtt_message_free( &msg );
        xi_delete_context( xi_context_handle );
        tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
    } )
XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
