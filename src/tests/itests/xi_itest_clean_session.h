/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef XI_ITEST_CLEAN_SESSION_H
#define XI_ITEST_CLEAN_SESSION_H

#include "xi_itest_helpers.h"

extern int xi_itest_clean_session_setup( void** state );
extern int xi_itest_clean_session_teardown( void** state );

extern void
xi_itest_test_valid_flow__handlers_vector_should_be_empty__init_with_clean_session(
    void** state );
extern void
xi_itest_test_valid_flow__handlers_vector_should_be_empty__init_with_continue_session(
    void** state );
extern void xi_itest_test_valid_flow__handlers_should_be_copied( void** state );
extern void
xi_itest_test_valid_flow__handlers_should_be_prereserved_across_initialization_deinitinialization(
    void** state );

#ifdef XI_MOCK_TEST_PREPROCESSOR_RUN
struct CMUnitTest xi_itests_clean_session[] = {
    cmocka_unit_test_setup_teardown(
        xi_itest_test_valid_flow__handlers_vector_should_be_empty__init_with_clean_session,
        xi_itest_clean_session_setup,
        xi_itest_clean_session_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_test_valid_flow__handlers_vector_should_be_empty__init_with_continue_session,
        xi_itest_clean_session_setup,
        xi_itest_clean_session_teardown ),
    cmocka_unit_test_setup_teardown( xi_itest_test_valid_flow__handlers_should_be_copied,
                                     xi_itest_clean_session_setup,
                                     xi_itest_clean_session_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_test_valid_flow__handlers_should_be_prereserved_across_initialization_deinitinialization,
        xi_itest_clean_session_setup,
        xi_itest_clean_session_teardown )};
#endif

#endif // XI_ITEST_CLEAN_SESSION_H
