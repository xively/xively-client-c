/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_ITEST_MOCKBROKER_LAYERCHAIN_H__
#define __XI_ITEST_MOCKBROKER_LAYERCHAIN_H__

#include "xi_itest_mock_broker_layer.h"
#include "xi_layer_macros.h"
#include "xi_mqtt_codec_layer.h"
#include "xi_layer_default_functions.h"

enum xi_mock_broker_codec_layer_stack_order_e
{
    XI_LAYER_TYPE_MOCKBROKER_BOTTOM = 0,
    XI_LAYER_TYPE_MOCKBROKER_MQTT_CODEC,
    XI_LAYER_TYPE_MOCKBROKER_TOP
};

#define XI_MOCK_BROKER_CODEC_LAYER_CHAIN                                                 \
    XI_LAYER_TYPE_MOCKBROKER_BOTTOM                                                      \
    , XI_LAYER_TYPE_MOCKBROKER_MQTT_CODEC, XI_LAYER_TYPE_MOCKBROKER_TOP

XI_DECLARE_LAYER_TYPES_BEGIN( itest_mock_broker_codec_layer_chain )
XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_MOCKBROKER_BOTTOM,
                    xi_mock_broker_secondary_layer_push,
                    xi_mock_broker_secondary_layer_pull,
                    xi_mock_broker_secondary_layer_close,
                    xi_mock_broker_secondary_layer_close_externally,
                    xi_mock_broker_secondary_layer_init,
                    xi_mock_broker_secondary_layer_connect,
                    xi_layer_default_post_connect )
, XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_MOCKBROKER_MQTT_CODEC,
                      xi_mqtt_codec_layer_push,
                      xi_mqtt_codec_layer_pull,
                      xi_mqtt_codec_layer_close,
                      xi_mqtt_codec_layer_close_externally,
                      xi_mqtt_codec_layer_init,
                      xi_mqtt_codec_layer_connect,
                      xi_layer_default_post_connect ),
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_MOCKBROKER_TOP,
                        xi_mock_broker_layer_push,
                        xi_mock_broker_layer_pull,
                        xi_mock_broker_layer_close,
                        xi_mock_broker_layer_close_externally,
                        xi_mock_broker_layer_init,
                        xi_mock_broker_layer_connect,
                        xi_layer_default_post_connect ) XI_DECLARE_LAYER_TYPES_END()

        XI_DECLARE_LAYER_CHAIN_SCHEME( XI_LAYER_CHAIN_MOCK_BROKER_CODEC,
                                       XI_MOCK_BROKER_CODEC_LAYER_CHAIN );

#endif /* __XI_ITEST_MOCKBROKER_LAYERCHAIN_H__ */
