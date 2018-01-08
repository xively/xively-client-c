/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_MQTT_LOGIC_LAYER_DATA_H__
#define __XI_MQTT_LOGIC_LAYER_DATA_H__

#include "xi_connection_data.h"
#include "xi_data_desc.h"
#include "xi_event_dispatcher_api.h"
#include "xi_mqtt_message.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum xi_scenario_e {
    XI_MQTT_CONNECT = 0,
    XI_MQTT_PUBLISH,
    XI_MQTT_PUBACK,
    XI_MQTT_SUBSCRIBE,
    XI_MQTT_KEEPALIVE,
    XI_MQTT_SHUTDOWN
} xi_scenario_t;

typedef union {
    struct data_t_publish_t
    {
        char* topic;
        xi_data_desc_t* data;
        xi_mqtt_retain_t retain;
        xi_mqtt_dup_t dup;
    } publish;

    struct data_t_subscribe_t
    {
        char* topic;
        xi_event_handle_t handler;
        xi_mqtt_qos_t qos;
    } subscribe;

    struct data_t_shutdown_t
    {
        char placeholder; /* This is to meet requirements of the IAR ARM
                           * compiler */
    } shutdown;
} xi_mqtt_task_specific_data_t;

typedef struct xi_mqtt_logic_task_data_s
{
    struct xi_mqtt_logic_in_s
    {
        xi_scenario_t scenario;
        xi_mqtt_qos_t qos;
    } mqtt_settings;

    xi_mqtt_task_specific_data_t* data_u;

} xi_mqtt_logic_task_data_t;

typedef enum {
    XI_MQTT_LOGIC_TASK_NORMAL = 0,
    XI_MQTT_LOGIC_TASK_IMMEDIATE
} xi_mqtt_logic_task_priority_t;

typedef enum {
    XI_MQTT_LOGIC_TASK_SESSION_UNSET = 0,
    XI_MQTT_LOGIC_TASK_SESSION_DO_NOT_STORE,
    XI_MQTT_LOGIC_TASK_SESSION_STORE,
} xi_mqtt_logic_task_session_state_t;

typedef struct xi_mqtt_logic_task_s
{
    struct xi_mqtt_logic_task_s* __next;
    xi_time_event_handle_t timeout;
    xi_event_handle_t logic;
    xi_event_handle_t callback;
    xi_mqtt_logic_task_data_t data;
    xi_mqtt_logic_task_priority_t priority;
    xi_mqtt_logic_task_session_state_t session_state;
    uint16_t cs;
    uint16_t msg_id;
} xi_mqtt_logic_task_t;

typedef struct
{
    /* here we are going to store the mapping of the
     * handle functions versus the subscribed topics
     * so it's easy for the user to register his callback
     * for each of the subscribed topics */

    /* handle to the user idle function that suppose to */
    xi_mqtt_logic_task_t* q12_tasks_queue;
    xi_mqtt_logic_task_t* q12_recv_tasks_queue;
    xi_mqtt_logic_task_t* q0_tasks_queue;
    xi_mqtt_logic_task_t* current_q0_task;
    xi_vector_t* handlers_for_topics;
    xi_time_event_handle_t keepalive_event;
    uint16_t last_msg_id;
} xi_mqtt_logic_layer_data_t;

/* pseudo constructors */
extern xi_mqtt_logic_task_t*
xi_mqtt_logic_make_publish_task( const char* topic,
                                 xi_data_desc_t* data,
                                 const xi_mqtt_qos_t qos,
                                 const xi_mqtt_retain_t retain,
                                 xi_event_handle_t callback );

extern xi_mqtt_logic_task_t*
xi_mqtt_logic_make_subscribe_task( char* topic,
                                   const xi_mqtt_qos_t qos,
                                   xi_event_handle_t handler );

extern void
xi_mqtt_task_spec_data_free_publish_data( xi_mqtt_task_specific_data_t** data );

extern void
xi_mqtt_task_spec_data_free_subscribe_data( xi_mqtt_task_specific_data_t** data );

extern void
xi_mqtt_task_spec_data_free_subscribe_data_vec( union xi_vector_selector_u* data,
                                                void* arg );

extern xi_mqtt_logic_task_t* xi_mqtt_logic_make_shutdown_task( void );

/* pseudo destructor */
extern xi_mqtt_logic_task_t* xi_mqtt_logic_free_task_data( xi_mqtt_logic_task_t* task );

extern xi_mqtt_logic_task_t* xi_mqtt_logic_free_task( xi_mqtt_logic_task_t** task );

#ifdef __cplusplus
}
#endif

#endif /* __XI_MQTT_LOGIC_LAYER_DATA_H__ */
