/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client codebase,
 * it is licensed under the BSD 3-Clause license.
 */
#define XIF_MQTT_TOPIC_MAX_LEN 256

typedef struct
{
    char button_topic[XIF_MQTT_TOPIC_MAX_LEN];
    char led_topic[XIF_MQTT_TOPIC_MAX_LEN];
} xif_mqtt_topics_t;
extern xif_mqtt_topics_t xif_mqtt_topics; /* Filled by xif_set_device_info */

typedef enum /* Flags to request changes state machine actions - Ordered by priority */
{
    XIF_REQUEST_CONTINUE   = ( 1 << 0 ),
    XIF_REQUEST_PAUSE      = ( 1 << 1 ),
    XIF_REQUEST_SHUTDOWN   = ( 1 << 2 ),
    XIF_REQUEST_ALL        = 0xff
} xif_action_requests_t;

/* Configure Xively credentials and topics
 */
int xif_set_device_info( char* xi_acc_id, char* xi_dev_id, char* xi_dev_pwd );

/* Xively Interface event loop - It handles the MQTT library's events loop and
 * coordinates the actions requested/kickstarted from other RTOS tasks. Loops
 * forever until it's paused with xif_disconnect or aborted with xif_shutdown
 */
void xif_rtos_task( void* param );

/* Request a graceful MQTT disconnection
 * XIF will be paused after the disconnection - Call xif_events_continue to
 * unpause the events loop and re-connect to the broker
 */
int xif_disconnect( void );

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
 * @retval: -1: xi_connect failed
 *           0: 0 xi_connect OK
 *           1: xi_connect failed because the connection was already initialized
 */
int xif_connect( void );

void xif_publish_button_pressed( void );

int xif_request_action( xif_action_requests_t requested_action );

/* Query the MQTT connection status (as far as the TCP/MQTT layers are aware)
 */
int xif_is_connected( void );

/* This callback is __weak__ in xively_if.c so you can overwrite it with your own.
 * For this demo, we permanently shut down the Xively Interface when we get
 * unrecoverable errors, but you may want to handle that differently
 */
extern void xif_state_machine_aborted_callback( void );
