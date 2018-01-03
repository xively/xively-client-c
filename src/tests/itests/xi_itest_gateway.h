/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_ITEST_GATEWAY_H__
#define __XI_ITEST_GATEWAY_H__

extern int xi_itest_gateway_setup( void** fixture_void );
extern int xi_itest_gateway_teardown( void** fixture_void );

extern void xi_itest_gateway__first( void** fixture_void );

#ifdef XI_MOCK_TEST_PREPROCESSOR_RUN
struct CMUnitTest xi_itests_gateway[] = {cmocka_unit_test_setup_teardown(
    xi_itest_gateway__first, xi_itest_gateway_setup, xi_itest_gateway_teardown )};

#endif

#endif /* __XI_ITEST_GATEWAY_H__ */
