/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_ITEST_TLS_LAYER_H__
#define __XI_ITEST_TLS_LAYER_H__

/**
 * @name    xi_itest_tls_layer_setup
 * @brief   test case setup, called on start of each test case
 *
 * @param [in] state: test case specific custom data
 *
 * @retval 0              OK
 * @retval -1             tells cmocka error occurred
 */
extern int xi_itest_tls_layer_setup( void** state );

/**
 * @name    xi_itest_tls_layer_teardown
 * @brief   test case setup, called on finish of each test case
 *
 * @param [in] state: test case specific custom data
 *
 * @retval 0              OK
 * @retval -1             tells cmocka error occurred
 */
extern int xi_itest_tls_layer_teardown( void** state );

extern void xi_itest_tls_layer__null_host__graceful_closure( void** state );
extern void xi_itest_tls_layer__valid_dependencies__successful_init( void** state );
extern void xi_itest_tls_layer__bad_handshake_response__graceful_closure( void** state );

#ifdef XI_MOCK_TEST_PREPROCESSOR_RUN
struct CMUnitTest xi_itests_tls_layer[] = {
    cmocka_unit_test_setup_teardown( xi_itest_tls_layer__null_host__graceful_closure,
                                     xi_itest_tls_layer_setup,
                                     xi_itest_tls_layer_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_tls_layer__valid_dependencies__successful_init,
        xi_itest_tls_layer_setup,
        xi_itest_tls_layer_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_tls_layer__bad_handshake_response__graceful_closure,
        xi_itest_tls_layer_setup,
        xi_itest_tls_layer_teardown )};
#endif

#endif /* __XI_ITEST_TLS_LAYER_H__ */
