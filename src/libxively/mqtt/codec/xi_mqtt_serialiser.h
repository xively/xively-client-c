/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_MQTT_SERIALISER_H__
#define __XI_MQTT_SERIALISER_H__

#include "xi_mqtt_errors.h"
#include "xi_mqtt_message.h"

#ifdef __cplusplus
extern "C" {
#endif

/* @TODO: is the xi_mqtt_buffer still used? */

typedef enum xi_mqtt_serialiser_rc_e {
    XI_MQTT_SERIALISER_RC_ERROR,
    XI_MQTT_SERIALISER_RC_SUCCESS,
} xi_mqtt_serialiser_rc_t;

typedef struct xi_mqtt_serialiser_s
{
    xi_mqtt_error_t error;
} xi_mqtt_serialiser_t;

void xi_mqtt_serialiser_init( xi_mqtt_serialiser_t* serialiser );

xi_state_t xi_mqtt_serialiser_size( size_t* msg_len,
                                    size_t* remaining_len,
                                    size_t* publish_payload_len,
                                    xi_mqtt_serialiser_t* serialiser,
                                    const xi_mqtt_message_t* message );

xi_mqtt_serialiser_rc_t xi_mqtt_serialiser_write( xi_mqtt_serialiser_t* serialiser,
                                                  const xi_mqtt_message_t* message,
                                                  xi_data_desc_t* buffer,
                                                  const size_t message_len,
                                                  const size_t remaining_len );

#ifdef __cplusplus
}
#endif

#endif /* __XI_MQTT_SERIALISER_H__ */
