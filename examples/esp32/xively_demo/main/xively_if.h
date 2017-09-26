/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client codebase,
 * it is licensed under the BSD 3-Clause license.
 */
#ifndef __XIVELY_IF_H__
#define __XIVELY_IF_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "xively.h"

#define XIF_MQTT_TOPIC_MAX_LEN 256

typedef struct
{
    char button_topic[XIF_MQTT_TOPIC_MAX_LEN];
    char led_topic[XIF_MQTT_TOPIC_MAX_LEN];
} xif_mqtt_topics_t;

extern xif_mqtt_topics_t xif_mqtt_topics; /* Filled by xif_set_device_info */

/* Actions that can be requested of the XIF state machine - Ordered by priority */
typedef enum
{
    XIF_REQUEST_CONTINUE = ( 1 << 0 ), /* Keep ticking libxively */
    XIF_REQUEST_PAUSE    = ( 1 << 1 ), /* Stop ticking, wait for _CONTINUE */
    XIF_REQUEST_SHUTDOWN = ( 1 << 2 ), /* Stop ticking, shutdown libxi and RTOS task */
    XIF_REQUEST_ALL      = 0xff
} xif_action_requests_t;

/* Configure Xively credentials and topics
 */
int8_t xif_set_device_info( char* xi_acc_id, char* xi_dev_id, char* xi_dev_pwd );

/* Xively Interface event loop - It handles the MQTT library's events loop and
 * coordinates the actions requested/kickstarted from other RTOS tasks. Loops
 * forever until it's paused with xif_disconnect or aborted with xif_shutdown
 */
void xif_rtos_task( void* param );

/* Request a graceful MQTT disconnection
 * XIF will be paused after the disconnection - Call xif_events_continue to
 * unpause the events loop and re-connect to the broker
 */
int8_t xif_disconnect( void );

/*
 * xif_connect() is called internally from xif_rtos_task before entering the
 * events loop, so this function doesn't need to be called when starting the
 * Xively Interface
 *
 * It will also be called internally if the library detects a disconnection.
 *
 * The application only needs this function to reconnect after calling
 * xif_disconnect()
 *
 * @retval -1: xi_connect failed
 * @retval  0: 0 xi_connect OK
 * @retval  1: xi_connect failed because the connection was already initialized
 */
int8_t xif_connect( void );

void xif_publish_button_state( int input_level );

/* Request an action from the Xively Interface's state machine.
 * xif_action_requests_t is ordered by priority (highest value, highes priority)
 *
 * @retval -1: Error
 * @retval  0: OK
 */
int8_t xif_request_action( xif_action_requests_t requested_action );

/* Sample implementation declared WEAK in xively_if.c so you can overwrite it */
extern void xif_recv_mqtt_msg_callback( const xi_sub_call_params_t* const params );

/* Query the MQTT connection status (as far as the TCP/MQTT layers are aware)
 */
int8_t xif_is_connected( void );

/* This callback is __weak__ in xively_if.c so you can overwrite it with your own.
 * For this demo, we permanently shut down the Xively Interface when we get
 * unrecoverable errors, but you may want to handle that differently
 */
extern void xif_state_machine_aborted_callback( void );

#ifdef __cplusplus
}
#endif
#endif /* __XIVELY_IF_H__ */
