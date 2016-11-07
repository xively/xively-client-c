/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_MQTT_LOGIC_LAYER_PUBLISH_Q0_COMMAND_H__
#define __XI_MQTT_LOGIC_LAYER_PUBLISH_Q0_COMMAND_H__

#include "xi_layer_api.h"

#ifdef __cplusplus
extern "C" {
#endif

xi_state_t do_mqtt_publish_q0( void* ctx, /* should be the context of the logic layer */
                               void* data,
                               xi_state_t state,
                               void* msg_data );

#ifdef __cplusplus
}
#endif

#endif /* __XI_MQTT_LOGIC_LAYER_PUBLISH_Q0_COMMAND_H__ */
