/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_ITEST_MQTT_KEEPALIVE_H__
#define __XI_ITEST_MQTT_KEEPALIVE_H__

extern int xi_itest_mqtt_keepalive_setup( void** state );
extern int xi_itest_mqtt_keepalive_teardown( void** state );

extern void
xi_itest_mqtt_keepalive__PINGREQ_failed_to_send__client_disconnects_after_keepalive_seconds(
    void** state );
extern void
xi_itest_mqtt_keepalive__PINGREQ_failed_to_send__broker_disconnects_first(
    void** state );

#ifdef XI_MOCK_TEST_PREPROCESSOR_RUN
struct CMUnitTest xi_itests_mqtt_keepalive[] = {
    cmocka_unit_test_setup_teardown(
        xi_itest_mqtt_keepalive__PINGREQ_failed_to_send__client_disconnects_after_keepalive_seconds,
        xi_itest_mqtt_keepalive_setup,
        xi_itest_mqtt_keepalive_teardown ),
    cmocka_unit_test_setup_teardown(
        xi_itest_mqtt_keepalive__PINGREQ_failed_to_send__broker_disconnects_first,
        xi_itest_mqtt_keepalive_setup,
        xi_itest_mqtt_keepalive_teardown ),
};
#endif

#endif /* __XI_ITEST_MQTT_KEEPALIVE_H__ */
