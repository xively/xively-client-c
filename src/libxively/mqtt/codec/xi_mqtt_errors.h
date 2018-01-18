/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_MQTT_ERRORS_H__
#define __XI_MQTT_ERRORS_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum xi_mqtt_error_e {
    XI_MQTT_ERROR_NONE                            = 0,
    XI_MQTT_ERROR_PARSER_INVALID_STATE            = 1,
    XI_MQTT_ERROR_PARSER_INVALID_REMAINING_LENGTH = 2,
    XI_MQTT_ERROR_PARSER_INVALID_MESSAGE_ID       = 3,
    XI_MQTT_ERROR_SERIALISER_INVALID_MESSAGE_ID   = 4,
} xi_mqtt_error_t;

#ifdef __cplusplus
}
#endif

#endif /* __XI_MQTT_ERRORS_H__ */
