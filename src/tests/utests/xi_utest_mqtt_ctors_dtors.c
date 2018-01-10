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
#include "xi_mqtt_message.h"
#include "xi_mqtt_logic_layer_data_helpers.h"

#include "xi_memory_checks.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

typedef struct xi_utest_mqtt_message_details_s
{
    unsigned qos : 2;
    unsigned retain : 1;
    unsigned dup : 1;
} xi_utest_mqtt_message_details_t;

typedef struct xi_utest_mqtt_message_details_to_uint16_t_s
{
    union {
        xi_utest_mqtt_message_details_t qos_retain_dup;
        uint8_t value;
    } xi_mqtt_union;
} xi_utest_mqtt_message_details_to_uint16_t;

static void utest__fill_with_pingreq_data__valid_data__pingreq_msg_help( void )
{
    xi_state_t local_state = XI_STATE_OK;

    xi_mqtt_message_t* msg_matrix = NULL;
    xi_mqtt_message_t* msg        = NULL;

    XI_ALLOC_AT( xi_mqtt_message_t, msg, local_state );
    XI_ALLOC_AT( xi_mqtt_message_t, msg_matrix, local_state );

    tt_want_int_op( fill_with_pingreq_data( msg ), ==, XI_STATE_OK );

    msg_matrix->common.common_u.common_bits.type = XI_MQTT_TYPE_PINGREQ;
    tt_want_int_op( memcmp( msg, msg_matrix, sizeof( xi_mqtt_message_t ) ), ==, 0 );

    xi_mqtt_message_free( &msg );
    xi_mqtt_message_free( &msg_matrix );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

    return;

err_handling:
    tt_abort_msg( ( "test must not fail!" ) );
end:
    xi_mqtt_message_free( &msg );
    xi_mqtt_message_free( &msg_matrix );
    ;
}

static void utest__fill_with_publish_data__valid_data_max_payload__publish_msg_help()
{
    xi_state_t local_state = XI_STATE_OK;

    xi_mqtt_message_t* msg        = NULL;
    xi_mqtt_message_t* msg_matrix = NULL;

    const size_t cnt_size = XI_MQTT_MAX_PAYLOAD_SIZE;

    const char topic[] = "test_topic";

    xi_data_desc_t* content = xi_make_empty_desc_alloc( cnt_size );
    memset( ( void* )content->data_ptr, 'a', cnt_size );
    content->length = cnt_size;

    // test for all combinations of common
    size_t i = 0;
    for ( i = 0; i < 16; ++i )
    {
        XI_ALLOC_AT( xi_mqtt_message_t, msg, local_state );
        XI_ALLOC_AT( xi_mqtt_message_t, msg_matrix, local_state );

        xi_utest_mqtt_message_details_to_uint16_t mqtt_common = {{.value = i}};

        tt_want_int_op(
            fill_with_publish_data( msg, topic, content,
                                    mqtt_common.xi_mqtt_union.qos_retain_dup.qos,
                                    mqtt_common.xi_mqtt_union.qos_retain_dup.retain,
                                    mqtt_common.xi_mqtt_union.qos_retain_dup.dup, 17 ),
            ==, XI_STATE_OK );

        msg_matrix->common.common_u.common_bits.retain =
            mqtt_common.xi_mqtt_union.qos_retain_dup.retain;
        msg_matrix->common.common_u.common_bits.qos =
            mqtt_common.xi_mqtt_union.qos_retain_dup.qos;
        msg_matrix->common.common_u.common_bits.dup =
            mqtt_common.xi_mqtt_union.qos_retain_dup.dup;
        msg_matrix->common.common_u.common_bits.type = XI_MQTT_TYPE_PUBLISH;
        msg_matrix->common.remaining_length          = 0;

        msg_matrix->publish.topic_name = msg->publish.topic_name;
        msg_matrix->publish.content    = msg->publish.content;

        tt_want_int_op(
            memcmp( msg->publish.topic_name->data_ptr, topic, sizeof( topic ) - 1 ), ==,
            0 );
        tt_want_int_op(
            memcmp( msg->publish.content->data_ptr, content->data_ptr, content->length ),
            ==, 0 );

        msg_matrix->publish.message_id = 17;

        tt_want_int_op( memcmp( msg, msg_matrix, sizeof( xi_mqtt_message_t ) ), ==, 0 );

        msg_matrix->publish.topic_name = 0;
        msg_matrix->publish.content    = 0;

        xi_mqtt_message_free( &msg );
        xi_mqtt_message_free( &msg_matrix );
    }

    xi_free_desc( &content );
    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

    return;

err_handling:
    tt_abort_msg( ( "test must not fail!" ) );
end:
    xi_mqtt_message_free( &msg );
    xi_mqtt_message_free( &msg_matrix );
    xi_free_desc( &content );
}

#endif

XI_TT_TESTGROUP_BEGIN( utest_mqtt_ctors_dtors )

XI_TT_TESTCASE( utest__fill_with_pingreq_data__valid_data__pingreq_msg,
                { utest__fill_with_pingreq_data__valid_data__pingreq_msg_help(); } )

XI_TT_TESTCASE( utest__fill_with_puback_data__valid_data__puback_msg, {
    xi_state_t local_state = XI_STATE_OK;

    xi_mqtt_message_t* msg        = NULL;
    xi_mqtt_message_t* msg_matrix = NULL;

    XI_ALLOC_AT( xi_mqtt_message_t, msg, local_state );
    XI_ALLOC_AT( xi_mqtt_message_t, msg_matrix, local_state );

    size_t i = 0;

    for ( ; i < 0xFFFF; ++i )
    {
        msg_matrix->common.common_u.common_bits.type = XI_MQTT_TYPE_PUBACK;
        msg_matrix->puback.message_id                = i;

        tt_want_int_op( fill_with_puback_data( msg, i ), ==, XI_STATE_OK );
        tt_want_int_op( memcmp( msg, msg_matrix, sizeof( xi_mqtt_message_t ) ), ==, 0 );
    }

    xi_mqtt_message_free( &msg );
    xi_mqtt_message_free( &msg_matrix );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

    return;

err_handling:
    tt_abort_msg( ( "test must not fail!" ) );
end:
    xi_mqtt_message_free( &msg );
    xi_mqtt_message_free( &msg_matrix );
} )

XI_TT_TESTCASE( utest__fill_with_connect_data__valid_data__connect_msg, {
    xi_state_t local_state = XI_STATE_OK;

    const char username[] = "test_username";
    const char password[] = "test_password";

    xi_mqtt_message_t* msg        = NULL;
    xi_mqtt_message_t* msg_matrix = NULL;

    XI_ALLOC_AT( xi_mqtt_message_t, msg, local_state );
    XI_ALLOC_AT( xi_mqtt_message_t, msg_matrix, local_state );

    tt_want_int_op( fill_with_connect_data( msg, username, password, 7, XI_SESSION_CLEAN,
                                            NULL, NULL, 0, 0 ),
                    ==, XI_STATE_OK );

    msg_matrix->common.common_u.common_bits.retain = XI_MQTT_RETAIN_FALSE;
    msg_matrix->common.common_u.common_bits.qos    = XI_MQTT_QOS_AT_MOST_ONCE;
    msg_matrix->common.common_u.common_bits.dup    = XI_MQTT_DUP_FALSE;
    msg_matrix->common.common_u.common_bits.type   = XI_MQTT_TYPE_CONNECT;

    msg_matrix->connect.protocol_version = 4;
    msg_matrix->connect.protocol_name    = msg->connect.protocol_name;

    msg_matrix->common.remaining_length = 0;

    tt_want_int_op(
        memcmp( msg->connect.protocol_name->data_ptr, "MQTT", sizeof( "MQTT" ) - 1 ), ==,
        0 );

    tt_want_int_op(
        memcmp( msg->connect.client_id->data_ptr, username, sizeof( username ) - 1 ), ==,
        0 );

    tt_want_int_op(
        memcmp( msg->connect.password->data_ptr, password, sizeof( password ) - 1 ), ==,
        0 );

    msg_matrix->connect.client_id = msg->connect.client_id;
    msg_matrix->connect.username  = msg->connect.username;
    msg_matrix->connect.password  = msg->connect.password;
    msg_matrix->connect.keepalive = msg->connect.keepalive;
    msg_matrix->connect.flags_u   = msg->connect.flags_u;

    tt_want_int_op( memcmp( msg, msg_matrix, sizeof( xi_mqtt_message_t ) ), ==, 0 );

    msg_matrix->connect.protocol_name = 0;
    msg_matrix->connect.client_id     = 0;
    msg_matrix->connect.username      = 0;
    msg_matrix->connect.password      = 0;

    xi_mqtt_message_free( &msg );
    xi_mqtt_message_free( &msg_matrix );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

    return;

err_handling:
    tt_abort_msg( ( "test must not fail!" ) );
end:
    xi_mqtt_message_free( &msg );
    xi_mqtt_message_free( &msg_matrix );
} )

XI_TT_TESTCASE(
    utest__fill_with_connect_data__valid_data_session_continuous__connect_msg, {
        xi_state_t local_state = XI_STATE_OK;

        const char username[] = "test_username";
        const char password[] = "test_password";

        xi_mqtt_message_t* msg        = NULL;
        xi_mqtt_message_t* msg_matrix = NULL;

        XI_ALLOC_AT( xi_mqtt_message_t, msg, local_state );
        XI_ALLOC_AT( xi_mqtt_message_t, msg_matrix, local_state );

        tt_want_int_op( fill_with_connect_data( msg, username, password, 7,
                                                XI_SESSION_CONTINUE, NULL, NULL, 0, 0 ),
                        ==, XI_STATE_OK );

        msg_matrix->common.common_u.common_bits.retain = XI_MQTT_RETAIN_FALSE;
        msg_matrix->common.common_u.common_bits.qos    = XI_MQTT_QOS_AT_MOST_ONCE;
        msg_matrix->common.common_u.common_bits.dup    = XI_MQTT_DUP_FALSE;
        msg_matrix->common.common_u.common_bits.type   = XI_MQTT_TYPE_CONNECT;

        msg_matrix->connect.protocol_version = 4;
        msg_matrix->connect.protocol_name    = msg->connect.protocol_name;

        msg_matrix->common.remaining_length = 0;

        tt_want_int_op(
            memcmp( msg->connect.protocol_name->data_ptr, "MQTT", sizeof( "MQTT" ) - 1 ),
            ==, 0 );

        tt_want_int_op(
            memcmp( msg->connect.client_id->data_ptr, username, sizeof( username ) - 1 ),
            ==, 0 );

        tt_want_int_op(
            memcmp( msg->connect.password->data_ptr, password, sizeof( password ) - 1 ),
            ==, 0 );

        msg_matrix->connect.client_id = msg->connect.client_id;
        msg_matrix->connect.username  = msg->connect.username;
        msg_matrix->connect.password  = msg->connect.password;
        msg_matrix->connect.keepalive = msg->connect.keepalive;
        msg_matrix->connect.flags_u   = msg->connect.flags_u;

        tt_want_int_op( memcmp( msg, msg_matrix, sizeof( xi_mqtt_message_t ) ), ==, 0 );

        msg_matrix->connect.protocol_name = 0;
        msg_matrix->connect.client_id     = 0;
        msg_matrix->connect.username      = 0;
        msg_matrix->connect.password      = 0;

        xi_mqtt_message_free( &msg );
        xi_mqtt_message_free( &msg_matrix );

        tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

        return;

    err_handling:
        tt_abort_msg( ( "test must not fail!" ) );
    end:
        xi_mqtt_message_free( &msg );
        xi_mqtt_message_free( &msg_matrix );
    } )

XI_TT_TESTCASE( utest__fill_with_connect_data__valid_data_no_password__connect_msg, {
    xi_state_t local_state = XI_STATE_OK;

    const char username[] = "test_username";
    const char* password  = 0;

    xi_mqtt_message_t* msg        = NULL;
    xi_mqtt_message_t* msg_matrix = NULL;

    XI_ALLOC_AT( xi_mqtt_message_t, msg, local_state );
    XI_ALLOC_AT( xi_mqtt_message_t, msg_matrix, local_state );

    tt_want_int_op( fill_with_connect_data( msg, username, password, 7, XI_SESSION_CLEAN,
                                            NULL, NULL, 0, 0 ),
                    ==, XI_STATE_OK );

    msg_matrix->common.common_u.common_bits.retain = XI_MQTT_RETAIN_FALSE;
    msg_matrix->common.common_u.common_bits.qos    = XI_MQTT_QOS_AT_MOST_ONCE;
    msg_matrix->common.common_u.common_bits.dup    = XI_MQTT_DUP_FALSE;
    msg_matrix->common.common_u.common_bits.type   = XI_MQTT_TYPE_CONNECT;

    msg_matrix->connect.protocol_version = 4;
    msg_matrix->connect.protocol_name    = msg->connect.protocol_name;

    msg_matrix->common.remaining_length = 0;

    tt_want_int_op(
        memcmp( msg->connect.protocol_name->data_ptr, "MQTT", sizeof( "MQTT" ) - 1 ), ==,
        0 );

    tt_want_int_op(
        memcmp( msg->connect.client_id->data_ptr, username, sizeof( username ) - 1 ), ==,
        0 );

    tt_want_int_op( msg->connect.password, ==, 0 );

    msg_matrix->connect.client_id = msg->connect.client_id;
    msg_matrix->connect.username  = msg->connect.username;
    msg_matrix->connect.password  = msg->connect.password;
    msg_matrix->connect.keepalive = msg->connect.keepalive;
    msg_matrix->connect.flags_u   = msg->connect.flags_u;

    tt_want_int_op( memcmp( msg, msg_matrix, sizeof( xi_mqtt_message_t ) ), ==, 0 );

    msg_matrix->connect.protocol_name = 0;
    msg_matrix->connect.client_id     = 0;
    msg_matrix->connect.username      = 0;
    msg_matrix->connect.password      = 0;

    xi_mqtt_message_free( &msg );
    xi_mqtt_message_free( &msg_matrix );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

    return;

err_handling:
    tt_abort_msg( ( "test must not fail!" ) );
end:
    xi_mqtt_message_free( &msg );
    xi_mqtt_message_free( &msg_matrix );
} )

XI_TT_TESTCASE(
    utest__fill_with_connect_data__valid_data_no_password_continuous_session__connect_msg,
    {
        xi_state_t local_state = XI_STATE_OK;

        const char username[] = "test_username";
        const char* password  = 0;

        xi_mqtt_message_t* msg        = NULL;
        xi_mqtt_message_t* msg_matrix = NULL;

        XI_ALLOC_AT( xi_mqtt_message_t, msg, local_state );
        XI_ALLOC_AT( xi_mqtt_message_t, msg_matrix, local_state );

        tt_want_int_op( fill_with_connect_data( msg, username, password, 7,
                                                XI_SESSION_CONTINUE, NULL, NULL, 0, 0 ),
                        ==, XI_STATE_OK );

        msg_matrix->common.common_u.common_bits.retain = XI_MQTT_RETAIN_FALSE;
        msg_matrix->common.common_u.common_bits.qos    = XI_MQTT_QOS_AT_MOST_ONCE;
        msg_matrix->common.common_u.common_bits.dup    = XI_MQTT_DUP_FALSE;
        msg_matrix->common.common_u.common_bits.type   = XI_MQTT_TYPE_CONNECT;

        msg_matrix->connect.protocol_version = 4;
        msg_matrix->connect.protocol_name    = msg->connect.protocol_name;

        msg_matrix->common.remaining_length = 0;

        tt_want_int_op(
            memcmp( msg->connect.protocol_name->data_ptr, "MQTT", sizeof( "MQTT" ) - 1 ),
            ==, 0 );

        tt_want_int_op(
            memcmp( msg->connect.client_id->data_ptr, username, sizeof( username ) - 1 ),
            ==, 0 );

        tt_want_int_op( msg->connect.password, ==, 0 );

        msg_matrix->connect.client_id = msg->connect.client_id;
        msg_matrix->connect.username  = msg->connect.username;
        msg_matrix->connect.password  = msg->connect.password;
        msg_matrix->connect.keepalive = msg->connect.keepalive;
        msg_matrix->connect.flags_u   = msg->connect.flags_u;

        tt_want_int_op( memcmp( msg, msg_matrix, sizeof( xi_mqtt_message_t ) ), ==, 0 );

        msg_matrix->connect.protocol_name = 0;
        msg_matrix->connect.client_id     = 0;
        msg_matrix->connect.username      = 0;
        msg_matrix->connect.password      = 0;

        xi_mqtt_message_free( &msg );
        xi_mqtt_message_free( &msg_matrix );

        tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

        return;

    err_handling:
        tt_abort_msg( ( "test must not fail!" ) );
    end:
        xi_mqtt_message_free( &msg );
        xi_mqtt_message_free( &msg_matrix );
    } )

XI_TT_TESTCASE(
    utest__fill_with_connect_data__valid_data_no_password_no_username__connect_msg, {
        xi_state_t local_state = XI_STATE_OK;

        const char* username = 0;
        const char* password = 0;

        xi_mqtt_message_t* msg        = NULL;
        xi_mqtt_message_t* msg_matrix = NULL;

        XI_ALLOC_AT( xi_mqtt_message_t, msg, local_state );
        XI_ALLOC_AT( xi_mqtt_message_t, msg_matrix, local_state );

        tt_want_int_op( fill_with_connect_data( msg, username, password, 7,
                                                XI_SESSION_CLEAN, NULL, NULL, 0, 0 ),
                        ==, XI_STATE_OK );

        msg_matrix->common.common_u.common_bits.retain = XI_MQTT_RETAIN_FALSE;
        msg_matrix->common.common_u.common_bits.qos    = XI_MQTT_QOS_AT_MOST_ONCE;
        msg_matrix->common.common_u.common_bits.dup    = XI_MQTT_DUP_FALSE;
        msg_matrix->common.common_u.common_bits.type   = XI_MQTT_TYPE_CONNECT;

        msg_matrix->connect.protocol_version = 4;
        msg_matrix->connect.protocol_name    = msg->connect.protocol_name;

        msg_matrix->common.remaining_length = 0;

        tt_want_int_op(
            memcmp( msg->connect.protocol_name->data_ptr, "MQTT", sizeof( "MQTT" ) - 1 ),
            ==, 0 );

        tt_want_int_op( msg->connect.client_id, ==, 0 );
        tt_want_int_op( msg->connect.password, ==, 0 );

        msg_matrix->connect.client_id = msg->connect.client_id;
        msg_matrix->connect.username  = msg->connect.username;
        msg_matrix->connect.password  = msg->connect.password;
        msg_matrix->connect.keepalive = msg->connect.keepalive;
        msg_matrix->connect.flags_u   = msg->connect.flags_u;

        tt_want_int_op( memcmp( msg, msg_matrix, sizeof( xi_mqtt_message_t ) ), ==, 0 );

        msg_matrix->connect.protocol_name = 0;
        msg_matrix->connect.client_id     = 0;
        msg_matrix->connect.username      = 0;
        msg_matrix->connect.password      = 0;

        xi_mqtt_message_free( &msg );
        xi_mqtt_message_free( &msg_matrix );

        tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

        return;

    err_handling:
        tt_abort_msg( ( "test must not fail!" ) );
    end:
        xi_mqtt_message_free( &msg );
        xi_mqtt_message_free( &msg_matrix );
    } )

/* Test the last_will entries */
XI_TT_TESTCASE(
    utest__fill_with_connect_data__valid_data_will_topic_will_message__connect_msg, {
        xi_state_t local_state = XI_STATE_OK;

        const char* username      = 0;
        const char* password      = 0;
        const char will_topic[]   = "device_last_will";
        const char will_message[] = "device quit unexpectedly";

        xi_mqtt_message_t* msg        = NULL;
        xi_mqtt_message_t* msg_matrix = NULL;

        XI_ALLOC_AT( xi_mqtt_message_t, msg, local_state );
        XI_ALLOC_AT( xi_mqtt_message_t, msg_matrix, local_state );

        tt_want_int_op( fill_with_connect_data( msg, username, password, 7,
                                                XI_SESSION_CLEAN, will_topic,
                                                will_message, XI_MQTT_QOS_AT_LEAST_ONCE,
                                                XI_MQTT_RETAIN_TRUE ),
                        ==, XI_STATE_OK );

        msg_matrix->common.common_u.common_bits.retain = XI_MQTT_RETAIN_FALSE;
        msg_matrix->common.common_u.common_bits.qos    = XI_MQTT_QOS_AT_MOST_ONCE;
        msg_matrix->common.common_u.common_bits.dup    = XI_MQTT_DUP_FALSE;
        msg_matrix->common.common_u.common_bits.type   = XI_MQTT_TYPE_CONNECT;

        msg_matrix->connect.protocol_version = 4;
        msg_matrix->connect.protocol_name    = msg->connect.protocol_name;

        msg_matrix->common.remaining_length = 0;

        tt_want_int_op(
            memcmp( msg->connect.protocol_name->data_ptr, "MQTT", sizeof( "MQTT" ) - 1 ),
            ==, 0 );

        tt_want_int_op( msg->connect.client_id, ==, 0 );
        tt_want_int_op( msg->connect.password, ==, 0 );

        /* Check the last will elements */
        tt_want_int_op( memcmp( msg->connect.will_topic->data_ptr, will_topic,
                                sizeof( will_topic ) - 1 ),
                        ==, 0 );
        tt_want_int_op( memcmp( msg->connect.will_message->data_ptr, will_message,
                                sizeof( will_message ) - 1 ),
                        ==, 0 );
        /* The will bit should be set since we passed a will_topic AND
         * will_message */
        tt_want_int_op( msg->connect.flags_u.flags_bits.will, ==, 1 );
        tt_want_int_op( msg->connect.flags_u.flags_bits.will_retain, ==,
                        XI_MQTT_RETAIN_TRUE );
        tt_want_int_op( msg->connect.flags_u.flags_bits.will_qos, ==,
                        XI_MQTT_QOS_AT_LEAST_ONCE );

        msg_matrix->connect.client_id    = msg->connect.client_id;
        msg_matrix->connect.username     = msg->connect.username;
        msg_matrix->connect.password     = msg->connect.password;
        msg_matrix->connect.keepalive    = msg->connect.keepalive;
        msg_matrix->connect.flags_u      = msg->connect.flags_u;
        msg_matrix->connect.will_topic   = msg->connect.will_topic;
        msg_matrix->connect.will_message = msg->connect.will_message;

        tt_want_int_op( memcmp( msg, msg_matrix, sizeof( xi_mqtt_message_t ) ), ==, 0 );

        msg_matrix->connect.protocol_name = 0;
        msg_matrix->connect.client_id     = 0;
        msg_matrix->connect.username      = 0;
        msg_matrix->connect.password      = 0;
        msg_matrix->connect.will_topic    = 0;
        msg_matrix->connect.will_message  = 0;

        xi_mqtt_message_free( &msg );
        xi_mqtt_message_free( &msg_matrix );

        tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

        return;

    err_handling:
        tt_abort_msg( ( "test must not fail!" ) );
    end:
        xi_mqtt_message_free( &msg );
        xi_mqtt_message_free( &msg_matrix );
    } )
XI_TT_TESTCASE(
    utest__fill_with_connect_data__valid_data_will_topic_no_will_message__connect_msg, {
        xi_state_t local_state = XI_STATE_OK;

        const char* username     = 0;
        const char* password     = 0;
        const char will_topic[]  = "device_last_will";
        const char* will_message = 0;

        xi_mqtt_message_t* msg        = NULL;
        xi_mqtt_message_t* msg_matrix = NULL;

        XI_ALLOC_AT( xi_mqtt_message_t, msg, local_state );
        XI_ALLOC_AT( xi_mqtt_message_t, msg_matrix, local_state );

        tt_want_int_op( fill_with_connect_data( msg, username, password, 7,
                                                XI_SESSION_CLEAN, will_topic,
                                                will_message, XI_MQTT_QOS_AT_LEAST_ONCE,
                                                XI_MQTT_RETAIN_TRUE ),
                        ==, XI_NULL_WILL_MESSAGE );

        msg_matrix->connect.protocol_name = 0;
        msg_matrix->connect.client_id     = 0;
        msg_matrix->connect.username      = 0;
        msg_matrix->connect.password      = 0;
        msg_matrix->connect.will_topic    = 0;
        msg_matrix->connect.will_message  = 0;

        xi_mqtt_message_free( &msg );
        xi_mqtt_message_free( &msg_matrix );

        tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

        return;

    err_handling:
        tt_abort_msg( ( "test must not fail!" ) );
    end:
        xi_mqtt_message_free( &msg );
        xi_mqtt_message_free( &msg_matrix );
    } )
XI_TT_TESTCASE(
    utest__fill_with_connect_data__valid_data_no_will_topic_will_message__connect_msg, {
        xi_state_t local_state = XI_STATE_OK;

        const char* username      = 0;
        const char* password      = 0;
        const char* will_topic    = 0;
        const char will_message[] = "device quit unexpectedly";

        xi_mqtt_message_t* msg        = NULL;
        xi_mqtt_message_t* msg_matrix = NULL;

        XI_ALLOC_AT( xi_mqtt_message_t, msg, local_state );
        XI_ALLOC_AT( xi_mqtt_message_t, msg_matrix, local_state );

        tt_want_int_op( fill_with_connect_data( msg, username, password, 7,
                                                XI_SESSION_CLEAN, will_topic,
                                                will_message, XI_MQTT_QOS_AT_LEAST_ONCE,
                                                XI_MQTT_RETAIN_TRUE ),
                        ==, XI_NULL_WILL_TOPIC );

        msg_matrix->connect.protocol_name = 0;
        msg_matrix->connect.client_id     = 0;
        msg_matrix->connect.username      = 0;
        msg_matrix->connect.password      = 0;
        msg_matrix->connect.will_topic    = 0;
        msg_matrix->connect.will_message  = 0;

        xi_mqtt_message_free( &msg );
        xi_mqtt_message_free( &msg_matrix );

        tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

        return;

    err_handling:
        tt_abort_msg( ( "test must not fail!" ) );
    end:
        xi_mqtt_message_free( &msg );
        xi_mqtt_message_free( &msg_matrix );
    } )
XI_TT_TESTCASE(
    utest__fill_with_connect_data__valid_data_no_will_topic_no_will_message__connect_msg,
    {
        xi_state_t local_state = XI_STATE_OK;

        const char* username     = 0;
        const char* password     = 0;
        const char* will_topic   = 0;
        const char* will_message = 0;

        xi_mqtt_message_t* msg        = NULL;
        xi_mqtt_message_t* msg_matrix = NULL;

        XI_ALLOC_AT( xi_mqtt_message_t, msg, local_state );
        XI_ALLOC_AT( xi_mqtt_message_t, msg_matrix, local_state );

        tt_want_int_op( fill_with_connect_data( msg, username, password, 7,
                                                XI_SESSION_CLEAN, will_topic,
                                                will_message, XI_MQTT_QOS_AT_LEAST_ONCE,
                                                XI_MQTT_RETAIN_TRUE ),
                        ==, XI_STATE_OK );

        msg_matrix->common.common_u.common_bits.retain = XI_MQTT_RETAIN_FALSE;
        msg_matrix->common.common_u.common_bits.qos    = XI_MQTT_QOS_AT_MOST_ONCE;
        msg_matrix->common.common_u.common_bits.dup    = XI_MQTT_DUP_FALSE;
        msg_matrix->common.common_u.common_bits.type   = XI_MQTT_TYPE_CONNECT;

        msg_matrix->connect.protocol_version = 4;
        msg_matrix->connect.protocol_name    = msg->connect.protocol_name;

        msg_matrix->common.remaining_length = 0;

        tt_want_int_op(
            memcmp( msg->connect.protocol_name->data_ptr, "MQTT", sizeof( "MQTT" ) - 1 ),
            ==, 0 );

        tt_want_int_op( msg->connect.client_id, ==, 0 );
        tt_want_int_op( msg->connect.password, ==, 0 );

        /* Check the last will elements */
        tt_want_int_op( msg->connect.will_topic, ==, 0 );
        tt_want_int_op( msg->connect.will_message, ==, 0 );

        /* The will bit must not be set since we passed no will_topic and no
         * will_message */
        tt_want_int_op( msg->connect.flags_u.flags_bits.will, ==, 0 );
        /* We will check to see that the will_retain and will_qos are ZERO
         * instead of one of */
        /* the enumerated types since the protocol demands that they be exactly
         * zero */
        tt_want_int_op( msg->connect.flags_u.flags_bits.will_retain, ==, 0 );
        tt_want_int_op( msg->connect.flags_u.flags_bits.will_qos, ==, 0 );

        msg_matrix->connect.client_id    = msg->connect.client_id;
        msg_matrix->connect.username     = msg->connect.username;
        msg_matrix->connect.password     = msg->connect.password;
        msg_matrix->connect.keepalive    = msg->connect.keepalive;
        msg_matrix->connect.flags_u      = msg->connect.flags_u;
        msg_matrix->connect.will_topic   = msg->connect.will_topic;
        msg_matrix->connect.will_message = msg->connect.will_message;

        tt_want_int_op( memcmp( msg, msg_matrix, sizeof( xi_mqtt_message_t ) ), ==, 0 );

        msg_matrix->connect.protocol_name = 0;
        msg_matrix->connect.client_id     = 0;
        msg_matrix->connect.username      = 0;
        msg_matrix->connect.password      = 0;
        msg_matrix->connect.will_topic    = 0;
        msg_matrix->connect.will_message  = 0;

        xi_mqtt_message_free( &msg );
        xi_mqtt_message_free( &msg_matrix );

        tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

        return;

    err_handling:
        tt_abort_msg( ( "test must not fail!" ) );
    end:
        xi_mqtt_message_free( &msg );
        xi_mqtt_message_free( &msg_matrix );
    } )
XI_TT_TESTCASE(
    utest__fill_with_connect_data__valid_data_will_topic_will_message_no_retain__connect_msg,
    {
        xi_state_t local_state = XI_STATE_OK;

        const char* username      = 0;
        const char* password      = 0;
        const char will_topic[]   = "device_last_will";
        const char will_message[] = "device quit unexpectedly";

        xi_mqtt_message_t* msg        = NULL;
        xi_mqtt_message_t* msg_matrix = NULL;

        XI_ALLOC_AT( xi_mqtt_message_t, msg, local_state );
        XI_ALLOC_AT( xi_mqtt_message_t, msg_matrix, local_state );

        tt_want_int_op( fill_with_connect_data( msg, username, password, 7,
                                                XI_SESSION_CLEAN, will_topic,
                                                will_message, XI_MQTT_QOS_AT_LEAST_ONCE,
                                                XI_MQTT_RETAIN_FALSE ),
                        ==, XI_STATE_OK );

        msg_matrix->common.common_u.common_bits.retain = XI_MQTT_RETAIN_FALSE;
        msg_matrix->common.common_u.common_bits.qos    = XI_MQTT_QOS_AT_MOST_ONCE;
        msg_matrix->common.common_u.common_bits.dup    = XI_MQTT_DUP_FALSE;
        msg_matrix->common.common_u.common_bits.type   = XI_MQTT_TYPE_CONNECT;

        msg_matrix->connect.protocol_version = 4;
        msg_matrix->connect.protocol_name    = msg->connect.protocol_name;

        msg_matrix->common.remaining_length = 0;

        tt_want_int_op(
            memcmp( msg->connect.protocol_name->data_ptr, "MQTT", sizeof( "MQTT" ) - 1 ),
            ==, 0 );

        tt_want_int_op( msg->connect.client_id, ==, 0 );
        tt_want_int_op( msg->connect.password, ==, 0 );

        /* Check the last will elements */
        tt_want_int_op( memcmp( msg->connect.will_topic->data_ptr, will_topic,
                                sizeof( will_topic ) - 1 ),
                        ==, 0 );
        tt_want_int_op( memcmp( msg->connect.will_message->data_ptr, will_message,
                                sizeof( will_message ) - 1 ),
                        ==, 0 );

        /* The will bit should be set since we passed a will_topic AND
         * will_message */
        tt_want_int_op( msg->connect.flags_u.flags_bits.will, ==, 1 );
        tt_want_int_op( msg->connect.flags_u.flags_bits.will_retain, ==,
                        XI_MQTT_RETAIN_FALSE );
        tt_want_int_op( msg->connect.flags_u.flags_bits.will_qos, ==,
                        XI_MQTT_QOS_AT_LEAST_ONCE );

        msg_matrix->connect.client_id    = msg->connect.client_id;
        msg_matrix->connect.username     = msg->connect.username;
        msg_matrix->connect.password     = msg->connect.password;
        msg_matrix->connect.keepalive    = msg->connect.keepalive;
        msg_matrix->connect.flags_u      = msg->connect.flags_u;
        msg_matrix->connect.will_topic   = msg->connect.will_topic;
        msg_matrix->connect.will_message = msg->connect.will_message;

        tt_want_int_op( memcmp( msg, msg_matrix, sizeof( xi_mqtt_message_t ) ), ==, 0 );

        msg_matrix->connect.protocol_name = 0;
        msg_matrix->connect.client_id     = 0;
        msg_matrix->connect.username      = 0;
        msg_matrix->connect.password      = 0;
        msg_matrix->connect.will_topic    = 0;
        msg_matrix->connect.will_message  = 0;

        xi_mqtt_message_free( &msg );
        xi_mqtt_message_free( &msg_matrix );

        tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

        return;

    err_handling:
        tt_abort_msg( ( "test must not fail!" ) );
    end:
        xi_mqtt_message_free( &msg );
        xi_mqtt_message_free( &msg_matrix );
    } )
XI_TT_TESTCASE(
    utest__fill_with_connect_data__valid_data_will_topic_will_message_retain_willqosAMO__connect_msg,
    {
        xi_state_t local_state = XI_STATE_OK;

        const char* username      = 0;
        const char* password      = 0;
        const char will_topic[]   = "device_last_will";
        const char will_message[] = "device quit unexpectedly";

        xi_mqtt_message_t* msg        = NULL;
        xi_mqtt_message_t* msg_matrix = NULL;

        XI_ALLOC_AT( xi_mqtt_message_t, msg, local_state );
        XI_ALLOC_AT( xi_mqtt_message_t, msg_matrix, local_state );

        tt_want_int_op( fill_with_connect_data(
                            msg, username, password, 7, XI_SESSION_CLEAN, will_topic,
                            will_message, XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_TRUE ),
                        ==, XI_STATE_OK );

        msg_matrix->common.common_u.common_bits.retain = XI_MQTT_RETAIN_FALSE;
        msg_matrix->common.common_u.common_bits.qos    = XI_MQTT_QOS_AT_MOST_ONCE;
        msg_matrix->common.common_u.common_bits.dup    = XI_MQTT_DUP_FALSE;
        msg_matrix->common.common_u.common_bits.type   = XI_MQTT_TYPE_CONNECT;

        msg_matrix->connect.protocol_version = 4;
        msg_matrix->connect.protocol_name    = msg->connect.protocol_name;

        msg_matrix->common.remaining_length = 0;

        tt_want_int_op(
            memcmp( msg->connect.protocol_name->data_ptr, "MQTT", sizeof( "MQTT" ) - 1 ),
            ==, 0 );

        tt_want_int_op( msg->connect.client_id, ==, 0 );
        tt_want_int_op( msg->connect.password, ==, 0 );

        /* Check the last will elements */
        tt_want_int_op( memcmp( msg->connect.will_topic->data_ptr, will_topic,
                                sizeof( will_topic ) - 1 ),
                        ==, 0 );
        tt_want_int_op( memcmp( msg->connect.will_message->data_ptr, will_message,
                                sizeof( will_message ) - 1 ),
                        ==, 0 );
        /* The will bit should be set since we passed a will_topic AND
         * will_message */
        tt_want_int_op( msg->connect.flags_u.flags_bits.will, ==, 1 );
        tt_want_int_op( msg->connect.flags_u.flags_bits.will_retain, ==,
                        XI_MQTT_RETAIN_TRUE );
        tt_want_int_op( msg->connect.flags_u.flags_bits.will_qos, ==,
                        XI_MQTT_QOS_AT_MOST_ONCE );

        msg_matrix->connect.client_id    = msg->connect.client_id;
        msg_matrix->connect.username     = msg->connect.username;
        msg_matrix->connect.password     = msg->connect.password;
        msg_matrix->connect.keepalive    = msg->connect.keepalive;
        msg_matrix->connect.flags_u      = msg->connect.flags_u;
        msg_matrix->connect.will_topic   = msg->connect.will_topic;
        msg_matrix->connect.will_message = msg->connect.will_message;

        tt_want_int_op( memcmp( msg, msg_matrix, sizeof( xi_mqtt_message_t ) ), ==, 0 );

        msg_matrix->connect.protocol_name = 0;
        msg_matrix->connect.client_id     = 0;
        msg_matrix->connect.username      = 0;
        msg_matrix->connect.password      = 0;
        msg_matrix->connect.will_topic    = 0;
        msg_matrix->connect.will_message  = 0;

        xi_mqtt_message_free( &msg );
        xi_mqtt_message_free( &msg_matrix );

        tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

        return;

    err_handling:
        tt_abort_msg( ( "test must not fail!" ) );
    end:
        xi_mqtt_message_free( &msg );
        xi_mqtt_message_free( &msg_matrix );
    } )
XI_TT_TESTCASE(
    utest__fill_with_connect_data__valid_data_will_topic_will_message_retain_willqosEO__connect_msg,
    {
        xi_state_t local_state = XI_STATE_OK;

        const char* username      = 0;
        const char* password      = 0;
        const char will_topic[]   = "device_last_will";
        const char will_message[] = "device quit unexpectedly";

        xi_mqtt_message_t* msg        = NULL;
        xi_mqtt_message_t* msg_matrix = NULL;

        XI_ALLOC_AT( xi_mqtt_message_t, msg, local_state );
        XI_ALLOC_AT( xi_mqtt_message_t, msg_matrix, local_state );

        tt_want_int_op( fill_with_connect_data(
                            msg, username, password, 7, XI_SESSION_CLEAN, will_topic,
                            will_message, XI_MQTT_QOS_EXACTLY_ONCE, XI_MQTT_RETAIN_TRUE ),
                        ==, XI_STATE_OK );

        msg_matrix->common.common_u.common_bits.retain = XI_MQTT_RETAIN_FALSE;
        msg_matrix->common.common_u.common_bits.qos    = XI_MQTT_QOS_AT_MOST_ONCE;
        msg_matrix->common.common_u.common_bits.dup    = XI_MQTT_DUP_FALSE;
        msg_matrix->common.common_u.common_bits.type   = XI_MQTT_TYPE_CONNECT;

        msg_matrix->connect.protocol_version = 4;
        msg_matrix->connect.protocol_name    = msg->connect.protocol_name;

        msg_matrix->common.remaining_length = 0;

        tt_want_int_op(
            memcmp( msg->connect.protocol_name->data_ptr, "MQTT", sizeof( "MQTT" ) - 1 ),
            ==, 0 );

        tt_want_int_op( msg->connect.client_id, ==, 0 );
        tt_want_int_op( msg->connect.password, ==, 0 );

        /* Check the last will elements */
        tt_want_int_op( memcmp( msg->connect.will_topic->data_ptr, will_topic,
                                sizeof( will_topic ) - 1 ),
                        ==, 0 );
        tt_want_int_op( memcmp( msg->connect.will_message->data_ptr, will_message,
                                sizeof( will_message ) - 1 ),
                        ==, 0 );
        /* The will bit should be set since we passed a will_topic AND
         * will_message */
        tt_want_int_op( msg->connect.flags_u.flags_bits.will, ==, 1 );
        tt_want_int_op( msg->connect.flags_u.flags_bits.will_retain, ==,
                        XI_MQTT_RETAIN_TRUE );
        tt_want_int_op( msg->connect.flags_u.flags_bits.will_qos, ==,
                        XI_MQTT_QOS_EXACTLY_ONCE );

        msg_matrix->connect.client_id    = msg->connect.client_id;
        msg_matrix->connect.username     = msg->connect.username;
        msg_matrix->connect.password     = msg->connect.password;
        msg_matrix->connect.keepalive    = msg->connect.keepalive;
        msg_matrix->connect.flags_u      = msg->connect.flags_u;
        msg_matrix->connect.will_topic   = msg->connect.will_topic;
        msg_matrix->connect.will_message = msg->connect.will_message;

        tt_want_int_op( memcmp( msg, msg_matrix, sizeof( xi_mqtt_message_t ) ), ==, 0 );

        msg_matrix->connect.protocol_name = 0;
        msg_matrix->connect.client_id     = 0;
        msg_matrix->connect.username      = 0;
        msg_matrix->connect.password      = 0;
        msg_matrix->connect.will_topic    = 0;
        msg_matrix->connect.will_message  = 0;

        xi_mqtt_message_free( &msg );
        xi_mqtt_message_free( &msg_matrix );

        tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

        return;

    err_handling:
        tt_abort_msg( ( "test must not fail!" ) );
    end:
        xi_mqtt_message_free( &msg );
        xi_mqtt_message_free( &msg_matrix );
    } )

XI_TT_TESTCASE( utest__fill_with_publish_data__valid_data__publish_msg, {
    xi_state_t local_state = XI_STATE_OK;

    xi_mqtt_message_t* msg        = NULL;
    xi_mqtt_message_t* msg_matrix = NULL;

    const char topic[] = "test_topic";

    xi_data_desc_t* content = xi_make_desc_from_string_copy( "test_content" );

    // test for all combinations of common
    size_t i = 0;
    for ( i = 0; i < 16; ++i )
    {
        XI_ALLOC_AT( xi_mqtt_message_t, msg, local_state );
        XI_ALLOC_AT( xi_mqtt_message_t, msg_matrix, local_state );

        xi_utest_mqtt_message_details_to_uint16_t mqtt_common = {{.value = i}};

        tt_want_int_op(
            fill_with_publish_data( msg, topic, content,
                                    mqtt_common.xi_mqtt_union.qos_retain_dup.qos,
                                    mqtt_common.xi_mqtt_union.qos_retain_dup.retain,
                                    mqtt_common.xi_mqtt_union.qos_retain_dup.dup, 17 ),
            ==, XI_STATE_OK );

        msg_matrix->common.common_u.common_bits.retain =
            mqtt_common.xi_mqtt_union.qos_retain_dup.retain;
        msg_matrix->common.common_u.common_bits.qos =
            mqtt_common.xi_mqtt_union.qos_retain_dup.qos;
        msg_matrix->common.common_u.common_bits.dup =
            mqtt_common.xi_mqtt_union.qos_retain_dup.dup;
        msg_matrix->common.common_u.common_bits.type = XI_MQTT_TYPE_PUBLISH;
        msg_matrix->common.remaining_length          = 0;

        msg_matrix->publish.topic_name = msg->publish.topic_name;
        msg_matrix->publish.content    = msg->publish.content;

        tt_want_int_op(
            memcmp( msg->publish.topic_name->data_ptr, topic, sizeof( topic ) - 1 ), ==,
            0 );
        tt_want_int_op(
            memcmp( msg->publish.content->data_ptr, content->data_ptr, content->length ),
            ==, 0 );

        msg_matrix->publish.message_id = 17;

        tt_want_int_op( memcmp( msg, msg_matrix, sizeof( xi_mqtt_message_t ) ), ==, 0 );

        msg_matrix->publish.topic_name = 0;
        msg_matrix->publish.content    = 0;

        xi_mqtt_message_free( &msg );
        xi_mqtt_message_free( &msg_matrix );
    }

    xi_free_desc( &content );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

    return;

err_handling:
    tt_abort_msg( ( "test must not fail!" ) );
end:
    xi_mqtt_message_free( &msg );
    xi_mqtt_message_free( &msg_matrix );
    xi_free_desc( &content );
} )

#ifndef XI_EMBEDDED_TESTS
XI_TT_TESTCASE( utest__fill_with_publish_data__valid_data_max_payload__publish_msg, {
    utest__fill_with_publish_data__valid_data_max_payload__publish_msg_help();
} )

XI_TT_TESTCASE(
    utest__fill_with_publish_data__too_big_payload_data__xi_mqtt_too_big_paload, {
        xi_state_t local_state = XI_STATE_OK;

        xi_mqtt_message_t* msg        = NULL;
        xi_mqtt_message_t* msg_matrix = NULL;

        const size_t cnt_size = XI_MQTT_MAX_PAYLOAD_SIZE + 1;

        const char topic[] = "test_topic";

        xi_data_desc_t* content = xi_make_empty_desc_alloc( cnt_size );
        memset( ( void* )content->data_ptr, 'a', cnt_size );
        content->length = cnt_size;

        XI_ALLOC_AT( xi_mqtt_message_t, msg, local_state );
        XI_ALLOC_AT( xi_mqtt_message_t, msg_matrix, local_state );

        tt_want_int_op(
            fill_with_publish_data( msg, topic, content, XI_MQTT_QOS_AT_LEAST_ONCE,
                                    XI_MQTT_RETAIN_FALSE, XI_MQTT_DUP_FALSE, 17 ),
            ==, XI_MQTT_PAYLOAD_SIZE_TOO_LARGE );

        xi_mqtt_message_free( &msg );
        xi_mqtt_message_free( &msg_matrix );
        xi_free_desc( &content );

        tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

        return;

    err_handling:
        tt_abort_msg( ( "test must not fail!" ) );
    end:
        xi_mqtt_message_free( &msg );
        xi_mqtt_message_free( &msg_matrix );
        xi_free_desc( &content );
    } )
#endif // XI_EMBEDDED_TESTS

XI_TT_TESTCASE( utest__fill_with_subscribe_data__valid_data__publish_msg, {
    xi_state_t local_state = XI_STATE_OK;

    xi_mqtt_message_t* msg        = NULL;
    xi_mqtt_message_t* msg_matrix = NULL;

    const char topic[] = "test_topic";

    XI_ALLOC_AT( xi_mqtt_message_t, msg, local_state );
    XI_ALLOC_AT( xi_mqtt_message_t, msg_matrix, local_state );

    tt_want_int_op( fill_with_subscribe_data( msg, topic, 17, XI_MQTT_QOS_AT_LEAST_ONCE,
                                              XI_MQTT_DUP_FALSE ),
                    ==, XI_STATE_OK );

    msg_matrix->common.common_u.common_bits.retain = XI_MQTT_RETAIN_FALSE;
    msg_matrix->common.common_u.common_bits.qos    = XI_MQTT_QOS_AT_LEAST_ONCE;
    msg_matrix->common.common_u.common_bits.dup    = XI_MQTT_DUP_FALSE;
    msg_matrix->common.common_u.common_bits.type   = XI_MQTT_TYPE_SUBSCRIBE;

    msg_matrix->common.remaining_length = 0;

    msg_matrix->subscribe.topics = msg->subscribe.topics;

    tt_want_int_op(
        memcmp( msg->subscribe.topics->name->data_ptr, topic, sizeof( topic ) - 1 ), ==,
        0 );

    msg_matrix->subscribe.message_id = 17;

    tt_want_int_op( memcmp( msg, msg_matrix, sizeof( xi_mqtt_message_t ) ), ==, 0 );

    msg_matrix->subscribe.topics = 0;

    xi_mqtt_message_free( &msg );
    xi_mqtt_message_free( &msg_matrix );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

    return;

err_handling:
    tt_abort_msg( ( "test must not fail!" ) );
end:
    xi_mqtt_message_free( &msg );
    xi_mqtt_message_free( &msg_matrix );
} )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
