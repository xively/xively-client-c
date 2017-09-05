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

/* Abruptly shut down the MQTT and TLS libraries.
 * Triggers the events loop to shut down and the RTOS task to delete itself.
 * If you'd like to gracefully disconnect from the server, call xif_disconnect()
 * before calling xif_shutdown
 */
int xif_shutdown( void );

/* Abruptly pause the XIF events loop - If there's an ongoing connection, don't
 * pause the events loop unless you don't mind it timing out.
 * This function will cause the XIF events loop to block until it is unpaused
 * via xif_events_continue(), xif_disconnect() or xif_events_shutdown()
 */
int xif_events_pause( void );

/* Unpause the XIF events loop
 * After signaling XIF to continue, if the program was paused with an ongoing
 * connection, XIF will try to pick up where it left off. If the connection
 * timed out, or if XIF was disconnected to begin with, this will cause the
 * loop to re-connect
 * TODO: If the connection was shut down via xif_disconnect, this won't automatically
 *       re-connect. Should I xif_connect() if !xif_is_connected() ? Will that cover all cases?
 */
int xif_events_continue( void );

/* Query the MQTT connection status (as far as the TCP/MQTT layers are aware)
 */
int xif_is_connected( void );

/* This callback is __weak__ in xively_if.c so you can overwrite it with your own.
 * For this demo, we permanently shut down the Xively Interface when we get
 * unrecoverable errors, but you may want to handle that differently
 */
extern void xif_state_machine_aborted_callback( void );
