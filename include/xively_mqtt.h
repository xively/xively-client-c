/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XIVELY_MQTT_H__
#define __XIVELY_MQTT_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name xi_mqtt_retain_t
 * @brief MQTT Retain flag
 *
 * For details see Oasis Mqtt Variable header / Retain flag specs
 * http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718030
 */
typedef enum xi_mqtt_retain_e {
    XI_MQTT_RETAIN_FALSE = 0,
    XI_MQTT_RETAIN_TRUE  = 1,
} xi_mqtt_retain_t;

/**
 * @name xi_mqtt_qos_t
 * @brief MQTT Quality of Service levels
 *
 * For details see Oasis Mqtt QoS specs
 * http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718099
 */
typedef enum xi_mqtt_qos_e {
    XI_MQTT_QOS_AT_MOST_ONCE  = 0,
    XI_MQTT_QOS_AT_LEAST_ONCE = 1,
    XI_MQTT_QOS_EXACTLY_ONCE  = 2,
} xi_mqtt_qos_t;

/**
 * @name xi_mqtt_suback_status_t
 * @brief MQTT SUBACK status
 *
 * For details see Oasis Mqtt SUBACK payload
 * http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718068
 */
typedef enum xi_mqtt_suback_status_e {
    XI_MQTT_QOS_0_GRANTED = 0x00,
    XI_MQTT_QOS_1_GRANTED = 0x01,
    XI_MQTT_QOS_2_GRANTED = 0x02,
    XI_MQTT_SUBACK_FAILED = 0x80
} xi_mqtt_suback_status_t;

/**
 * @name xi_mqtt_dup_t
 * @brief MQTT DUP flag
 *
 * For details see Oasis Mqtt DUP flag
 * http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718038
 */
typedef enum xi_mqtt_dup_e {
    XI_MQTT_DUP_FALSE = 0,
    XI_MQTT_DUP_TRUE  = 1,
} xi_mqtt_dup_t;

#ifdef __cplusplus
}
#endif

#endif /* __XIVELY_MQTT_H__ */
