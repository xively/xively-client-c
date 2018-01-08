/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_ITEST_TLS_ERROR_H__
#define __XI_ITEST_TLS_ERROR_H__

extern int xi_itest_tls_error_setup( void** state );
extern int xi_itest_tls_error_teardown( void** state );

extern void xi_itest_tls_error__connection_flow__basic_checks( void** state );
extern void
xi_itest_tls_error__tls_init_and_connect_errors__graceful_error_handling( void** state );
extern void
xi_itest_tls_error__tls_push_CONNECT_errors__graceful_error_handling( void** state );
extern void
xi_itest_tls_error__tls_push_infinite_SUBSCRIBE_errors__reSUBSCRIBE_occurs_once_in_a_second(
    void** state );
extern void
xi_itest_tls_error__tls_push_SUBSCRIBE_errors__graceful_error_handling( void** state );
extern void
xi_itest_tls_error__tls_push_PUBLISH_errors__graceful_error_handling( void** state );
extern void
xi_itest_tls_error__tls_pull_CONNACK_errors__graceful_error_handling( void** state );
extern void
xi_itest_tls_error__tls_pull_SUBACK_errors__graceful_error_handling( void** state );
extern void
xi_itest_tls_error__tls_pull_PUBACK_errors__graceful_error_handling( void** state );

#ifdef XI_MOCK_TEST_PREPROCESSOR_RUN
struct CMUnitTest xi_itests_tls_error[] = {
    cmocka_unit_test_setup_teardown( xi_itest_tls_error__connection_flow__basic_checks,
                                     xi_itest_tls_error_setup,
                                     xi_itest_tls_error_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_tls_error__tls_init_and_connect_errors__graceful_error_handling,
        xi_itest_tls_error_setup,
        xi_itest_tls_error_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_tls_error__tls_push_CONNECT_errors__graceful_error_handling,
        xi_itest_tls_error_setup,
        xi_itest_tls_error_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_tls_error__tls_push_infinite_SUBSCRIBE_errors__reSUBSCRIBE_occurs_once_in_a_second,
        xi_itest_tls_error_setup,
        xi_itest_tls_error_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_tls_error__tls_push_SUBSCRIBE_errors__graceful_error_handling,
        xi_itest_tls_error_setup,
        xi_itest_tls_error_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_tls_error__tls_push_PUBLISH_errors__graceful_error_handling,
        xi_itest_tls_error_setup,
        xi_itest_tls_error_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_tls_error__tls_pull_CONNACK_errors__graceful_error_handling,
        xi_itest_tls_error_setup,
        xi_itest_tls_error_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_tls_error__tls_pull_SUBACK_errors__graceful_error_handling,
        xi_itest_tls_error_setup,
        xi_itest_tls_error_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_tls_error__tls_pull_PUBACK_errors__graceful_error_handling,
        xi_itest_tls_error_setup,
        xi_itest_tls_error_teardown )};
#endif

#endif /* __XI_ITEST_TLS_ERROR_H__ */
