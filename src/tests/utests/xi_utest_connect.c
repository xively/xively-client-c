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

#include <stdio.h>


#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

extern void xi_default_client_callback( xi_context_handle_t in_context_handle,
                                        void* data,
                                        xi_state_t state );

#endif

XI_TT_TESTGROUP_BEGIN( utest_connect )

XI_TT_TESTCASE_WITH_SETUP(
    test_INVALID_connect_context, xi_utest_setup_basic, xi_utest_teardown_basic, NULL, {
        xi_state_t state;

        /* Call xi_connect_with_callback before creating a context */
        xi_context_handle_t xi_invalid_context = -1;

        state = xi_connect( xi_invalid_context, NULL, NULL, 10, 20, XI_SESSION_CLEAN,
                            &xi_default_client_callback );

        tt_assert( XI_NULL_CONTEXT == state );
    end:;
    } )

XI_TT_TESTCASE_WITH_SETUP(
    test_VALID_connect_context, xi_utest_setup_basic, xi_utest_teardown_basic, NULL, {
        xi_state_t state;

        /* Call xi_connect_with_callback after creating a context */
        xi_context_handle_t xi_context = xi_create_context();
        tt_assert( 0 <= xi_context );

        state = xi_connect( xi_context, NULL, NULL, 10, 20, XI_SESSION_CLEAN,
                            &xi_default_client_callback );

        tt_assert( XI_STATE_OK == state );
        xi_delete_context( xi_context );
    end:;
    } )

XI_TT_TESTCASE_WITH_SETUP(
    test_connect_to__valid_host, xi_utest_setup_basic, xi_utest_teardown_basic, NULL, {
        xi_state_t state;

        /* Call xi_connect_with_callback after creating a context */
        xi_context_handle_t xi_context = xi_create_context();
        tt_assert( 0 <= xi_context );

        state = xi_connect_to( xi_context, "beer.deer.com" /* host */
                               ,
                               12345 /* port */
                               ,
                               NULL, NULL, 10, 20, XI_SESSION_CLEAN,
                               &xi_default_client_callback );

        tt_assert( XI_STATE_OK == state );
        xi_delete_context( xi_context );
    end:;
    } )

XI_TT_TESTCASE_WITH_SETUP(
    test_connect_to__invalid_host, xi_utest_setup_basic, xi_utest_teardown_basic, NULL, {
        xi_state_t state;

        /* Call xi_connect_with_callback after creating a context */
        xi_context_handle_t xi_context = xi_create_context();
        tt_assert( 0 <= xi_context );

        state = xi_connect_to( xi_context, NULL /* host */
                               ,
                               1234 /* port */
                               ,
                               NULL, NULL, 10, 20, XI_SESSION_CLEAN,
                               &xi_default_client_callback );

        tt_assert( XI_NULL_HOST == state );
        xi_delete_context( xi_context );
    end:;
    } )

XI_TT_TESTCASE_WITH_SETUP( test_INVALIDCONTEXT_connect_lastwill,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               xi_state_t state;

                               /* Call xi_connect_with_callback with an invalid context */
                               xi_context_handle_t xi_invalid_context =
                                   XI_INVALID_CONTEXT_HANDLE;

                               state = xi_connect_with_lastwill(
                                   xi_invalid_context, NULL, NULL, 10, 20,
                                   XI_SESSION_CLEAN, "dummy will topic",
                                   "dummy will message", 0, 0,
                                   xi_default_client_callback );

                               tt_assert( XI_NULL_CONTEXT == state );
                           end:;
                           } )

XI_TT_TESTCASE_WITH_SETUP(
    test_connect_lastwill, xi_utest_setup_basic, xi_utest_teardown_basic, NULL, {
        xi_state_t state;

        /* Call xi_connect_with_callback after creating a context */
        xi_context_handle_t xi_context = xi_create_context();
        tt_assert( XI_INVALID_CONTEXT_HANDLE < xi_context );

        state =
            xi_connect_with_lastwill( xi_context, NULL, NULL, 10, 20, XI_SESSION_CLEAN,
                                      NULL, NULL, 0, 0, xi_default_client_callback );

        tt_assert( XI_NULL_WILL_TOPIC == state );
        xi_delete_context( xi_context );
    end:;
    } )

XI_TT_TESTCASE_WITH_SETUP( test_connect_lastwill_will_topic_will_message,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               xi_state_t state;

                               /* Call xi_connect_with_lastwill after creating a context
                                */
                               xi_context_handle_t xi_context = xi_create_context();
                               tt_assert( XI_INVALID_CONTEXT_HANDLE < xi_context );

                               state = xi_connect_with_lastwill(
                                   xi_context, NULL, NULL, 10, 20, XI_SESSION_CLEAN,
                                   "lastwilltopic", "This is the last will mesage",
                                   XI_MQTT_QOS_AT_LEAST_ONCE, XI_MQTT_RETAIN_TRUE,
                                   xi_default_client_callback );

                               tt_assert( XI_STATE_OK == state );
                               xi_delete_context( xi_context );
                           end:;
                           } )

XI_TT_TESTCASE_WITH_SETUP( test_connect_lastwill_will_topic_no_will_message,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               xi_state_t state;

                               /* Call xi_connect_wikth_last_will after creating a
                                * context */
                               xi_context_handle_t xi_context = xi_create_context();
                               tt_assert( XI_INVALID_CONTEXT_HANDLE < xi_context );

                               state = xi_connect_with_lastwill(
                                   xi_context, NULL, NULL, 10, 20, XI_SESSION_CLEAN,
                                   "lastwilltopic", NULL, XI_MQTT_QOS_AT_LEAST_ONCE,
                                   XI_MQTT_RETAIN_TRUE, xi_default_client_callback );

                               tt_assert( XI_NULL_WILL_MESSAGE == state );
                               xi_delete_context( xi_context );
                           end:;
                           } )

XI_TT_TESTCASE_WITH_SETUP( test_connect_lastwill_no_will_topic__will_message,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               xi_state_t state;

                               /* Call xi_connect_with_lastwill after creating a context
                                */
                               xi_context_handle_t xi_context = xi_create_context();
                               tt_assert( XI_INVALID_CONTEXT_HANDLE < xi_context );

                               state = xi_connect_with_lastwill(
                                   xi_context, NULL, NULL, 10, 20, XI_SESSION_CLEAN, NULL,
                                   "This is the last will mesage",
                                   XI_MQTT_QOS_AT_LEAST_ONCE, XI_MQTT_RETAIN_TRUE,
                                   xi_default_client_callback );

                               tt_assert( XI_NULL_WILL_TOPIC == state );
                               xi_delete_context( xi_context );
                           end:;
                           } )

XI_TT_TESTCASE_WITH_SETUP( test_connect_lastwill_to__invalid_host,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               xi_state_t state;

                               /* Call xi_connect_with_lastwill after creating a context
                                */
                               xi_context_handle_t xi_context = xi_create_context();
                               tt_assert( XI_INVALID_CONTEXT_HANDLE < xi_context );

                               state = xi_connect_with_lastwill_to(
                                   xi_context, NULL /* host */
                                   ,
                                   1234 /* port */
                                   ,
                                   NULL, NULL, 10, 20, XI_SESSION_CLEAN, "lastwilltopic",
                                   "This is the last will mesage",
                                   XI_MQTT_QOS_AT_LEAST_ONCE, XI_MQTT_RETAIN_TRUE,
                                   xi_default_client_callback );

                               tt_assert( XI_NULL_HOST == state );
                               xi_delete_context( xi_context );
                           end:;
                           } )

XI_TT_TESTCASE_WITH_SETUP( test_connect_lastwill_to__valid_host,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                               xi_state_t state;

                               /* Call xi_connect_with_lastwill after creating a context
                                */
                               xi_context_handle_t xi_context = xi_create_context();
                               tt_assert( XI_INVALID_CONTEXT_HANDLE < xi_context );

                               state = xi_connect_with_lastwill_to(
                                   xi_context, "beer.deer.lastwill.com" /* host */
                                   ,
                                   1234 /* port */
                                   ,
                                   NULL, NULL, 10, 20, XI_SESSION_CLEAN, "lastwilltopic",
                                   "This is the last will mesage",
                                   XI_MQTT_QOS_AT_LEAST_ONCE, XI_MQTT_RETAIN_TRUE,
                                   xi_default_client_callback );

                               tt_assert( XI_STATE_OK == state );
                               xi_delete_context( xi_context );
                           end:;
                           } )

XI_TT_TESTCASE_WITH_SETUP( test_INVALID_is_context_connected_context,
                           xi_utest_setup_basic,
                           xi_utest_teardown_basic,
                           NULL,
                           {
                                uint8_t is_connected = 0;
                        
                                /* Call xi_is_context_connected before creating a context */
                                xi_context_handle_t xi_invalid_context = -1;
                        
                                is_connected = xi_is_context_connected( xi_invalid_context );
                        
                                tt_assert( 0 == is_connected );
                            end:;
                            } )
                        
XI_TT_TESTCASE_WITH_SETUP( test_VALID_is_context_connected_context,
                            xi_utest_setup_basic,
                            xi_utest_teardown_basic,
                            NULL, 
                            {
                                uint8_t is_connected = 0;
                        
                                /* Call xi_is_context_connected after creating a context */
                                xi_context_handle_t xi_context = xi_create_context();
                                tt_assert( XI_INVALID_CONTEXT_HANDLE < xi_context );
 
                                is_connected = xi_is_context_connected( xi_context );
                        
                                tt_assert( 0 == is_connected );
                                xi_delete_context( xi_context );
                            end:;
                            } )
     
XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
