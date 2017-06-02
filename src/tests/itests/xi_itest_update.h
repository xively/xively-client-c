/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_ITEST_UPDATE_H__
#define __XI_ITEST_UPDATE_H__

extern int xi_itest_update_setup( void** state );
extern int xi_itest_update_teardown( void** state );

extern void xi_itest_update__basic_flow__SFT_protocol_intact( void** state );


#ifdef XI_MOCK_TEST_PREPROCESSOR_RUN
struct CMUnitTest xi_itests_update[] = {
    cmocka_unit_test_setup_teardown( xi_itest_update__basic_flow__SFT_protocol_intact,
                                     xi_itest_update_setup,
                                     xi_itest_update_teardown )};

#endif

#endif // __XI_ITEST_UPDATE_H__
