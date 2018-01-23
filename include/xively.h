/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XIVELY_H__
#define __XIVELY_H__

#include <stdlib.h>
#include <stdint.h>

#include <xively_types.h>
#include <xively_connection_data.h>
#include <xively_mqtt.h>
#include <xively_time.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \file
 * @brief The main API for compiling your Application against the
 * the xively library.
 *
 * \mainpage Xively Client for C
 *
 * # Welcome
 * The functions of the main Xively API are used to build a client
 * connection to the Xively Service over a secure socket, and
 * publish / subscribe to messages over MQTT.
 * This documention, in conjuction with the Xively C Client User Guide
 * (<code>/doc/user_guide.md</code>) and source examples in
 * <code>/examples/</code>, should provide
 * you with enough information to create your custom
 * Xively-enabled applications.
 *
 * If you're looking for information on how to port the Xively C
 * Client to your custom embedded device, then please instead direct
 * your attention to the Board Support Package documentation.  This
 * exists in two parts, the Xively C Client Porting Guide
 * (<code>/doc/porting_guide.md</code>) and the
 * <a href="../../bsp/html/index.html">BSP doxygen</a>.
 *
 * # Where to Start
 * All of the standard Xively Client API functions are contained within
 * the <code>/include</code> directory of the project's base path, and have the
 * 'xively' prefix. We suggest clicking on xively.h in the doxygen
 * File tab to get started.
 *
 * For most POSIX systems this might be all of the documentation that
 * you need. However, if you're building a library to be exeucted on your
 * own custom platform with non POSIX library support, then you may also
 * need to browse the Board Support Package (BSP) functions as well.
 * These functions are documented in the BSP doxygen documentation that's
 * contained in a sibling directory as this doxygen.
 *
 * # Further Reading
 * <ul><li><a href="../../bsp/html/index.html">
 * Xively Board Support Package doxygen</a></li>
 * <li>
 * <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html">
 * MQTT v3.1.1 Specification</a></li>
 * <li> Xively Client Users Guide (<code>/doc/user_guide.md</code>)</li>
 * <li> Xively Client Porting Guide (<code>/doc/porting_guide.md</code>)</il>

 * </ul>
 * \copyright 2003-2018, LogMeIn, Inc.  All rights reserved.
 *
 */

/* -----------------------------------------------------------------------
 * MAIN LIBRARY FUNCTIONS
 * ----------------------------------------------------------------------- */

/**
 * @brief     Required before use of the xively library.
 * @detailed  This should be the first function that you call on a new runtime.  It
 * requires a string parameter that should be unique for the device the library is
 * running on ( serial number, etc. )  This is very important as otherwise messages
 * delivered by this device could be confused with other devices of the same unique
 * identifier.
 *
 * Additionaly messages meant for other devices might be delivered to this
 * device if the device_unique_id is used across mulitple devices.
 *
 * @param [in] account_id A string that identifies the user's account in BluePrint.
 * @param [in] device_unique_id A string that represents this specific device.
 *
 * @retval XI_STATE_OK              Status OK
 * @retval XI_FAILED_INITIALIZATION An urecoverable error occured
 * @retval XI_ALREADY_INITIALIZED   The runtime has previously invoked this
 * function succesfully.
 * @retval XI_INVALID_PARAMETER     If device_unique_id is null.
 */
extern xi_state_t xi_initialize( const char* account_id, const char* device_unique_id );

/**
 * @brief     Signals the xively library to cleanup any internal memory
 * @detailed  This should by the last function called while shutting down your
 * application.
 * Any resources that were created during initialization will be cleaned up and
 * freed.
 *
 * Note that you should always clean up xively contexts individually with
 * xi_delete_context
 *
 * @see xi_initialize
 * @see xi_create_context
 * @see xi_delete_context
 * @see xi_shutdown_connection
 *
 * @retval XI_STATE_OK              Status OK
 * @retval XI_FAILED_INITIALIZATION An urecoverable error occured
 */
extern xi_state_t xi_shutdown();

/**
 * @brief     Returns the device unique string that was passed to the xively
 * library during initialization.
 *
 * @retval NULL  If the system has not been succesfully initialized. Otherwise a
 * pointer to the device_unique_id.
 */
extern const char* xi_get_device_unique_id();

/**
 * @brief     Creates a connection context for subscriptions and publications
 * @detailed  This should by invoked following a successful libxively
 * initialization.  This  creates a specific context that can be passed to
 * connection, subscription, and publish functions.
 *
 * @see xi_initialize
 * @see xi_delete_context
 *
 * @retval nonnegative number         A valid context handle
 * @retval negative number            Negated error code
 */
extern xi_context_handle_t xi_create_context();

/**
 * @brief     Frees the provided connection context for subscriptions and
 * publications
 * @detailed  This should by invoked to free up memory when your applicaiton is done
 * connecting to the xively service throught the given context.
 *
 * @param [in] context handle that should be freed.
 *
 * @see xi_create_context
 *
 * @retval XI_STATE_OK           Status OK
 * @retval XI_INVALID_PARAMETER  If the provided context handle is invalid.
 */
extern xi_state_t xi_delete_context( xi_context_handle_t context_handle );


/**
 * @brief     Used to determine the state of a xively context's connection to
 * the remote service.
 *
 * @param [in] context handle to determine connection status of.
 *
 * @see xi_create_context
 * @see xi_connect
 * @see xi_connect_to
 *
 * @retval 1 if the context is currently connected
 * @retval 0 if the context is invalid, or the connection is currently any
 * of the other following states: Unitialized, connecting, closing or closed.
 */
extern uint8_t xi_is_context_connected( xi_context_handle_t context_handle );


/**
 * @brief     Invokes the Xively Event Processing loop.
 * @detailed  This function is meant when exectuing the Xively Client
 * as the main process and therefore it does not return until xi_events_stop
 * is invoked.
 *
 * If you're executing on a platform that cannot block indefinitely, then
 * please use xi_events_process_tick instead.
 *
 * @see xi_events_process_tick
 * @see xi_events_stop
 */
extern void xi_events_process_blocking();


/**
 * @brief     Invokes the Xively Event Processing loop.
 * @detailed This function will begin to process any pending tasks but then
 * return control to the client application on Embedded devices only.
 *
 * This function is meant for RTOS or No OS devices that require
 * that control be yielded to the OS for standard tick operations.
 *
 * NOTE: This function WILL BLOCK on standard UNIX devices like
 * LINUX or MAC OSX.
 *
 * XI_STATE_OK will be returned if the event system is ongoing and
 * can continue to operate
 *
 * XI_EVENT_PROCESS_STOPPED will be returned if xi_events_stop
 * has been invoked by the client application or if the event
 * processor has been stopped due to an unrecoverable error.
 *
 * @see xi_events_process_blocking
 * @see xi_events_stop
 *
 * @retval XI_STATE_OK           Status OK
 * @retval XI_EVENT_PROCESS_STOPPED  If the event processor has been stopped
 */
extern xi_state_t xi_events_process_tick();

/**
 * @brief     Causes the Xively Client event loop to exit.
 * @detailed  Pending scehduled events will not be invoked again until the
 * event loop is invoked again via xi_events_process.
 *
 * Stop is often called by the client application when shuttdown
 * its connectivity to the Xively Service.  The Xively Client
 * might also call stop if there's an unrecoverable error.
 *
 * @see xi_events_process_blocking
 * @see xi_events_process_tick
 */
extern void xi_events_stop();

/**
 * @brief Files set by this function will be kept updated by the Xively C Client.
 *
 * This API function is used to tell Xively C Client the update file manifest.
 * The Xively file IO BSP will collect revision for these files and send them
 * to the Xively Secure File Transfer (SFT) service immediately after a successful
 * MQTT connection. File revision is stored as a "twin" file of the original, with
 * `.xirev` extension. This "twin" file is maintained by the internal SFT logic. This
 * function must be called prior to `xi_connect`. This move initializes the update
 * process.
 *
 * The file list may or may not contain firmware binary. If it does then the firmware
 * update process will be triggered after whole package got downloaded.
 * A firmware differs in name from other update files. Plese visit BSP FWU's
 * `xi_bsp_fwu_is_this_firmware` function to check the exact firmware file
 * name since this function is responsible for differentiation.
 *
 * Large files (*8MB+ on 13.11.2017*) are not supported through the Xively Secure
 * File Transfer MQTT service. To download these files pass a function pointer
 * as function parameter: `url_handler`. This function gets called for each
 * individual file in the SFT update package. This function should start the
 * download on a separate thread and return immediately without blocking. The
 * `url_handler` function may reject file download causing a fallback to MQTT file
 * download if available. Read more details in `xively_types.h`.
 *
 * *example call:*
 *
 * ```
 * xi_set_updateable_files( xih,
 *                          ( const char* [] ){"file.cfg", "firmware.bin"}, 2,
 *                          url_handler_callback );
 * ```
 *
 * @param [in] xih a context handle created by invoking xi_create_context
 * @param [in] filenames a list of file names representing the files which have to
 *                       be kept updated
 * @param [in] count number of file names contained in the file names list
 * @param [in] url_handler function pointer, this is the application entry point for
 *             HTTP file download. This function is called with each filename and URL
 *             within an update package. Leave it NULL if HTTP download is not necessary
 *             for your update package, if Xively SFT service provides all the update
 *             files through MQTT.
 *
 * @retval XI_STATE_OK if file list set properly, an error value otherwise.
 */
extern xi_state_t xi_set_updateable_files( xi_context_handle_t xih,
                                           const char** filenames,
                                           uint16_t count,
                                           xi_sft_url_handler_callback_t* url_handler );

/**
 * @brief     Opens a connection to the xively service using the provided context,
 * includes a callback.
 * @detailed  Using the provided context, this function requests that a connection
 * be made to the xively service given the provide username and password.
 * The callback method will be invoked when this connection is complete.
 *
 * The callback function is type defined by xi_user_callback_t, which has the
 * following siguature:
 *    void foo( xi_context_handle_t in_context_handle
 *                  , void* data
 *                  , xi_state_t state )
 *
 * where the callback parameters are:
 *   - in_context_handle is context handle that you provided to xi_connect
 *   - data is a multifunctional structure. Please see the Xively User Guide or
 * examples for More Information
 *   - state should be XI_STATE_OK if the connection succeded. For other error
 * codes please see the Xively User Guide or Examples.
 *
 * @param [in] xih a context handle created by invoking xi_create_context
 * @param [in] username The username that was provided to you by the xively
 * service website for your device.
 * @param [in] password The password that was provided to you by the xively
 * service website for your device.
 * @param [in] connection_timeout Number of seconds that the socket will be kept
 * before CONNACK without data coming from the server. In case of 0 TCP timeout
 * will be used.
 * @param [in] keepalive_timeout Number of seconds that the socket will be kept
 * connect while being unused.
 *
 * @see xi_create_context
 * @see xi_connect_with_lastwill
 *
 * @retval XI_STATE_OK  If the connection request was formatted correctly.
 */
extern xi_state_t xi_connect( xi_context_handle_t xih,
                              const char* username,
                              const char* password,
                              uint16_t connection_timeout,
                              uint16_t keepalive_timeout,
                              xi_session_type_t session_type,
                              xi_user_callback_t* client_callback );

/**
 * @brief     Opens a connection to a custom service using the provided context,
 * host and port.
 *
 * @param [in] host client will connect to this address
 * @param [in] port client will connect to the host on this port
 *
 * Other than this further parameters and behaviour is identical to function xi_connect.
 *
 * @see xi_connect
 */
extern xi_state_t xi_connect_to( xi_context_handle_t xih,
                                 const char* host,
                                 uint16_t port,
                                 const char* username,
                                 const char* password,
                                 uint16_t connection_timeout,
                                 uint16_t keepalive_timeout,
                                 xi_session_type_t session_type,
                                 xi_user_callback_t* client_callback );

/**
 * @brief     Opens a connection to the xively service using the provided context,
 * includes a callback and a last will.
 * @detailed Using the provided context, this function requests that a connection be
 * made to the xively service given the provide username and password.  The callback
 * method will be invoked when this connection is complete.
 *
 * The callback function is typdefed as xi_user_callback_t, and should have the
 * following siguature:
 *    void foo( xi_context_handle_t in_context_handle
 *                  , void* data
 *                  , xi_state_t state )
 *
 * where the callback parameters are:
 *   - in_context_handle is context handle that you provided to
 * xi_connect_with_last_will
 *   - data is a multifunctional structure. Please see the Xively User Guide or
 * examples for More Information
 *   - state should be XI_STATE_OK if the connection succeded. For other error
 * codes please see the Xively User Guide or Examples.
 *
 *
 * @param [in] xih a context handle created by invoking xi_create_context
 * @param [in] username The username that was provided to you by the xively
 * service website for your device.
 * @param [in] password The password that was provided to you by the xively
 * service website for your device.
 * @param [in] connection_timeout Number of seconds that the socket will be kept
 * before CONNACK without data coming from the server. In case of 0 TCP timeout
 * will be used.
 * @param [in] keepalive_timeout Number of seconds that the socket will be kept
 * connect while being unused.
 * @param [in] will_topic The topic that should be used for the last will
 * function
 * @param [in] will_message The message that should be used for the last will
 * function
 * @param [in] will_qos The quality of service that should be used for the last
 * will function
 * @param [in] will_retain The retain state that should be used for the last
 * will function
 * @param [in] on_connected A function pointer to a function with the signature:
 *   xi_state_t foo(void*,void*,xi_state_t)
 *
 * @see xi_create_context
 * @see xi_connect
 *
 * @retval XI_STATE_OK  If the connection request was formatted correctly.
 */
extern xi_state_t xi_connect_with_lastwill( xi_context_handle_t xih,
                                            const char* username,
                                            const char* password,
                                            uint16_t connection_timeout,
                                            uint16_t keepalive_timeout,
                                            xi_session_type_t session_type,
                                            const char* will_topic,
                                            const char* will_message,
                                            xi_mqtt_qos_t will_qos,
                                            xi_mqtt_retain_t will_retain,
                                            xi_user_callback_t* client_callback );

/**
 * @brief     Opens a connection to a custom service using the provided context,
 * host and port.
 *
 * @param [in] host client will connect to this address
 * @param [in] port client will connect to the host on this port
 *
 * Other than this further parameters and behaviour is identical to function
 * xi_connect_with_lastwill.
 *
 * @see xi_connect_with_lastwill
 */
extern xi_state_t xi_connect_with_lastwill_to( xi_context_handle_t xih,
                                               const char* host,
                                               uint16_t port,
                                               const char* username,
                                               const char* password,
                                               uint16_t connection_timeout,
                                               uint16_t keepalive_timeout,
                                               xi_session_type_t session_type,
                                               const char* will_topic,
                                               const char* will_message,
                                               xi_mqtt_qos_t will_qos,
                                               xi_mqtt_retain_t will_retain,
                                               xi_user_callback_t* client_callback );

/**
 * @brief     Publishes a message to the xively server on the given topic and
 * notifies about the result through callback.
 * @detailed  Using the provided context, this function requests that a message be
 * delivered to the xively service on the given topic. This requires that a connection
 * already be made to the xively service via a xi_connect call
 *
 * The callback function should have the following siguature:
 *  void foo( xi_context_handle_t in_context_handle
 *                , void* user_data
 *                , xi_state_t state )
 *
 * where the callback parameters are:
 *   - in_context_handle is context handle that you provided to xi_publish
 *   - user_data is the value that you passed in to the user_data parameter of the
 * publish call (below) this can be used for you attach any sort support data or
 * relevant data that would help identify the message.
 *   - state should be XI_STATE_OK if the publication succeded, or a non ok value
 * if the publish failed.
 *
 * please return XI_STATE_OK for these callbacks.
 *
 * @param [in] xih a context handle created by invoking xi_create_context
 * @param [in] topic a string based topic name that you have created for messaging via
 * the xively webservice.
 * @param [in] msg the payload to send to the xively service.
 * @param [in] qos Quality of Service MQTT level. 0, 1, or 2.  Please see MQTT
 * specification or xi_mqtt_qos_e in xi_mqtt_message.h for constant values.
 * @param [in] retain of the message, retain may be XI_MQTT_RETAIN_TRUE or
 * XI_MQTT_RETAIN_FALSE. Please see MQTT specification or xi_mqtt_retain_e in
 * xi_mqtt_message.h for constant values.
 * @param [in] callback Optional callback function that will be called upon
 * successful or unsuccessful msg delivery. This may be NULL.
 * @param [in] user_data Optional abstract data that will be passed to callback
 * when the publication completes. This may be NULL.
 *
 * @see xi_create_context
 * @see xi_publish_timeseries
 * @see xi_publish_formatted_timeseries
 * @see xi_publish_data
 *
 * @retval XI_STATE_OK         If the publication request was formatted correctly.
 * @retval XI_OUT_OF_MEMORY    If the platform did not have enough free memory
 * to fulfill the request
 * @retval XI_INTERNAL_ERROR   If an unforseen and unrecoverable error has occured.
 * @retval XI_BACKOFF_TERMINAL If backoff has been applied
 */
extern xi_state_t xi_publish( xi_context_handle_t xih,
                              const char* topic,
                              const char* msg,
                              const xi_mqtt_qos_t qos,
                              const xi_mqtt_retain_t retain,
                              xi_user_callback_t* callback,
                              void* user_data );

/**
 * @brief     Publishes a float value to the xively server on the given topic.
 * @detailed  Using the provided context, this function requests that a value be
 * delivered to the xively service on the given topic. This requires that a
 * connection already be made to the xively service via a xi_connect call
 *
 * The callback function should have the following siguature:
 *  void foo( xi_context_handle_t in_context_handle
 *                , void* user_data
 *                , xi_state_t state )
 *
 * where the callback parameters are:
 *   - in_context_handle is the context_handle that you provided to xi_publish_timeseries
 *   - user_data parameter of the publish call (below) this can be used for you attach
 * any sort support data or relevant data that would help identify the message.
 *   - state should be XI_STATE_OK if the publication succeded, or a non ok value if the
 * publish failed.
 *
 * please return XI_STATE_OK for these callbacks.
 *
 * @param [in] xih a context handle created by invoking xi_create_context
 * @param [in] topic a string based topic name that you have created for messaging via
 * the xively webservice.
 * @param [in] float value the payload to send to the xively service.
 * @param [in] qos Quality of Service MQTT level. 0, 1, or 2.  Please see MQTT
 * specification or xi_mqtt_qos_e in xi_mqtt_message.h for constant values.
 * @param [in] callback Optional callback function that will be called upon
 * successful or unsuccessful msg delivery. This may be NULL.
 * @param [in] user_data Optional abstract data that will be passed to callback
 * when the publication completes. This may be NULL.
 *
 * @see xi_create_context
 * @see xi_publish
 * @see xi_publish_data
 * @see xi_publish_formatted_timeseries
 *
 * @retval XI_STATE_OK        If the publication request was formatted correctly.
 * @retval XI_OUT_OF_MEMORY   If the platform did not have enough free memory to
 * fulfill the request
 * @retval XI_INTERNAL_ERROR  If an unforseen and unrecoverable error has
 * occurred.
 */
extern xi_state_t xi_publish_timeseries( xi_context_handle_t xih,
                                         const char* topic,
                                         const float value,
                                         const xi_mqtt_qos_t qos,
                                         xi_user_callback_t* callback,
                                         void* user_data );

/**
 * @brief     Publishes a float value to the xively server on the given topic.
 * @detailed  Using the provided context, this function requests that a tagged timeseries
 * value be delivered to the xively service on the given topic. This requires that a
 * connection already be made to the xively service via a xi_connect call.
 *
 * Unlike the standard float timeseries publication, this timeseries can include a
 * category name, a string value and a numeric float value.
 *
 * At least one of these fields must be set.  If you wish to omit any one of these fields
 * the pass NULL as value to this function.
 *
 * The time value is also optional.  If provided the timeseires will be stored in the
 * Xively database with the provided milliseconds since the epoch.
 *
 * If not provide (if it's NULL) then the Xively service will tag the time series data
 * with the current server time.
 *
 * NOTE: this function uses CSV for a message format internally. Therefore the
 * following characters cannot be within a category or string_value:
 *  newline: \n
 *  carriage return: \r
 *  comma: ,
 *
 * If a new line, a carriage return, or a comma is detected, then this function
 * will be return XI_SERIALZIATION_ERROR as an error code.
 *
 * NOTE: category and string_value cannot exceed 1024 characters individually.
 *
 * Like the other publication functions, the callback function that you provide
 * is optional,and should have the following siguature:
 *  void foo( xi_context_handle_t in_context_handle
 *                , void* user_data
 *                , xi_state_t state )
 *
 * where the callback parameters are:
 *   - in_context_handle is context handle that you provided to
 * xi_publish_formatted_timeseries
 *   - user_data is the value that you passed in to the user_data parameter of the
 * publish call (below) this can be used for you attach any sort support data or
 * relevant data that would help identify the message.
 *   - state should be XI_STATE_OK if the publication succeded, or a non ok value
 * if the publish failed.
 *
 * please return XI_STATE_OK for these callbacks.
 *
 * @param [in] xih a context handle created by invoking xi_create_context
 * @param [in] topic Required. A string based topic name that you have created
 * for messaging via the xively webservice.
 * @param [in] time Optional. The number of milliseconds since the epoch. If
 * NULL, the Xively service will timestamp the timeseries with the server time.
 * @param [in] category Optional. The timeseries category as defined above.
 * This is a customer dependent value and has no impact on how Xively handles the
 * timeseries. May be used in conjuction with numeric_value and string_value.
 * @param [in] string_value Optional. A field that will log a string value for
 * the time series. May be used in conjuction with nuermic_value and category.
 * @param [in] numeric_value Optional. A field that will log a float value for
 * the time series. May be used in conjuction with string_Value and category.
 * @param [in] qos Quality of Service MQTT level. 0, 1, or 2.  Please see MQTT
 * specification or xi_mqtt_qos_e in xi_mqtt_message.h for constant values.
 * @param [in] callback Optional A callback function that will be called upon
 * successful or unsuccessful msg delivery. This may be NULL.
 * @param [in] user_data Optional An abstract data value that will be passed to
 * callback when the publication completes. This may be NULL.
 *
 * @see xi_create_context
 * @see xi_publish
 * @see xi_publish_data
 * @see xi_publish_timeseries
 *
 * @retval XI_STATE_OK If the publication request was formatted correctly.
 * @retval XI_INVALID_PARAMETER If one of category, string_value or
 * numeric_value are not defined.  You do not need all of these values, but
 * at least one.
 * @retval XI_SERIALZIATION_ERROR if an invalid character is found in a CSV
 * string, if the category or string_value string exceed 1024 characters,
 * or if numeric_value cannot be formatted as a float, or if time can not be
 * formmated as a unsigened integer.
 * @retval XI_OUT_OF_MEMORY     If the platform did not have enough free memory
 * to fulfill the request
 * @retval XI_INTERNAL_ERROR    If an unforseen and unrecoverable error has
 * occurred.
 */
extern xi_state_t xi_publish_formatted_timeseries( xi_context_handle_t xih,
                                                   const char* topic,
                                                   const uint32_t* time,
                                                   char* category,
                                                   char* string_value,
                                                   const float* numeric_value,
                                                   const xi_mqtt_qos_t qos,
                                                   xi_user_callback_t* callback,
                                                   void* user_data );

/**
 * @brief     Publishes binary data to the xively server on the given topic.
 * @detailed Using the provided context, this function requests that binary data be
 * delivered to the xively service on the given topic. This requires that a connection
 * already be made to the xively service via a xi_connect call.
 *
 * The callback function should have the following siguature:
 *  void foo( xi_context_handle_t in_context_handle
 *                , void* user_data
 *                , xi_state_t state )
 *
 * where the callback parameters are:
 *   - in_context_handle is context handle that you provided to xi_publish_data
 *   - user_data is the value that you passed in to the user_data parameter of the
 * publish call (below) this can be used for you attach any sort support data or relevant
 * data that would help identify the message.
 *   - state should be XI_STATE_OK if the publication succeded, or a non ok value
 * if the publish failed.
 *
 * please return XI_STATE_OK for these callbacks.
 *
 * @param [in] xih a context handle created by invoking xi_create_context
 * @param [in] topic a string based topic name that you have created for
 * messaging via the xively webservice.
 * @param [in] data the payload to send to the xively service.
 * @param [in] qos Quality of Service MQTT level. 0, 1, or 2.  Please see MQTT
 * specification or xi_mqtt_qos_e in xi_mqtt_message.h for constant values.
 * @param [in] retain of the message, retain may be XI_MQTT_RETAIN_TRUE or
 * XI_MQTT_RETAIN_FALSE. Please see MQTT specification or xi_mqtt_retain_e in
 * xi_mqtt_message.h for constant values.
 * @param [in] callback Optional callback function that will be called upon
 * successful or unsuccessful msg delivery. This may be NULL.
 * @param [in] user_data Optional abstract data that will be passed to callback
 * when the publication completes. This may be NULL.
 *
 * @see xi_create_context
 * @see xi_publish
 * @see xi_publish_timeseries
 * @see xi_publish_formatted_timeseries
 *
 * @retval XI_STATE_OK If the publication request was formatted correctly.
 * @retval XI_OUT_OF_MEMORY   If the platform did not have enough free memory to
 * fulfill the request
 * @retval XI_INTERNAL_ERROR  If an unforseen and unrecoverable error has
 * occurred.
 */
extern xi_state_t xi_publish_data( xi_context_handle_t xih,
                                   const char* topic,
                                   const uint8_t* data,
                                   size_t data_len,
                                   const xi_mqtt_qos_t qos,
                                   const xi_mqtt_retain_t retain,
                                   xi_user_callback_t* callback,
                                   void* user_data );

/**
 * @brief     Subscribes to request notifications if a message from the xively
 * service is posted to the given topic.
 * @detailed  Using the provided context, this function requests that the xively library
 * listen to a topic that the xively service my post messages to.  The provided callback
 * function will be invoked when a message arrives.
 *
 * The callback function is type defined by xi_user_callback_t, which has the
 * following siguature:
 *    void foo( xi_context_handle_t in_context_handle,
 *          xi_sub_call_type_t call_type,
 *          const xi_sub_call_params_t* const params,
 *          xi_state_t state,
 *          void* user_data )
 *
 * where:
 *   - context_handle is the context handle you provided to xi_subscribe
 *   - call type each callback invocation may be related to subscription confirmation or
 * new message, this enum points to the type of the invocation
 *   - params is a structure that holds the details about the subscription confirmation or
 * new message depends on the call_type value
 *   - state should be XI_STATE_OK if the message reception succeded or
 * XI_MQTT_SUBSCRIPTION_SUCCESSFULL/XI_MQTT_SUBSCRIPTION_FAILED in case of
 * successfull / failed subscription, values different than described should be treated
 * as errors
 *   - user is the pointer you provided to user parameter in xi_subscribe
 *
 * @param [in] xih a context handle created by invoking xi_create_context
 * @param [in] topic a string based topic name that you have created for
 * messaging via the xively webservice.
 * @param [in] qos Quality of Service MQTT level. 0, 1, or 2.  Please see MQTT
 * specification or xi_mqtt_qos_e in xi_mqtt_message.h for constant values.
 * @param [in] callback a function pointer to be invoked when a
 * message arrives, as described above
 * @param [in] user a pointer that will be returned back during the callback invocation
 *
 * @see xi_create_context
 * @see xi_connect
 * @see xi_publish
 * @see xi_publish_data
 *
 * @retval XI_STATE_OK If the publication request was formatted correctly.
 * @retval XI_OUT_OF_MEMORY   If the platform did not have enough free memory to
 * fulfill the request
 * @retval XI_INTERNAL_ERROR  If an unforseen and unrecoverable error has occurred.
 */
extern xi_state_t xi_subscribe( xi_context_handle_t xih,
                                const char* topic,
                                const xi_mqtt_qos_t qos,
                                xi_user_subscription_callback_t* callback,
                                void* user_data );

/**
 * @brief     Closes the connection associated with the provide context.
 * @detailed  Closes connection to the Xively Service.  This will happen asynchronously.
 * The callback defined by xi_connect.
 *
 * @param [in] xih a context handle created by invoking xi_create_context
 *
 * @retval XI_SOCKET_NO_ACTIVE_CONNECTION_ERROR If there is no connection for
 * this context.
 * @retval XI_STATE_OK If the disconnection request has been queued for process.
 *
 * @see xi_create_context
 * @see xi_connect
 */
extern xi_state_t xi_shutdown_connection( xi_context_handle_t xih );

/**
 * @brief     Schedule a task for timed execution
 * @detailed Using the provided context, this function adds a task to the internal
 * container. The callback method will be invoked when the given time has passed.
 *
 * the callback function should have the following siguature:
 *  typedef void ( xi_user_task_callback_t ) ( const xi_context_handle_t context_handle
 *                                           , const xi_timed_task_handle_t
 *                                           , timed_task_handle
 *                                           , void* user_data );
 *
 * where:
 *   - in_context_handle is context handle you provided to xi_schedule_timed_task
 *   - timed_task_handle is the handle that xi_schedule_timed_task call returned, this
 *                       can be used to cancel the task if the task is no longer needed
 *                       from within the callback
 *   - user_data is the data you provided to xi_schedule_timed_task
 *
 * @param [in] xih a context handle created by invoking xi_create_context
 * @param [in] xi_user_task_callback_t* a function pointer to be invoked when
 * given time has passed
 * @param [in] seconds_from_now number of seconds to wait before task invocation
 * @param [in] repeats_forever if zero is passed the callback will be called only once,
 * otherwise the callback will be called continuously with the seconds_from_now delay
 *  until xi_cancel_timed_task() called.
 *
 * @see xi_create_context
 * @see xi_cancel_timed_task
 *
 * @retval xi_time_task_handle_t if bigger than 0 it's the unique identifier of
 * the task, if smaller, it's an error code multiplied by -1, the possible error
 * codes are :
 *      XI_STATE_OK        If the request was formatted correctly.
 *      XI_OUT_OF_MEMORY   If the platform did not have enough free memory to
 * fulfill the request
 *      XI_INTERNAL_ERROR  If an unforseen and unrecoverable error has occurred.
 */
xi_timed_task_handle_t xi_schedule_timed_task( xi_context_handle_t xih,
                                               xi_user_task_callback_t* callback,
                                               const xi_time_t seconds_from_now,
                                               const uint8_t repeats_forever,
                                               void* data );

/**
 * @brief     Cancel a timed task
 * @detailed  This function cancels the timed execution of the task defined by the given
 * handle.
 *
 * @param [in] timed_task_handle a handle returned by xi_schedule_timed_task
 *
 * @see xi_create_context
 * @see xi_schedule_timed_task
 */

void xi_cancel_timed_task( xi_timed_task_handle_t timed_task_handle );

/*-----------------------------------------------------------------------
 * HELPER FUNCTIONS
 * ---------------------------------------------------------------------- */

/**
 * @brief     Timeout value that's passed to the networking implementation
 * @detailed  This function configures how long a socket will live while there
 * is no data sent on the socket.  Note that the MQTT Keep Alive
 * value configured in the xi_connect will cause the Xively Client to
 * periodically create network traffic, as part of the MQTT specification.
 *
 * This value is only observed when constructing new connections, so invoking
 * this will not have any affect on on-going connections.
 *
 * @param [in] timeout a timeout value that will be passed to the implementing
 * networking layer during socket initialization.
 *
 * @see xi_connect
 * @see xi_connect_to
 * @see xi_get_network_timeout
 **/
extern void xi_set_network_timeout( uint32_t timeout );


/**
 * @brief     Returns the timeout value for socket connections.
 * @detailed  Queries the current configuration of network timeout values that will
 * be fed to that native network implementation when sockets are initialized.
 *
 * @see xi_set_network_timeout
 **/
extern uint32_t xi_get_network_timeout( void );


/**
 * @brief     Sets Maximum Amount of Heap Allocated Memory the Xively Client May Use
 * @detailed  This function is part of an optional configuration of the Xively Client
 * called the Memory Limiter.
 *
 * This system can be used to guarantee that the Xively Client will use only a certain
 * amount of the heapspace during its standard execution.  On many platforms this can
 * also include the heap usage of the TLS implementation if the TLS implementation can be
 * written to use the Xively malloc and free functions.
 *
 * @param [in] max_bytes the upper bounds of memory that the Xively Client will use
 *  in its serialization, parsing and encryption of mqtt packets, and for the
 *  facilitation of Client Application callbacks and events.
 *
 * @retval XI_NOT_SUPPORTED If the Memory Limiter module has not been compiled
 *  into this version of the Xively Client Library
 *
 * @retval XI_OUT_OF_MEMORY If the new memory limit is too small to support
 *  the current Xively Client heapspace footprint.
 *
 * @retval XI_STATE_OK the new memory limit has been set to the value
 *  specified by max_bytes.
*/
xi_state_t xi_set_maximum_heap_usage( const size_t max_bytes );


/**
 * @brief     Fetches the Xively Client's current amount of heap usage
 * @detailed  This function is part of an optional configuration of the Xively Client
 * called the Memory Limiter.  If enabled, you can use this function to determine
 * the current heap usage of the Xively Client.  Depending on the TLS implementation,
 * this might also include the TLS buffers used for encoding / decoding and
 * certificate parsing.
 *
 * @retval XI_NOT_SUPPORTED If the Memory Limiter module has not been compiled
 *  into this version of the Xively Client Library
 *
 * @retval XI_INVALID_PARAMETER If the provided parameter is NULL
 *
 * @retval XI_STATE_OK if the parameter has been filled-in with the Xively Client's
 *  current heap usage.
 *
*/
xi_state_t xi_get_heap_usage( size_t* const heap_usage );

/**
 * @brief Contains the major version number of the Xively C Client
 * library.
 **/
extern const uint16_t xi_major;

/**
 * @brief Contains the minor version number of the Xively C Client
 * library.
 **/
extern const uint16_t xi_minor;

/**
 * @brief Contains the revsion number of the Xively C Client library.
 **/
extern const uint16_t xi_revision;

/**
 * @brief String representation of the Major.Minor.Revision version
 * of the Xively C Client library.
 * library.
 **/
extern const char xi_cilent_version_str[];


#ifdef XI_EXPOSE_FS
#include "xi_fs_api.h"

/**
 * @brief     Sets filesystem functions so that the libxively can use custom filesystem
 */
xi_state_t xi_set_fs_functions( const xi_fs_functions_t fs_functions );

#endif /* XI_EXPOSE_FS */

#ifdef __cplusplus
}
#endif

#endif /* __XIVELY_H__ */
