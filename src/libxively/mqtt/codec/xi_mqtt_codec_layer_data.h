/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_MQTT_CODEC_LAYER_DATA_H__
#define __XI_MQTT_CODEC_LAYER_DATA_H__

#include "xi_mqtt_parser.h"
#include "xi_vector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xi_mqtt_codec_layer_task_s
{
    struct xi_mqtt_codec_layer_task_s* __next;
    xi_mqtt_message_t* msg;
    uint16_t msg_id;
    xi_mqtt_type_t msg_type;
} xi_mqtt_codec_layer_task_t;

typedef struct xi_mqtt_codec_layer_data_s
{
    xi_mqtt_message_t* msg;
    xi_mqtt_codec_layer_task_t* task_queue;
    xi_mqtt_parser_t parser;
    xi_state_t local_state;
    uint16_t msg_id;
    xi_mqtt_type_t msg_type;
    uint16_t pull_cs;
    uint16_t push_cs;
} xi_mqtt_codec_layer_data_t;

/**
 * @brief xi_mqtt_code_layer_make_task
 *
 * Helper ctor like function to create mqtt codec layer's task
 *
 * @param msg
 * @return
 */
extern xi_mqtt_codec_layer_task_t*
xi_mqtt_codec_layer_make_task( xi_mqtt_message_t* msg );

/**
 * @brief xi_mqtt_codec_layer_activate_task
 *
 * After the task activation certain steps must be performed
 * like detaching the msg from the task so that the msg memory
 * won't be double deleted in case of an error or system shutdown
 *
 * @param task
 * @return msg
 */
xi_mqtt_message_t* xi_mqtt_codec_layer_activate_task( xi_mqtt_codec_layer_task_t* task );

/**
 * @brief xi_mqtt_codec_layer_continue_task
 *
 * When the task continues execution the msg memory must be restored
 * so that msg lifetime can be controled by the task
 *
 * @param task
 * @param msg
 */
void xi_mqtt_codec_layer_continue_task( xi_mqtt_codec_layer_task_t* task,
                                        xi_mqtt_message_t* msg );

/**
 * @brief xi_mqtt_code_layer_free_task
 *
 * Helper dtor like function
 *
 * @param task
 * @return
 */
extern void xi_mqtt_codec_layer_free_task( xi_mqtt_codec_layer_task_t** task );

#ifdef __cplusplus
}
#endif

#endif /* __XI_MQTT_CODEC_LAYER_DATA_H__ */
