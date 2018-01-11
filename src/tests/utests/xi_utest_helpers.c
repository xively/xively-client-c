/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"

#include "xively.h"
#include "xi_helpers.h"
#include "xi_memory_checks.h"
#include "xi_mqtt_logic_layer_data_helpers.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*-----------------------------------------------------------------------*/
/* HELPER TESTS                                                          */
/*-----------------------------------------------------------------------*/
#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

void test_helpers_xi_parse_payload_as_string_parse_empty_payload_help( void )
{
    const char topic[]     = "test_topic";
    const char content[]   = "";
    char* string_payload   = NULL;
    xi_mqtt_message_t* msg = NULL;

    xi_data_desc_t* message_payload = xi_make_desc_from_string_copy( content );

    xi_state_t local_state = XI_STATE_OK;
    XI_ALLOC_AT( xi_mqtt_message_t, msg, local_state );

    tt_want_int_op( fill_with_publish_data( msg, topic, message_payload,
                                            XI_MQTT_QOS_AT_LEAST_ONCE,
                                            XI_MQTT_RETAIN_FALSE, XI_MQTT_DUP_FALSE, 17 ),
                    ==, XI_STATE_OK );

    string_payload = xi_parse_message_payload_as_string( msg );

    tt_assert( string_payload != NULL );

    tt_want_int_op( strlen( string_payload ), ==, 0 );

err_handling:
end:
    XI_SAFE_FREE( string_payload );
    xi_free_desc( &message_payload );
    xi_mqtt_message_free( &msg );
    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
    ;
}

#endif

XI_TT_TESTGROUP_BEGIN( utest_helpers )

XI_TT_TESTCASE( test_helpers_xi_parse_payload_as_string_null_param, {
    char* result = xi_parse_message_payload_as_string( NULL );

    tt_assert( NULL == result );

end:
    XI_SAFE_FREE( result );
    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
    ;
} )

XI_TT_TESTCASE( test_helpers_xi_parse_payload_as_string_parse_legitmiate_payload, {
    const char topic[]     = "test_topic";
    const char content[]   = "0123456789";
    char* string_payload   = NULL;
    xi_mqtt_message_t* msg = NULL;


    xi_data_desc_t* message_payload = xi_make_desc_from_string_copy( content );

    xi_state_t local_state = XI_STATE_OK;
    XI_ALLOC_AT( xi_mqtt_message_t, msg, local_state );

    tt_want_int_op( fill_with_publish_data( msg, topic, message_payload,
                                            XI_MQTT_QOS_AT_LEAST_ONCE,
                                            XI_MQTT_RETAIN_FALSE, XI_MQTT_DUP_FALSE, 17 ),
                    ==, XI_STATE_OK );


    string_payload = xi_parse_message_payload_as_string( msg );

    tt_assert( string_payload != NULL );

    tt_want_int_op( strncmp( content, string_payload, 10 ), ==, 0 );
    tt_want_int_op( strlen( string_payload ), ==, 10 );

err_handling:
end:
    XI_SAFE_FREE( string_payload );
    xi_free_desc( &message_payload );
    xi_mqtt_message_free( &msg );
    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
    ;
} )

XI_TT_TESTCASE( test_helpers_xi_parse_payload_as_string_parse_empty_payload,
                { test_helpers_xi_parse_payload_as_string_parse_empty_payload_help(); } )

XI_TT_TESTCASE( test_helpers__xi_str_reposition_after_first_n_char__with_null_string, {
    xi_str_reposition_after_first_n_char( 'f', 100, NULL );
    const char* str = NULL;
    xi_str_reposition_after_first_n_char( 'f', 100, &str );
} )

XI_TT_TESTCASE( test_helpers__xi_str_reposition_after_first_n_char__empty_string, {
    const char* str = "";
    xi_str_reposition_after_first_n_char( 'f', 100, &str );
} )

XI_TT_TESTCASE( test_helpers__xi_str_reposition_after_first_n_char__reposition_to_the_end,
                {
                    const char* str = "find/this/pos->";
                    xi_str_reposition_after_first_n_char( '/', 3, &str );
                    tt_want_str_op( str, ==, "" );
                } )

XI_TT_TESTCASE(
    test_helpers__xi_str_reposition_after_first_n_char__reposition_to_the_end_due_to_no_char_found,
    {
        const char* str = "find/this/pos->";
        xi_str_reposition_after_first_n_char( '&', 1, &str );
        tt_want_str_op( str, ==, "" );
    } )

XI_TT_TESTCASE( test_helpers__xi_str_reposition_after_first_n_char__reposition_to_middle,
                {
                    const char* str = "find/this/pos->";
                    xi_str_reposition_after_first_n_char( '/', 1, &str );
                    tt_want_str_op( str, ==, "this/pos->" );
                } )

XI_TT_TESTCASE(
    test_helpers__xi_str_reposition_after_first_n_char__no_reposition_due_to_0_num_of_chars_to_find,
    {
        const char* str = "find/this/pos->";
        xi_str_reposition_after_first_n_char( '/', 0, &str );
        tt_want_str_op( str, ==, "find/this/pos->" );
    } )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
