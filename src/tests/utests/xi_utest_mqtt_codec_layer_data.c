/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"

#include "xively.h"
#include "xi_err.h"
#include "xi_helpers.h"
#include "xi_data_desc.h"
#include "xi_mqtt_logic_layer_data_helpers.h"
#include "xi_mqtt_codec_layer_data.h"

#include "xi_memory_checks.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#endif

XI_TT_TESTGROUP_BEGIN( utest_mqtt_codec_layer_data )

XI_TT_TESTCASE( utest__xi_mqtt_codec_layer_make_task__valid_data__new_task_created, {

    xi_state_t state = XI_STATE_OK;

    xi_mqtt_message_t* msg = NULL;

    XI_ALLOC_AT( xi_mqtt_message_t, msg, state );

    state = fill_with_pingreq_data( msg );

    XI_CHECK_STATE( state );

    xi_mqtt_codec_layer_task_t* task = xi_mqtt_codec_layer_make_task( msg );

    tt_ptr_op( NULL, !=, task );
    tt_ptr_op( task->msg, ==, msg );

    xi_mqtt_codec_layer_free_task( &task );

    tt_ptr_op( NULL, ==, task );

    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );

    return;
err_handling:
    tt_fail();
end:;
} )

XI_TT_TESTCASE( utest__xi_mqtt_codec_layer_activate_task__valid_data__msg_zeroed, {
    xi_state_t state = XI_STATE_OK;

    xi_mqtt_message_t* msg = NULL;

    XI_ALLOC_AT( xi_mqtt_message_t, msg, state );

    state = fill_with_pingreq_data( msg );

    XI_CHECK_STATE( state );

    xi_mqtt_codec_layer_task_t* task = xi_mqtt_codec_layer_make_task( msg );

    xi_mqtt_message_t* detached_msg = xi_mqtt_codec_layer_activate_task( task );

    tt_ptr_op( msg, ==, detached_msg );
    tt_ptr_op( NULL, ==, task->msg );

    xi_mqtt_codec_layer_free_task( &task );
    xi_mqtt_message_free( &detached_msg );

    tt_ptr_op( NULL, ==, task );

    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );

    return;

err_handling:
    tt_fail();
end:;
} )

XI_TT_TESTCASE( utest__xi_mqtt_codec_layer_continue_task__valid_data__msg_restored, {
    xi_state_t state = XI_STATE_OK;

    xi_mqtt_message_t* msg = NULL;

    XI_ALLOC_AT( xi_mqtt_message_t, msg, state );

    state = fill_with_pingreq_data( msg );

    XI_CHECK_STATE( state );

    xi_mqtt_codec_layer_task_t* task = xi_mqtt_codec_layer_make_task( msg );

    xi_mqtt_message_t* detached_msg = xi_mqtt_codec_layer_activate_task( task );

    tt_ptr_op( msg, ==, detached_msg );
    tt_ptr_op( NULL, ==, task->msg );

    xi_mqtt_codec_layer_continue_task( task, detached_msg );

    tt_ptr_op( task->msg, ==, detached_msg );

    xi_mqtt_codec_layer_free_task( &task );

    tt_ptr_op( NULL, ==, task );

    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );

    return;

err_handling:
    tt_fail();
end:;
} )

XI_TT_TESTCASE( utest__xi_mqtt_codec_layer_free_task__valid_data__task_released, {
    xi_state_t state = XI_STATE_OK;

    xi_mqtt_message_t* msg = NULL;

    XI_ALLOC_AT( xi_mqtt_message_t, msg, state );

    state = fill_with_pingreq_data( msg );

    XI_CHECK_STATE( state );

    xi_mqtt_codec_layer_task_t* task = xi_mqtt_codec_layer_make_task( msg );

    xi_mqtt_codec_layer_free_task( &task );

    tt_ptr_op( NULL, ==, task );

    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );

    return;

err_handling:
    tt_fail();
end:;
} )

XI_TT_TESTCASE( utest__xi_mqtt_codec_layer_free_task__valid_data_no_msg__task_released, {
    xi_state_t state = XI_STATE_OK;

    xi_mqtt_message_t* msg = NULL;

    XI_ALLOC_AT( xi_mqtt_message_t, msg, state );

    state = fill_with_pingreq_data( msg );

    XI_CHECK_STATE( state );

    xi_mqtt_codec_layer_task_t* task = xi_mqtt_codec_layer_make_task( msg );

    xi_mqtt_message_t* detached_msg = xi_mqtt_codec_layer_activate_task( task );

    xi_mqtt_message_free( &detached_msg );
    xi_mqtt_codec_layer_free_task( &task );

    tt_ptr_op( NULL, ==, task );

    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );

    return;

err_handling:
    tt_fail();
end:;
} )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
