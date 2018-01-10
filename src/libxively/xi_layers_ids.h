/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_LAYERS_IDS_H__
#define __XI_LAYERS_IDS_H__

#ifdef __cplusplus
extern "C" {
#endif

enum xi_layer_stack_order_e
{
    XI_LAYER_TYPE_IO = 0
#ifndef XI_NO_TLS_LAYER
    ,
    XI_LAYER_TYPE_TLS
#endif
    ,
    XI_LAYER_TYPE_MQTT_CODEC,
    XI_LAYER_TYPE_MQTT_LOGIC,
    XI_LAYER_TYPE_CONTROL_TOPIC
};

#ifdef __cplusplus
}
#endif

#endif /* __XI_LAYERS_IDS_H__ */
