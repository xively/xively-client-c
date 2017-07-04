/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_ITEST_SFT_H__
#define __XI_ITEST_SFT_H__

extern int xi_itest_sft_setup( void** state );
extern int xi_itest_sft_teardown( void** state );

extern void
xi_itest_sft__client_doesnt_start_SFT_if_no_update_file_is_set( void** state );
extern void
xi_itest_sft__basic_flow__SFT_with_happy_broker__protocol_intact( void** state );
extern void
xi_itest_sft__broker_replies_FILE_INFO_on_FILE_GET_CHUNK__client_does_not_crash_or_leak(
    void** state );
extern void
xi_itest_sft__broker_replies_FUA_on_FILE_GET_CHUNK__client_does_not_crash_or_leak(
    void** state );
extern void xi_itest_sft__manymany_updateable_files( void** state );


#ifdef XI_MOCK_TEST_PREPROCESSOR_RUN
struct CMUnitTest xi_itests_sft[] = {
    cmocka_unit_test_setup_teardown(
        xi_itest_sft__client_doesnt_start_SFT_if_no_update_file_is_set,
        xi_itest_sft_setup,
        xi_itest_sft_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_sft__basic_flow__SFT_with_happy_broker__protocol_intact,
        xi_itest_sft_setup,
        xi_itest_sft_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_sft__broker_replies_FILE_INFO_on_FILE_GET_CHUNK__client_does_not_crash_or_leak,
        xi_itest_sft_setup,
        xi_itest_sft_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_sft__broker_replies_FUA_on_FILE_GET_CHUNK__client_does_not_crash_or_leak,
        xi_itest_sft_setup,
        xi_itest_sft_teardown ),
    cmocka_unit_test_setup_teardown( xi_itest_sft__manymany_updateable_files,
                                     xi_itest_sft_setup,
                                     xi_itest_sft_teardown )};

#endif

#endif /* __XI_ITEST_SFT_H__ */
