/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

/*
 * This example application connects to the Xively Service with
 * a username and password that you must specify.  It then publishes
 * test messages to a topic that you also must specify
 *
 * This example application is meant as a companion example to
 * mqtt_logic_consumer example.
 */

#include <xively.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define XI_UNUSED( x ) ( void )( x )

#include "../../common/src/commandline.h"

xi_context_handle_t xi_context = XI_INVALID_CONTEXT_HANDLE;

static xi_timed_task_handle_t delayed_publish_task_handle = XI_INVALID_TIMED_TASK_HANDLE;


void on_connection_state_changed( xi_context_handle_t in_context_handle,
                                  void* data,
                                  xi_state_t state );
void delayed_publish( const xi_context_handle_t context_handle,
                      const xi_timed_task_handle_t timed_task_handle,
                      void* user_data );

unsigned int times_to_publish;

/*
 * This function is for a feature that's still in development slated for Q2 2015.
 * Please do not edit this portion of the example, otherwise your topic names will
 * be incorrectly mangled.
 */
const char* get_unique_device_id()
{
    /*
     * this needs to return an identifier unique to this particular device. For instance,
     * the device's serial number in string form.
     */
    return "";
}

/*
 * -main-
 * The main entry point for this example binary.
 *
 * There are two different ways of starting this application:
 * 1: If you already have a username and password stored in libxively_last_will.creds
 *  (see below) then you need only specify on the topic name that you want the
 *  mqtt_last_will to publish to.
 *  For instance:
 *      ./mqtt_last_will -p test/topic -T lastwill/topic -M "Last Will Message"
 *
 * 2: If the file libxively_last_will.creds does not exist or is empty then you must
 *  specify the username and password on the command line, followed by the
 *  topic that you want the mqtt_last_will to publish to.
 *  For instance:
 *      ./mqtt_last_will -u your_username -P your_password -p test/topic \
 *                       -T lastwill/topic -M "Last Will Message"
 *
 * This example will publish to the test/topic periodically.
 *
 * If the application is killed, the broker must publish the last_will message to the
 * last_will topic.
 *
 * There are optional parameters that can be used to control this application.
 *  -R --will-retain
 *      If the client disconnects unexpectedly the last will message sent out will be
 *      treated as a retained message. This must be used in conjunction with
 *      -T --will-topic and -M -- will-message
 *
 *  -E --exitwithoutclosing
 *      Exit the program abnormally. This will cause the broker to publish the last will
 *      message.
 *
 * For information on creating username and password credentials for your devices
 * to access to Xively Service during development then please contact Xively Client
 * Services.
 */

#ifndef XI_CROSS_TARGET
int main( int argc, char* argv[] )
#else
int xi_mqtt_last_will_main( xi_embedded_args_t* xi_embedded_args )
#endif /* XI_CROSS_TARGET */

{
    char options[]       = "ha:u:P:p:T:M:R:En:";
    int missingparameter = 0;
    int retval           = 0;

#ifndef XI_CROSS_TARGET
    printf( "%s\n%s\n", argv[0], xi_cilent_version_str );

    /* Parse the argv array for ONLY the options specified in the options string */
    retval = xi_parse( argc, argv, options, sizeof( options ) );
#else
    printf( "mqtt_last_will\n%s\n", xi_cilent_version_str );
    retval = xi_embedded_parse( xi_embedded_args, options, sizeof( options ) );
#endif /* XI_CROSS_TARGET */


    if ( -1 == retval )
    {
        exit( -1 );
    }

    /* Check to see that the required parameters were all present on the command line */
    if ( NULL == xi_account_id )
    {
        missingparameter = 1;
        printf( "-a --account-id is required\n" );
    }
    if ( NULL == xi_username )
    {
        missingparameter = 1;
        printf( "-u --username is required\n" );
    }
    if ( NULL == xi_password )
    {
        missingparameter = 1;
        printf( "-P --password is required\n" );
    }
    if ( NULL == xi_publishtopic )
    {
        missingparameter = 1;
        printf( "-p --publishtopic is required\n" );
    }
    if ( NULL == xi_will_topic )
    {
        missingparameter = 1;
        printf( "-T --will-topic is required\n" );
    }
    if ( NULL == xi_will_message )
    {
        missingparameter = 1;
        printf( "-M --will-payload is required\n" );
    }

    if ( 1 == missingparameter )
    {
        exit( -1 );
    }

    times_to_publish = xi_numberofpublishes;

    /* initialize xi library and create a context to use to connect to the Xively Service
     * Device-id must be the same as username
     */
    const xi_state_t error_init = xi_initialize( xi_account_id, xi_username );

    if ( XI_STATE_OK != error_init )
    {
        printf( " xi failed to initialize, error: %d\n", error_init );
        return -1;
    }

    /*
     * Create a connection context. A context represents a Connection on a single socket,
     * and can be used to publish and subscribe to numerous topics.
     * If you would like to use more than one connection then please use xi_create_context
     * to create more contexts.
     */
    xi_context = xi_create_context();
    if ( XI_INVALID_CONTEXT_HANDLE >= xi_context )
    {
        printf( " xi failed to create context, error: %d\n", -xi_context );
        return -1;
    }

#ifndef XI_CROSS_TARGET
    printf( "%s running with will_retain = %d, abnormalexit = %d\n", argv[0],
            xi_will_retain, xi_abnormalexit_flag );
#else
    printf( "mqtt_last_will running with will_retain = %d, abnormalexit = %d\n",
            xi_will_retain, xi_abnormalexit_flag );
#endif /* XI_CROSS_TARGET */
       /*
        * Create a connection request with the credentials.
        *
        * The topic name will be used for publication requests only after the connection has
        * been established.
        *
        * The 'on_connection_state_changed' parameter is the name of the function called
        * after the connection request completes, and its implementation should handle
        * both successful connections and unsuccessful connections as well as disconnections.
        */
    uint16_t connection_timeout = 10;
    uint16_t keepalive_timeout  = 20;

    const xi_state_t connect_state = xi_connect_with_lastwill(
        xi_context, xi_username, xi_password, connection_timeout, keepalive_timeout,
        XI_SESSION_CLEAN, xi_will_topic, xi_will_message, xi_example_qos, xi_will_retain,
        on_connection_state_changed );

    if ( XI_STATE_OK != connect_state )
    {
        printf( " xi failed to connect, error: %d\n", connect_state );
        return -1;
    }

    /*
     * The Xively Client was designed for single threaded devices.
     *
     * As such it does not have its own event loop thread. Instead you must call
     * the function xi_events_process_blocking() or xi_events_process_tick() to
     * process connection requests and to regularly check the sockets for incoming
     * data.
     *
     * This xi_events_process_blocking has a loop which operates endlessly and only
     * returns:
     *  1) In the event of an error condition
     *  2) After a call to xi_events_stop()
     */
    xi_events_process_blocking();

    /*  Cleanup the default context, releasing its memory */
    xi_delete_context( xi_context );

    /* Cleanup Xively global allocations */
    xi_shutdown();

    return 0;
}

/*
 * on_connection_state_changed() is a callback function that will be invoked whenever the
 * connection state has changed. It was set by the xi_connect_with_lastwill() call
 *
 * The provided information will alert the client application to the status of its
 * connection to the Xively service.
 *
 * The variable 'data' contains a pointer to an xi_connection_data_t structure.
 * This structure contains a 'connection_state' parameter that reflects the current state
 * of the connection.
 *
 * There are three states that has to be served in order to maintain proper connection
 * state change handling.
 *
 * POTENTIAL CONNECTION STATES ( values of data->connection_state ):
 *
 *   XI_CONNECTION_STATE_OPENED:
 *     The connection has been successfully established and TLS handshaking has verified
 *     the server certificate.
 *
 *   XI_CONNECTION_STATE_OPEN_FAILED:
 *     there was an error in the connection process and the connection could not be made.
 *     You can determine the reason in the xi_state_t which contains a standard Xively
 *     error code.
 *
 *   XI_CONNECTION_STATE_CLOSED:
 *     disconnected from Xively service. The reason is returned in xi_state_t.
 *     There are two possible reasons of being disconnected:
 *           1) The application intentionally initiated the disconnection procedure
 *           2) there has been a problem with maintaining the connection
 *
 *     each of those reasons can be easily checked using xi_state_t
 *
 *     if state == XI_STATE_OK, the application intentionally disconnected
 *     if state != XI_STATE_OK, there was a problem
 *
 * In the case of this example, the response to a successful connection is that the
 * application creates a publication request to a topic that was defined in the command
 * line parameters.
 */
void on_connection_state_changed( xi_context_handle_t in_context_handle,
                                  void* data,
                                  xi_state_t state )
{
    xi_connection_data_t* conn_data = ( xi_connection_data_t* )data;

    switch ( conn_data->connection_state )
    {
        /*
         * XI_CONNECTION_STATE_OPENED means that the connection has been
         * established and the Xively Client is ready to send and receive messages.
         */
        case XI_CONNECTION_STATE_OPENED:

            printf( "connected!\n" );
            /*
             * You can pass data to your callbacks using the user_data pointer.
             */
            void* user_data = NULL;
            /*
             * Here, we call the delayed_publish() function to publish a message
             * immediately
             */
            delayed_publish( in_context_handle, XI_INVALID_TIMED_TASK_HANDLE, user_data );
            /*
             * It's also possible to schedule a callback to occur at some time in the
             * future.
             *
             * Here we schedule a call to the function delayed_publish() to occur 5
             * seconds in the future and to repeat the call every 5 seconds forever.
             */
            delayed_publish_task_handle = xi_schedule_timed_task(
                in_context_handle, &delayed_publish, 5, 1, user_data );
            return;

        /*
         * XI_CONNECTION_STATE_OPEN_FAILED is set when there was a problem with
         * establishing  the connection to the server and the reason why has can be
         * read from the state variable.
         *
         * Xively Client hasn't established a connection and the usual behavior should be
         * to re-try the connection request."LibXively" has built in backoff system so no
         * harm is done to the servers.
         */
        case XI_CONNECTION_STATE_OPEN_FAILED:
            printf( "connection has failed reason %d\n", state );

            /*
             * this will cause the Xively Client to try to connect to the broker
             * with previously set configuration
             */

            xi_connect_with_lastwill(
                in_context_handle, conn_data->username, conn_data->password,
                conn_data->connection_timeout, conn_data->keepalive_timeout,
                conn_data->session_type, conn_data->will_topic, conn_data->will_message,
                conn_data->will_qos, conn_data->will_retain,
                &on_connection_state_changed );
            /*
             * return because we don't want to call any api function at this stage
             */
            return;

        /*
         * XI_CONNECTION_STATE_CLOSED is set when the Xively Client has been disconnected.
         *
         * The disconnection may have been caused by some external issue, or user may have
         * requested a disconnection.
         *
         * In order to distinguish between those two situation it is advised to check the
         * state variable value.
         *
         * If (state == XI_STATE_OK) it means that application has requested a
         * disconnection.
         * If (state != XI_STATE_OK) it means that the connection has been closed from
         *                           the broker.
         */
        case XI_CONNECTION_STATE_CLOSED:
            /*
             * When the connection is closed it's better to cancel some of previously
             * registered activities. Using cancel function on handler will remove the
             * handler from the timed queue which prevents the registered handler from
             * being
             * called when there is no connection.
             */
            if ( XI_INVALID_TIMED_TASK_HANDLE != delayed_publish_task_handle )
            {
                xi_cancel_timed_task( delayed_publish_task_handle );
                delayed_publish_task_handle = XI_INVALID_TIMED_TASK_HANDLE;
            }

            if ( state == XI_STATE_OK )
            {
                /*
                 * the connection has been closed intentionally. Let's stop
                 * the event processing loop as there's nothing left to do
                 * in this example.
                 */
                xi_events_stop();
            }
            else
            {
                printf( "connection closed - unknown reason %d!\n", state );
            }
            return;

        default:
            printf( "on_connection_state_changed() called with unknown connection_state "
                    "value %d\n",
                    conn_data->connection_state );
            return;
    }

    return;
}

/*
 * A function that publishes a message to the topic specified in the publishtopic
 * variable.
 */
void delayed_publish( const xi_context_handle_t context_handle,
                      const xi_timed_task_handle_t timed_task_handle,
                      void* user_data )
{
    XI_UNUSED( user_data );

    /*
     * When we have published the requested number of messages exit either cleanly
     * or abnormally, depending on the abnormal_exit flag.
     */
    if ( 0 == times_to_publish )
    {
        if ( 1 == xi_abnormalexit_flag )
        {
            /*
             * Exit without closing cleanly so that the last will message will be
             * published.
             */
            exit( -1 );
        }
        else
        {
            /*
             * Exit cleanly so that the last will message will not be published.
             */

            /* Stop the recurring timed callback calls */
            xi_cancel_timed_task( timed_task_handle );
            delayed_publish_task_handle = XI_INVALID_TIMED_TASK_HANDLE;

            /* Cleanup Xively global allocations */
            xi_shutdown_connection( context_handle );
        }
    }

    printf( "publishing msg\n" );

    /* Publish the message */
    xi_publish( context_handle, xi_publishtopic, "Hello From Xively!", xi_example_qos,
                XI_MQTT_RETAIN_FALSE, NULL, NULL );

    /* Decrement the times_to_publish counter */
    --times_to_publish;
}
