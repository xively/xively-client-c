/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_ITEST_MOCK_BROKER_LAYER_H__
#define __XI_ITEST_MOCK_BROKER_LAYER_H__

#include "xi_layer_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum xi_mock_broker_control_init_e {
    CONTROL_CONTINUE,
    CONTROL_ERROR,
    CONTROL_SKIP_CHECK_EXPECTED
} xi_mock_broker_control_t;

typedef struct xi_mock_broker_data_s
{
    const char* control_topic_name_broker_in;
    const char* control_topic_name_broker_out;

    /* two mock broker contexts (mock broker and the mock broker edge device handler)
     * alternate the execution between each other. This solves the MQTT over MQTT codec
     * (double MQTT encoding / decoding) */
    xi_layer_t* layer_tunneled_message_target;
    xi_layer_t* layer_output_target;

    xi_mqtt_message_t* last_tunneled_recvd_msg;
} xi_mock_broker_data_t;

/**
 * @name    mock broker primary and secondary layers
 * @brief   mock broker is used in integration tests for monitoring and driving
 * libxively's behavior
 *
 * These layers are part of a mock broker layer chain.
 * (e.g. mock broker primary layer <-> mqtt codec layer <-> mock broker secondary layer)
 * Primary layer push function receives emitted messages from libxively's layers
 * and does two things with that:
 * forwards it to mock io (which behaves like libxively's default io layer except of
 * networking)
 * and forward it for decoding to codec layer inside mock broker layer chain.
 * P
 */

/************************************************************************************
 * mock broker primary layer ********************************************************
************************************************************************************/
/**
 * @name    xi_mock_broker_layer_push
 * @brief   receives mqtt encoded messages from libxively's layers
 *
 * Receives emitted messages from libxively's layers and does two things with that:
 * forwards it to mock io (which behaves like libxively's default io layer except of
 * networking)
 * and forwards it for decoding to codec layer inside mock broker layer chain.
 * Instrumented function: can be monitored and driven by test case.
 */
xi_state_t xi_mock_broker_layer_push( void* context, void* data, xi_state_t state );

/**
 * @name    xi_mock_broker_layer_pull
 * @brief   contains mock broker behavior, receives mqtt decoded messages from mock broker
 * layer chain
 *
 * Receives decoded messages which were previously sent for decode by push function.
 * Implements the mock broker behavior, can be driven by preset values by test case,
 * constructs broker reactions on incoming messages, send replies for encoding to
 * mqtt codec layer inside the mock broker layer chain.
 * Instrumented function: can be monitored and driven by test case.
 */
xi_state_t xi_mock_broker_layer_pull( void* context, void* data, xi_state_t state );

xi_state_t xi_mock_broker_layer_close( void* context, void* data, xi_state_t state );

xi_state_t
xi_mock_broker_layer_close_externally( void* context, void* data, xi_state_t state );

/**
 * @name    xi_mock_broker_layer_init
 * @brief   mock instrumented function to mimic different behaviours
 */
xi_state_t xi_mock_broker_layer_init( void* context, void* data, xi_state_t state );

/**
 * @name    xi_mock_broker_layer_connect
 * @brief   mock instrumented function to mimic different behaviours
 */
xi_state_t xi_mock_broker_layer_connect( void* context, void* data, xi_state_t state );

/************************************************************************************
 * mock broker secondary layer ******************************************************
************************************************************************************/
/**
* @name    xi_mock_broker_secondary_layer_push
* @brief   receives mqtt encoded messages from mock broker layer chain
*
* Receives emitted messages from mock broker layer chain and forwards it to
* libxively's layer chain.
* Instrumented function: can be monitored and driven by test case.
*/
xi_state_t
xi_mock_broker_secondary_layer_push( void* context, void* data, xi_state_t state );

xi_state_t
xi_mock_broker_secondary_layer_pull( void* context, void* data, xi_state_t in_out_state );

xi_state_t xi_mock_broker_secondary_layer_close( void* context,
                                                 void* data,
                                                 xi_state_t in_out_state );

xi_state_t xi_mock_broker_secondary_layer_close_externally( void* context,
                                                            void* data,
                                                            xi_state_t in_out_state );

xi_state_t
xi_mock_broker_secondary_layer_init( void* context, void* data, xi_state_t in_out_state );

xi_state_t xi_mock_broker_secondary_layer_connect( void* context,
                                                   void* data,
                                                   xi_state_t in_out_state );

#ifdef __cplusplus
}
#endif

#endif /* __XI_ITEST_MOCK_BROKER_LAYER_H__ */
