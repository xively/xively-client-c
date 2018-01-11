/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_ITEST_MQTTLOGIC_LAYER_H__
#define __XI_ITEST_MQTTLOGIC_LAYER_H__

extern int xi_itest_mqttlogic_layer_setup( void** state );
extern int xi_itest_mqttlogic_layer_teardown( void** state );

extern void
xi_itest_mqtt_logic_layer__backoff_class_error_PUSH__layerchain_closure_is_expected(
    void** state );
extern void
xi_itest_mqtt_logic_layer__backoff_class_error_PULL__layerchain_closure_is_expected(
    void** state );
extern void
xi_itest_mqtt_logic_layer__subscribe_success__success_suback_callback_invocation(
    void** state );
extern void
xi_itest_mqtt_logic_layer__subscribe_failure__failed_suback_callback_invocation(
    void** state );
extern void
xi_itest_mqtt_logic_layer__subscribe_success__success_message_callback_invocation(
    void** state );
extern void
xi_itest_mqtt_logic_layer__persistant_session__unsent_messages_not_prereserved(
    void** state );
extern void
xi_itest_mqtt_logic_layer__persistant_session__failure_unsent_message_are_not_resend_after_reconnect(
    void** state );
extern void
xi_itest_mqtt_logic_layer__persistant_session__success_unacked_messages_are_resend_after_reconnect(
    void** state );

#ifdef XI_MOCK_TEST_PREPROCESSOR_RUN
struct CMUnitTest xi_itests_mqttlogic_layer[] = {
    cmocka_unit_test_setup_teardown(
        xi_itest_mqtt_logic_layer__backoff_class_error_PUSH__layerchain_closure_is_expected,
        xi_itest_mqttlogic_layer_setup,
        xi_itest_mqttlogic_layer_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_mqtt_logic_layer__backoff_class_error_PULL__layerchain_closure_is_expected,
        xi_itest_mqttlogic_layer_setup,
        xi_itest_mqttlogic_layer_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_mqtt_logic_layer__subscribe_success__success_suback_callback_invocation,
        xi_itest_mqttlogic_layer_setup,
        xi_itest_mqttlogic_layer_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_mqtt_logic_layer__subscribe_failure__failed_suback_callback_invocation,
        xi_itest_mqttlogic_layer_setup,
        xi_itest_mqttlogic_layer_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_mqtt_logic_layer__persistant_session__unsent_messages_not_prereserved,
        xi_itest_mqttlogic_layer_setup,
        xi_itest_mqttlogic_layer_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_mqtt_logic_layer__persistant_session__failure_unsent_message_are_not_resend_after_reconnect,
        xi_itest_mqttlogic_layer_setup,
        xi_itest_mqttlogic_layer_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_mqtt_logic_layer__persistant_session__success_unacked_messages_are_resend_after_reconnect,
        xi_itest_mqttlogic_layer_setup,
        xi_itest_mqttlogic_layer_teardown )};
#endif

#endif /* __XI_ITEST_MQTTLOGIC_LAYER_H__ */
