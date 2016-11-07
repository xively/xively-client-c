/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_MQTT_LOGIC_LAYER_TASK_HELPERS_H__
#define __XI_MQTT_LOGIC_LAYER_TASK_HELPERS_H__

#include "xi_event_dispatcher_api.h"
#include "xi_mqtt_logic_layer_data.h"
#include "xi_layer_api.h"
#include "xi_list.h"
#include "xi_globals.h"
#include "xi_io_timeouts.h"

#ifdef __cplusplus
extern "C" {
#endif

xi_state_t xi_mqtt_logic_layer_finalize_task( xi_layer_connectivity_t* context,
                                              xi_mqtt_logic_task_t* task );

xi_state_t xi_mqtt_logic_layer_run_next_q0_task( void* data );

void xi_mqtt_logic_task_defer_users_callback( void* context,
                                              xi_mqtt_logic_task_t* task,
                                              xi_state_t state );

#define CMP_TASK_MSG_ID( task, id ) ( task->msg_id == id )

void cancel_task_timeout( xi_mqtt_logic_task_t* task, xi_layer_connectivity_t* context );

void signal_task( xi_mqtt_logic_task_t* task, xi_state_t state );

/* if sent after layer_data cleared
 * will stop each task and free tasks memory */
void abort_task( xi_mqtt_logic_task_t* task );

void resend_task( xi_mqtt_logic_task_t* task );

void timeout_task( xi_mqtt_logic_task_t* task );

void set_new_context_and_call_resend( xi_mqtt_logic_task_t* task, void* context );

xi_state_t run_task( xi_layer_connectivity_t* context, xi_mqtt_logic_task_t* task );

#ifdef __cplusplus
}
#endif

#endif /* __XI_MQTT_LOGIC_LAYER_TASK_HELPERS_H__ */
