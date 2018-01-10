/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client codebase,
 * it is licensed under the BSD 3-Clause license.
 */
#ifndef __XIVELY_TASK_H__
#define __XIVELY_TASK_H__

#include <stdint.h>
#include "xively.h"

#ifdef __cplusplus
extern "C" {
#endif
/*! \file
 * @brief Interface between the top level application logic and the Xively Client
 * Library for an RTOS-based environment.
 *
 * \mainpage Sample Implementation of a Dedicated RTOS Task for the Xively Client
 * Library
 *
 * @detailed Xively Task is a state machine that will run on its own RTOS task,
 * and a task-safe API to communicate with it, request the task to pause/shutdown
 * when other parts of the application need extra resources, etc.
 *
 * Most, if not all of the Xively Client logic is run from within this task, so
 * this demo can be used to measure the stack requirements of the library.
 *
 * For some production systems, you may want/need to implement a limit on how many
 * subscribe/publish messages are in flight at any given point; not sending the
 * next publish/subscribe message until the callback for the previous one(s)
 * have acknowledged success.
 * This will help make sure the Xively Client doesn't internally allocate too
 * much memory (required for QoS>0)  and crash your device. This can be done by
 * adding a new _PUBSUB request and machine state that only publishes the next
 * item in a queue when ready, and rejects messages when the queue is full.

 * \copyright 2003-2018, LogMeIn, Inc.  All rights reserved.
 *
 */

#define XT_MQTT_TOPIC_MAX_LEN 256

/**
 * @see xt_mqtt_topics
 */
typedef struct {
    char button_topic[XT_MQTT_TOPIC_MAX_LEN];
    char led_topic[XT_MQTT_TOPIC_MAX_LEN];
} xt_mqtt_topics_t;

/**
 * @brief This struct contains the full topic paths, including Account ID and Device ID.
 * It is filled by xt_init
 */
extern xt_mqtt_topics_t xt_mqtt_topics;

/**
 * @brief Actions that can be requested of the XT state machine - Ordered by priority
 * Priorities are enforced from xt_pop_highest_priority_request()
 */
typedef enum {
    XT_REQUEST_CONTINUE   = ( 1 << 0 ), /* Keep ticking the Xively Client Library */
    XT_REQUEST_DISCONNECT = ( 1 << 1 ), /* Send MQTT disconnect and close socket */
    XT_REQUEST_CONNECT    = ( 1 << 2 ), /* Stablish MQTT connection */
    XT_REQUEST_PAUSE      = ( 1 << 3 ), /* Stop ticking, wait for _CONTINUE */
    XT_REQUEST_SHUTDOWN   = ( 1 << 4 ), /* Stop ticking, shutdown libxi and RTOS task */
    XT_REQUEST_ALL        = 0xff
} xt_action_requests_t;

/**
 * @brief Find out whether xt_init() has been called already, so you know whether
 * your request will be denied/fail
 *
 * @retval 0 Not ready for requests
 * @retval 1 Ready for requests
 */
int8_t xt_ready_for_requests( void );

/**
 * @brief Initialize variables required for the Xively Task
 * @detailed Configure Xively credentials and topics, initialize XT-related RTOS
 * event groups, initialize the Xively Client and create a context handle xt_init
 * must be called EVERY TIME you start a xively task
 *
 * None of the MQTT credential strings will be copied in this interface to save
 * space, but the Xively Client does copy some strings to make sure they're always
 * accessible when required.
 * The argument pointers will be stored, and the referenced strings cannot be
 * released
 *
 * @param [in] *xi_acc_id Pointer to the Xively account ID string
 * @param [in] *xi_acc_id Pointer to the Xively device ID string
 * @param [in] *xi_acc_id Pointer to the Xively device password string
 *
 * @see xt_rtos_task
 *
 * @retval -1 Error
 * @retval  0 OK
 */
int8_t xt_init( char* xi_acc_id, char* xi_dev_id, char* xi_dev_pwd );

/**
 * @brief A Finite State Machine that coordinates when to connect, disconnect,
 * tick the Xively Client, pause the task or shut it down.
 * Loops forever until it's paused with a _PAUSE request or aborted with _SHUTDOWN
 *
 * @param [None - This is a standard declaration of a FreeRTOS task function]
 *
 * @see xt_action_requests_t
 * @see xt_request_machine_state
 */
void xt_rtos_task( void* param );

/**
 * @brief Request an action from the Xively Task's state machine.
 * xt_action_requests_t is ordered by priority (highest value, highes priority)
 *
 * @retval -1 Error
 * @retval  0 OK
 */
int8_t xt_request_machine_state( xt_action_requests_t requested_action );

/**
 * @brief Publish a "1" or "0" string to the Button topic
 *
 * @param [in] Value to publish, as an int
 */
void xt_publish_button_state( int input_level );

/**
 * @brief This function will be called from the Xively task when a new MQTT
 * message is received
 * @detailed Sample implementation declared WEAK in xively_task.c so you can overwrite it
 *
 * @param [in] Received message datastructure. See xively_types.h and the __weak__
 * implementation in xively_task.c to understand how to access the useful data
 */
extern void xt_recv_mqtt_msg_callback( const xi_sub_call_params_t* const params );

/**
 * @brief Get the status of the MQTT connection, as perceived from the MQTT lib
 *
 * @retval 1: MQTT Connection established
 * @retval 0: Not connected
 */
int8_t xt_is_connected( void );

/**
 * @brief This function gracefully shuts down the xt_rtos_task when an unrecoverable
 * MQTT/Xively Client issue is detected. e.g. Invalid credentials, critical errors, etc.
 * @detailed This callback is __weak__ in xively_task.c so you can overwrite it with your
 * own. For this demo, we permanently shut down the Xively Task when we get unrecoverable
 * errors, but you may want to handle that differently
 */
void xt_handle_unrecoverable_error( void );

#ifdef __cplusplus
}
#endif
#endif /* __XIVELY_TASK_H__ */
