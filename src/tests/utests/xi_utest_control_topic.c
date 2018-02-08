/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"

#include "xi_control_topic_name.h"
#include "xi_types.h"

#include "xively.h"
#include "xi_memory_checks.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#define CTID "xi"
#define S "/"

#define CONTROL_TOPIC_SERVICE_ID "ctrl"
#define CONTROL_TOPIC_VERSION_ID "v1"
#define CONTROL_TOPIC_DEVICE_ID "device-id-is-missing"
#define CONTROL_TOPIC_DEVICE_ID2 "434234-234234-234234-2234234-23423423"
#define CONTROL_TOPIC_DEVICE_ID3                                                         \
    "some-very-long-1232343-mixed-with-23423423-numbers-device-id-$$$$$"
#define CONTROL_TOPIC_CHANNEL_NAME_PUBLISH "svc"
#define CONTROL_TOPIC_CHANNEL_NAME_SUBSCRIBE "cln"

/**
 * @brief The utest_control_topic_topic_name_s struct
 *
 * contains the data for a single test case, name and pass are used
 * in order to create a control topic name and expected topic names
 * will be used later to compare the result of the control topic generator
 * function
 */
struct utest_control_topic_topic_name_s
{
    const char* const device_id;
    const char* const expected_subscribe_topic_name;
    const char* const expected_publish_topic_name;
};

/* this is the array that contains all the topic names ingridients to be tested
    against control topic name generation */
const struct utest_control_topic_topic_name_s topic_names[] = {
    {0, CTID S CONTROL_TOPIC_SERVICE_ID S CONTROL_TOPIC_VERSION_ID S
            CONTROL_TOPIC_DEVICE_ID S CONTROL_TOPIC_CHANNEL_NAME_SUBSCRIBE,
     CTID S CONTROL_TOPIC_SERVICE_ID S CONTROL_TOPIC_VERSION_ID S CONTROL_TOPIC_DEVICE_ID
         S CONTROL_TOPIC_CHANNEL_NAME_PUBLISH},
    {CONTROL_TOPIC_DEVICE_ID2,
     CTID S CONTROL_TOPIC_SERVICE_ID S CONTROL_TOPIC_VERSION_ID S CONTROL_TOPIC_DEVICE_ID2
         S CONTROL_TOPIC_CHANNEL_NAME_SUBSCRIBE,
     CTID S CONTROL_TOPIC_SERVICE_ID S CONTROL_TOPIC_VERSION_ID S CONTROL_TOPIC_DEVICE_ID2
         S CONTROL_TOPIC_CHANNEL_NAME_PUBLISH},
    {CONTROL_TOPIC_DEVICE_ID3,
     CTID S CONTROL_TOPIC_SERVICE_ID S CONTROL_TOPIC_VERSION_ID S CONTROL_TOPIC_DEVICE_ID3
         S CONTROL_TOPIC_CHANNEL_NAME_SUBSCRIBE,
     CTID S CONTROL_TOPIC_SERVICE_ID S CONTROL_TOPIC_VERSION_ID S CONTROL_TOPIC_DEVICE_ID3
         S CONTROL_TOPIC_CHANNEL_NAME_PUBLISH}};

/**
 * @brief utest__control_topic_test_topic_name
 *
 * This function is just a helper that tests if the pattern matches the
 *generated control topic
 * name.
 */
void utest__control_topic_test_topic_name( const char* const topic_name,
                                           const char* const pattern )
{
    /* if you want the visualisation */
    printf( "%s vs %s\n", topic_name, pattern );

    /* compare the static part of the topic name ( except random number ) */
    tt_want_int_op( memcmp( topic_name, pattern, strlen( pattern ) ), ==, 0 );
}

#endif

XI_TT_TESTGROUP_BEGIN( utest_control_topic )

XI_TT_TESTCASE(
    utest__xi_control_topic_create_topic_name__correct_data__create_topic_name, {
        size_t i = 0;
        for ( ; i < XI_ARRAYSIZE( topic_names ); ++i )
        {
            char* sub_topic_name = NULL;
            char* pub_topic_name = NULL;

            if ( topic_names[i].device_id != NULL )
            {
                tt_int_op( xi_initialize( "" ), ==, XI_STATE_OK );
            }

            xi_state_t state = xi_control_topic_create_topic_name(
                topic_names[i].device_id, &sub_topic_name, &pub_topic_name );

            tt_int_op( state, ==, XI_STATE_OK );

            utest__control_topic_test_topic_name(
                sub_topic_name, topic_names[i].expected_subscribe_topic_name );
            utest__control_topic_test_topic_name(
                pub_topic_name, topic_names[i].expected_publish_topic_name );

            if ( topic_names[i].device_id != NULL )
            {
                tt_int_op( xi_shutdown(), ==, XI_STATE_OK );
            }

            XI_SAFE_FREE( sub_topic_name );
            XI_SAFE_FREE( pub_topic_name );
        }

        tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
    end:;
    } )

XI_TT_TESTCASE(
    utest__xi_control_topic_create_topic_name__wrong_data_sub_topic_not_empty__should_return_invalid_parameter,
    {
        char* sub_topic_name = "NULL";
        char* pub_topic_name = NULL;

        xi_state_t state = xi_control_topic_create_topic_name(
            "utest_device_id", &sub_topic_name, &pub_topic_name );

        tt_want_int_op( state, ==, XI_INVALID_PARAMETER );
        tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
    end:;
    } )

XI_TT_TESTCASE(
    utest__xi_control_topic_create_topic_name__wrong_data_pub_not_empty__should_return_invalid_parameter,
    {
        char* sub_topic_name = NULL;
        char* pub_topic_name = "NULL";

        xi_state_t state = xi_control_topic_create_topic_name(
            "utest_device_id", &sub_topic_name, &pub_topic_name );

        tt_want_int_op( state, ==, XI_INVALID_PARAMETER );
        tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
    end:;
    } )

XI_TT_TESTCASE(
    utest__xi_control_topic_create_topic_name__wrong_topic_name1__should_return_invalid_parameter,
    {
        char* pub_topic_name = NULL;

        xi_state_t state = xi_control_topic_create_topic_name( "utest_device_id", NULL,
                                                               &pub_topic_name );

        tt_want_int_op( state, ==, XI_INVALID_PARAMETER );
        tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
    end:;
    } )

XI_TT_TESTCASE(
    utest__xi_control_topic_create_topic_name__wrong_topic_name2__should_return_invalid_parameter,
    {
        char* sub_topic_name = NULL;

        xi_state_t state = xi_control_topic_create_topic_name( "utest_device_id",
                                                               &sub_topic_name, NULL );

        tt_want_int_op( state, ==, XI_INVALID_PARAMETER );
        tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
    end:;
    } )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
