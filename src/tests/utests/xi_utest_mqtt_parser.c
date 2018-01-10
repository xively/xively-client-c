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
#include "xi_globals.h"
#include "xi_mqtt_parser.h"

#include "xi_memory_checks.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#endif

XI_TT_TESTGROUP_BEGIN( utest_mqtt_parser )

XI_TT_TESTCASE( utest__parse_suback_response__valid_data__qos_0_granted, {
    xi_mqtt_suback_status_t status = ( xi_mqtt_suback_status_t )-1;
    xi_state_t state               = xi_mqtt_parse_suback_response( &status, 0x00 );

    tt_want_int_op( state, ==, XI_STATE_OK );
    tt_want_int_op( status, ==, XI_MQTT_QOS_0_GRANTED );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
} )

XI_TT_TESTCASE( utest__parse_suback_response__valid_data__qos_1_granted, {
    xi_mqtt_suback_status_t status = ( xi_mqtt_suback_status_t )-1;
    xi_state_t state               = xi_mqtt_parse_suback_response( &status, 0x01 );

    tt_want_int_op( state, ==, XI_STATE_OK );
    tt_want_int_op( status, ==, XI_MQTT_QOS_1_GRANTED );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
} )

XI_TT_TESTCASE( utest__parse_suback_response__valid_data__qos_2_granted, {
    xi_mqtt_suback_status_t status = ( xi_mqtt_suback_status_t )-1;
    xi_state_t state               = xi_mqtt_parse_suback_response( &status, 0x02 );

    tt_want_int_op( state, ==, XI_STATE_OK );
    tt_want_int_op( status, ==, XI_MQTT_QOS_2_GRANTED );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
} )

XI_TT_TESTCASE( utest__parse_suback_response__valid_data__subscription_failed, {
    xi_mqtt_suback_status_t status = ( xi_mqtt_suback_status_t )-1;
    xi_state_t state               = xi_mqtt_parse_suback_response( &status, 0x80 );

    tt_want_int_op( state, ==, XI_STATE_OK );
    tt_want_int_op( status, ==, XI_MQTT_SUBACK_FAILED );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
} )

XI_TT_TESTCASE( utest__parse_suback_response__invalid_data__parser_error, {
    xi_mqtt_suback_status_t status = ( xi_mqtt_suback_status_t )-1;

    uint8_t i = 3;

    for ( ; i < 0xFF; ++i )
    {
        if ( i == 0x80 )
        {
            continue;
        }

        xi_state_t state = xi_mqtt_parse_suback_response( &status, i );

        tt_want_int_op( state, ==, XI_MQTT_PARSER_ERROR );
    }

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
} )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
