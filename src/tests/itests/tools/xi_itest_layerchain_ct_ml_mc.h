/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_ITEST_LAYERCHAIN_CT_ML_MC_H__
#define __XI_ITEST_LAYERCHAIN_CT_ML_MC_H__

#include "xi_layer_macros.h"

#include "xi_mock_layer_tls_prev.h"
#include "xi_itest_mock_broker_layer.h"
#include "xi_mqtt_codec_layer.h"
#include "xi_mqtt_logic_layer.h"
#include "xi_control_topic_layer.h"
#include "xi_layer_default_functions.h"

enum xi_ct_ml_mc_layer_stack_order_e
{
    XI_LAYER_TYPE_MOCK_IO = 0,
    XI_LAYER_TYPE_MOCK_BROKER,
    XI_LAYER_TYPE_MQTT_CODEC_SUT,
    XI_LAYER_TYPE_MQTT_LOGIC_SUT,
    XI_LAYER_TYPE_CONTROL_TOPIC_SUT
};

#define XI_CT_ML_MC_LAYER_CHAIN                                                          \
    XI_LAYER_TYPE_MOCK_IO                                                                \
    , XI_LAYER_TYPE_MOCK_BROKER, XI_LAYER_TYPE_MQTT_CODEC_SUT,                           \
        XI_LAYER_TYPE_MQTT_LOGIC_SUT, XI_LAYER_TYPE_CONTROL_TOPIC_SUT

XI_DECLARE_LAYER_TYPES_BEGIN( itest_ct_ml_mc_layer_chain )
XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_MOCK_IO,
                    xi_mock_layer_tls_prev_push,
                    xi_mock_layer_tls_prev_pull,
                    xi_mock_layer_tls_prev_close,
                    xi_mock_layer_tls_prev_close_externally,
                    xi_mock_layer_tls_prev_init,
                    xi_mock_layer_tls_prev_connect,
                    xi_layer_default_post_connect )
, XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_MOCK_BROKER,
                      xi_mock_broker_layer_push,
                      xi_mock_broker_layer_pull,
                      xi_mock_broker_layer_close,
                      xi_mock_broker_layer_close_externally,
                      xi_mock_broker_layer_init,
                      xi_mock_broker_layer_connect,
                      xi_layer_default_post_connect ),
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_MQTT_CODEC_SUT,
                        xi_mqtt_codec_layer_push,
                        xi_mqtt_codec_layer_pull,
                        xi_mqtt_codec_layer_close,
                        xi_mqtt_codec_layer_close_externally,
                        xi_mqtt_codec_layer_init,
                        xi_mqtt_codec_layer_connect,
                        xi_layer_default_post_connect ),
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_MQTT_LOGIC_SUT,
                        xi_mqtt_logic_layer_push,
                        xi_mqtt_logic_layer_pull,
                        xi_mqtt_logic_layer_close,
                        xi_mqtt_logic_layer_close_externally,
                        xi_mqtt_logic_layer_init,
                        xi_mqtt_logic_layer_connect,
                        xi_layer_default_post_connect ),
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_CONTROL_TOPIC_SUT,
                        xi_control_topic_layer_push,
                        xi_control_topic_layer_pull,
                        xi_control_topic_layer_close,
                        xi_control_topic_layer_close_externally,
                        xi_control_topic_layer_init,
                        xi_control_topic_layer_connect,
                        xi_layer_default_post_connect ) XI_DECLARE_LAYER_TYPES_END()

        XI_DECLARE_LAYER_CHAIN_SCHEME( XI_LAYER_CHAIN_CT_ML_MC, XI_CT_ML_MC_LAYER_CHAIN );

#endif /* __XI_ITEST_LAYERCHAIN_CT_ML_MC_H__ */
