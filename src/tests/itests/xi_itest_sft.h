/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_ITEST_SFT_H__
#define __XI_ITEST_SFT_H__

extern int xi_itest_sft_setup( void** fixture_void );
extern int xi_itest_sft_teardown( void** fixture_void );

extern void
xi_itest_sft__client_doesnt_start_SFT_if_no_update_file_is_set( void** fixture_void );
extern void
xi_itest_sft__basic_flow__SFT_with_happy_broker__protocol_intact( void** fixture_void );
extern void
xi_itest_sft__broker_replies_FILE_INFO_on_FILE_GET_CHUNK__client_does_not_crash_or_leak(
    void** fixture_void );
extern void xi_itest_sft__broker_replies_FUA_on_FILE_GET_CHUNK__client_processes_2nd_FUA(
    void** fixture_void );
extern void xi_itest_sft__manymany_updateable_files( void** fixture_void );
extern void xi_itest_sft__firmware_bin_received__firmware_test_commit_triggered(
    void** fixture_void );
extern void
xi_itest_sft__revision_non_volatile_storage__proper_value_stored( void** fixture_void );
extern void xi_itest_sft__checksum_mismatch__update_process_exits( void** fixture_void );

extern void xi_itest_sft__custom_URL_download__single_file( void** fixture_void );
extern void
xi_itest_sft__custom_URL_download__single_file_firmware( void** fixture_void );
extern void
xi_itest_sft__custom_URL_download__two_files_oneURL_oneMQTT( void** fixture_void );
extern void
xi_itest_sft__custom_URL_download__three_files_URLFAIL_fallback_URLREJECTED_fallback_URLSUCCEEDS(
    void** fixture_void );
extern void
xi_itest_sft__custom_URL_download__two_files__URLREJECTED__no_fallback_available(
    void** fixture_void );
extern void xi_itest_sft__custom_URL_download__two_files__URLFAILS__no_fallback_available(
    void** fixture_void );
extern void
xi_itest_sft__custom_URL_download_checksum_validation__single_file( void** fixture_void );


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
        xi_itest_sft__broker_replies_FUA_on_FILE_GET_CHUNK__client_processes_2nd_FUA,
        xi_itest_sft_setup,
        xi_itest_sft_teardown ),
    cmocka_unit_test_setup_teardown( xi_itest_sft__manymany_updateable_files,
                                     xi_itest_sft_setup,
                                     xi_itest_sft_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_sft__firmware_bin_received__firmware_test_commit_triggered,
        xi_itest_sft_setup,
        xi_itest_sft_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_sft__revision_non_volatile_storage__proper_value_stored,
        xi_itest_sft_setup,
        xi_itest_sft_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_sft__checksum_mismatch__update_process_exits,
        xi_itest_sft_setup,
        xi_itest_sft_teardown ),
    cmocka_unit_test_setup_teardown( xi_itest_sft__custom_URL_download__single_file,
                                     xi_itest_sft_setup,
                                     xi_itest_sft_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_sft__custom_URL_download__single_file_firmware,
        xi_itest_sft_setup,
        xi_itest_sft_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_sft__custom_URL_download__two_files_oneURL_oneMQTT,
        xi_itest_sft_setup,
        xi_itest_sft_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_sft__custom_URL_download__three_files_URLFAIL_fallback_URLREJECTED_fallback_URLSUCCEEDS,
        xi_itest_sft_setup,
        xi_itest_sft_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_sft__custom_URL_download__two_files__URLREJECTED__no_fallback_available,
        xi_itest_sft_setup,
        xi_itest_sft_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_sft__custom_URL_download__two_files__URLFAILS__no_fallback_available,
        xi_itest_sft_setup,
        xi_itest_sft_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_sft__custom_URL_download_checksum_validation__single_file,
        xi_itest_sft_setup,
        xi_itest_sft_teardown )};

#endif

#endif /* __XI_ITEST_SFT_H__ */
