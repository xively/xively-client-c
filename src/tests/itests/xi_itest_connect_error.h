/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_ITEST_CONNECT_ERROR_H__
#define __XI_ITEST_CONNECT_ERROR_H__

#include "xi_itest_helpers.h"

extern int xi_itest_connect_error_setup( void** fixture_void );
extern int xi_itest_connect_error_teardown( void** fixture_void );

extern void
xi_itest_test_valid_flow__call_connect_function_twice_in_a_row__second_call_returns_error(
    void** state );
extern void
xi_itest_test_valid_flow__call_connect_function_twice__second_call_returns_error(
    void** state );
extern void
xi_itest_test_valid_flow__call_is_context_connected_on_connecting_context__call_returns_false(
    void** state );    
extern void
xi_itest_test_valid_flow__call_is_context_connected_on_connected_context__call_returns_true(
    void** state );
extern void
xi_itest_test_valid_flow__call_is_context_connected_on_disconnecting_context__call_returns_false(
    void** state );
extern void
xi_itest_test_valid_flow__call_is_context_connected_on_disconnected_context__call_returns_false(
    void** state );
extern void
xi_itest_test_valid_flow__call_disconnect_twice_on_connected_context__second_call_should_return_error(
    void** state );
extern void
xi_itest_test_valid_flow__call_connect_function_then_disconnect_without_making_a_connection__shutdown_should_unregister_connect(
    void** state );

#ifdef XI_MOCK_TEST_PREPROCESSOR_RUN

struct CMUnitTest xi_itests_connect_error[] = {
    cmocka_unit_test_setup_teardown(
        xi_itest_test_valid_flow__call_connect_function_twice_in_a_row__second_call_returns_error,
        xi_itest_connect_error_setup,
        xi_itest_connect_error_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_test_valid_flow__call_connect_function_twice__second_call_returns_error,
        xi_itest_connect_error_setup,
        xi_itest_connect_error_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_test_valid_flow__call_disconnect_twice_on_connected_context__second_call_should_return_error,
        xi_itest_connect_error_setup,
        xi_itest_connect_error_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_test_valid_flow__call_connect_function_then_disconnect_without_making_a_connection__shutdown_should_unregister_connect,
        xi_itest_connect_error_setup,
        xi_itest_connect_error_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_test_valid_flow__call_is_context_connected_on_connecting_context__call_returns_false,
        xi_itest_connect_error_setup,
        xi_itest_connect_error_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_test_valid_flow__call_is_context_connected_on_connected_context__call_returns_true,
        xi_itest_connect_error_setup,
        xi_itest_connect_error_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_test_valid_flow__call_is_context_connected_on_disconnecting_context__call_returns_false,
        xi_itest_connect_error_setup,
        xi_itest_connect_error_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_test_valid_flow__call_is_context_connected_on_disconnected_context__call_returns_false,
        xi_itest_connect_error_setup,
        xi_itest_connect_error_teardown ),
};

#endif /* XI_MOCK_TEST_PREPROCESSOR_RUN */

#endif /* __XI_ITEST_CONNECT_ERROR_H__ */
