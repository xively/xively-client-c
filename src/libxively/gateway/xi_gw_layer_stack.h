/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_GW_LAYER_STACK_H__
#define __XI_GW_LAYER_STACK_H__

#include "xi_layer_macros.h"

#include "xi_layer_default_functions.h"

#include <xi_gw_glue_layer.h>
#include <xi_mqtt_codec_layer.h>
#include <xi_mqtt_logic_layer.h>
#include <xi_control_topic_layer.h>
#include <xi_gw_gateway_layer.h>

enum xi_gw_layer_stack_order_e
{
    XI_LAYER_TYPE_GW_GATEWAY = 0,
    XI_LAYER_TYPE_GW_CONTROL_TOPIC,
    XI_LAYER_TYPE_GW_MQTT_LOGIC,
    XI_LAYER_TYPE_GW_MQTT_CODEC,
    XI_LAYER_TYPE_GW_TUNNEL
};

#define XI_GW_LAYER_CHAIN                                                                \
    XI_LAYER_TYPE_GW_GATEWAY, XI_LAYER_TYPE_GW_CONTROL_TOPIC,                            \
        XI_LAYER_TYPE_GW_MQTT_LOGIC, XI_LAYER_TYPE_GW_MQTT_CODEC,                        \
        XI_LAYER_TYPE_GW_TUNNEL

XI_DECLARE_LAYER_CHAIN_SCHEME( XI_LAYER_CHAIN_GW, XI_GW_LAYER_CHAIN );

XI_DECLARE_LAYER_TYPES_BEGIN( xi_gw_layer_chain )
XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_GW_TUNNEL,
                    xi_gw_glue_layer_push,
                    xi_gw_glue_layer_pull,
                    xi_gw_glue_layer_close,
                    xi_gw_glue_layer_close_externally,
                    xi_gw_glue_layer_init,
                    xi_gw_glue_layer_connect,
                    xi_layer_default_post_connect )
, XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_GW_MQTT_CODEC,
                      xi_mqtt_codec_layer_push,
                      xi_mqtt_codec_layer_pull,
                      xi_mqtt_codec_layer_close,
                      xi_mqtt_codec_layer_close_externally,
                      xi_mqtt_codec_layer_init,
                      xi_mqtt_codec_layer_connect,
                      xi_layer_default_post_connect ),
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_GW_MQTT_LOGIC,
                        xi_mqtt_logic_layer_push,
                        xi_mqtt_logic_layer_pull,
                        xi_mqtt_logic_layer_close,
                        xi_mqtt_logic_layer_close_externally,
                        xi_mqtt_logic_layer_init,
                        xi_mqtt_logic_layer_connect,
                        xi_mqtt_logic_layer_post_connect ),
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_GW_CONTROL_TOPIC,
                        xi_control_topic_layer_push,
                        xi_control_topic_layer_pull,
                        xi_control_topic_layer_close,
                        xi_control_topic_layer_close_externally,
                        xi_control_topic_layer_init,
                        xi_control_topic_layer_connect,
                        xi_layer_default_post_connect ),
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_GW_GATEWAY,
                        xi_gw_gateway_layer_push,
                        xi_gw_gateway_layer_pull,
                        xi_gw_gateway_layer_close,
                        xi_gw_gateway_layer_close_externally,
                        xi_gw_gateway_layer_init,
                        xi_gw_gateway_layer_connect,
                        xi_layer_default_post_connect ) XI_DECLARE_LAYER_TYPES_END()

#endif /* __XI_GW_LAYER_STACK_H__ */
