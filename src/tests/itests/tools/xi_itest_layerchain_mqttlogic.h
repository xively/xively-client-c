/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_ITEST_LAYERCHAIN_MQTTLOGIC_LAYER_H__
#define __XI_ITEST_LAYERCHAIN_MQTTLOGIC_LAYER_H__

#include "xi_layer_macros.h"

#include "xi_mock_layer_mqttlogic_prev.h"
#include "xi_mqtt_logic_layer.h"
#include "xi_mock_layer_mqttlogic_next.h"
#include "xi_layer_default_functions.h"

enum xi_mqttlogic_layer_stack_order_e
{
    XI_LAYER_TYPE_MOCK_MQTTLOGIC_PREV = 0,
    XI_LAYER_TYPE_SUT_MQTTLOGIC,
    XI_LAYER_TYPE_MOCK_MQTTLOGIC_NEXT
};

#define XI_MQTTLOGIC_LAYER_CHAIN                                                         \
    XI_LAYER_TYPE_MOCK_MQTTLOGIC_PREV                                                    \
    , XI_LAYER_TYPE_SUT_MQTTLOGIC, XI_LAYER_TYPE_MOCK_MQTTLOGIC_NEXT

XI_DECLARE_LAYER_TYPES_BEGIN( xi_itest_layer_chain_mqttlogic )
XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_MOCK_MQTTLOGIC_PREV,
                    xi_mock_layer_mqttlogic_prev_push,
                    xi_mock_layer_mqttlogic_prev_pull,
                    xi_mock_layer_mqttlogic_prev_close,
                    xi_mock_layer_mqttlogic_prev_close_externally,
                    xi_mock_layer_mqttlogic_prev_init,
                    xi_mock_layer_mqttlogic_prev_connect,
                    xi_layer_default_post_connect )
, XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_SUT_MQTTLOGIC,
                      xi_mqtt_logic_layer_push,
                      xi_mqtt_logic_layer_pull,
                      xi_mqtt_logic_layer_close,
                      xi_mqtt_logic_layer_close_externally,
                      xi_mqtt_logic_layer_init,
                      xi_mqtt_logic_layer_connect,
                      xi_mqtt_logic_layer_post_connect ),
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_MOCK_MQTTLOGIC_NEXT,
                        xi_mock_layer_mqttlogic_next_push,
                        xi_mock_layer_mqttlogic_next_pull,
                        xi_mock_layer_mqttlogic_next_close,
                        xi_mock_layer_mqttlogic_next_close_externally,
                        xi_mock_layer_mqttlogic_next_init,
                        xi_mock_layer_mqttlogic_next_connect,
                        xi_layer_default_post_connect ) XI_DECLARE_LAYER_TYPES_END()

        XI_DECLARE_LAYER_CHAIN_SCHEME( XI_LAYER_CHAIN_MQTTLOGIC,
                                       XI_MQTTLOGIC_LAYER_CHAIN );

#endif /* __XI_ITEST_LAYERCHAIN_MQTTLOGIC_LAYER_H__ */
