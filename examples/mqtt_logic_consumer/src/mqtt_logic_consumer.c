/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

/*
 * This example application connects to the Xively Service with
 * a username and password that you must specify.  It then subscribes
 * to a topic that you also must specify, and prints-out message
 * data that's published to the specific topic as it arrvies.
 *
 * This example application is meant as a companion example to the
 * mqtt_logic_producer example.
 */

#include "../../common/src/commandline.h"
#include <xively.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define XI_UNUSED( x ) ( void )( x )

xi_context_handle_t xi_context = XI_INVALID_CONTEXT_HANDLE;

static uint8_t connected    = 0;
static int32_t num_messages = 0;

/*
 * Forward declaration of callback functions.
 * The Documentation and Definitions of these functions can be found after the
 * implementation of main() below.
 */
void on_test_message( xi_context_handle_t in_context_handle,
                      xi_sub_call_type_t call_type,
                      const xi_sub_call_params_t* const params,
                      xi_state_t state,
                      void* user_data );
void on_connection_state_changed( xi_context_handle_t in_context_handle,
                                  void* data,
                                  xi_state_t state );

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
 *  You must specify the username and password on the command line, followed by the
 *  topic that you want the libxively_consumer to publish to.
 *  For instance:
 *      ./libxively_consumer -u your_username -P your_password -t test/topic
 */

#ifndef XI_CROSS_TARGET
int main( int argc, char* argv[] )
#else
int xi_mqtt_logic_consumer_main( xi_embedded_args_t* xi_embedded_args )
#endif
{
    char options[]       = "ha:u:P:t:";
    int missingparameter = 0;
    int retval           = 0;

#ifndef XI_CROSS_TARGET
    printf( "%s\n%s\n", argv[0], xi_cilent_version_str );

    /* Parse the argv array for ONLY the options specified in the options string */
    retval = xi_parse( argc, argv, options, sizeof( options ) );
#else
    printf( "mqtt_logic_consumer\n%s\n", xi_cilent_version_str );
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
    if ( NULL == xi_subscribetopic )
    {
        missingparameter = 1;
        printf( "-t --subscribetopic is required\n" );
    }
    if ( 1 == missingparameter )
    {
        exit( -1 );
    }

    /* initialize xi library and create a context to use to connect to the Xively Service
     * Device-id must be the same as username
     */
    const xi_state_t error_init = xi_initialize( xi_account_id );

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
    const uint16_t connection_timeout = 10;
    const uint16_t keepalive_timeout  = 20;

    xi_connect( xi_context, xi_username, xi_password, connection_timeout,
                keepalive_timeout, XI_SESSION_CLEAN, &on_connection_state_changed );


    /*
     * The Xively Client was designed for single threaded devices.
     *
     * As such it does not have its own event loop thread. Instead you must call
     * the function xi_platform_event_loop() to process connection requests and to
     * regularly check the sockets for incoming data.
     *
     * This implementation has a loop  which operates endlessly and only returns:
     *  1) In the event of an error condition
     *  2) After a call to xi_shutdown_connection()
     *  3) After a call to xi_events_stop()
     *
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
    XI_UNUSED( in_context_handle );

    xi_connection_data_t* conn_data = ( xi_connection_data_t* )data;

    switch ( conn_data->connection_state )
    {
        /* XI_CONNECTION_STATE_OPENED means that the connection has been
           established and the libxibely is ready to send/recv messages */
        case XI_CONNECTION_STATE_OPENED:
            printf( "connected!\n" );
            break;

        /* XI_CONNECTION_STATE_OPEN_FAILED is set when there was a problem
           with establishing the connection to the server and the reason why
           has can be read from the state variable. Xively Client hasn't
           established
           a connection and the usual behaviour should be to re-try. "LibXively"
           has built in backoff system so no harm is done to the servers. */
        case XI_CONNECTION_STATE_OPEN_FAILED:
            printf( "connection has failed reason %d\n", state );

            /* this will cause the Xively Client to try to connect to the
               servers
                with previously set configuration */
            xi_connect( in_context_handle, conn_data->username, conn_data->password,
                        conn_data->connection_timeout, conn_data->keepalive_timeout,
                        conn_data->session_type, &on_connection_state_changed );

            /* return because we don't want to call any api function at this
               stage */
            return;

        /* XI_CONNECTION_STATE_CLOSED is set when the Xively Client has been
           disconnected. The disconnection may have been caused by some external
           issue, or user may have requested a disconnection. In order to
           distinguish between those two situation it is advised to check the state
           variable value.

           If the state == XI_STATE_OK it means that user has requested a
           disconnection

           If the state != XI_STATE_OK it meanst that the connection has been
           closed from one side.
          */
        case XI_CONNECTION_STATE_CLOSED:
            if ( state == XI_STATE_OK )
            {
                /* the connection has been closed intentionally. Let's stop
                 * the event processing loop as there's nothing left to do
                 * in this example.
                 */
                xi_events_stop();
            }
            else
            {
                printf( "connection closed - reason %d!\n", state );

                /* this will cause the libXively to try to connect to the
                   servers with previously set configuration */
                xi_connect( in_context_handle, conn_data->username, conn_data->password,
                            conn_data->connection_timeout, conn_data->keepalive_timeout,
                            conn_data->session_type, &on_connection_state_changed );
            }

            return;
        default:
            printf( "wrong value\n" );
            return;
    }

    if ( 0 == connected )
    {
        /* You can use any value here to help you handle your requests */
        void* user_data = NULL;

        /* Create the Subscription Requeust */
        xi_subscribe( xi_context, xi_subscribetopic, xi_example_qos, &on_test_message,
                      user_data );
    }
}

/* A callback function that will be invoked whenever someone publishes to the
   subscribed test topic.  Note that the params parameter contains different information
   depending on the provided state.

   call_type values:
    XI_SUB_CALL_UNKNOWN - if there was problem or error, please reach the state argument
   for details
    XI_SUB_CALL_SUBACK - callback is called upon the subscription finalize notification
   the params parameter contains structure that holds suback status in suback struct that
   should be used in order to verify the subscription state
    XI_SUB_CALL_MESSAGE - there has been message received, in order to reach the message
   payload and detailed information please read from the params parameter message struct
   which contains:
    - infromation about message ( topic, payload, message_id )
    - message flags( retain, qos, dup )

   params xi_sub_call_params_t.suback fields:
    - topic - pointer to the zero string contains name of the topic wchich this
   subscription callback invocation refers to
    - suback_status - result of subscription request

   params xi_sub_call_params_t.message fields:
    - topic - same as in case of suback structure
    - payload - pointer containing payload data ( see NOTES! )
    - message_id - mqtt message id
    - flags( retain, qos, dup )

   NOTE: The message payload should not be released or mutated there is only read access
   to the data, payload memory itself will be free'd after the callback invcation so it's
   not safe to use the pointer afterwards.
*/
void on_test_message( xi_context_handle_t in_context_handle,
                      xi_sub_call_type_t call_type,
                      const xi_sub_call_params_t* const params,
                      xi_state_t state,
                      void* user_data )
{
    /* In this example application we do not need to do any
       logical checks on the context.  We invoke the XI_UNUSED macro
       to suppresss any compiler warnings for an unused parameter */
    XI_UNUSED( in_context_handle );
    XI_UNUSED( state );
    XI_UNUSED( user_data );

    switch ( call_type )
    {
        case XI_SUB_CALL_SUBACK:
            if ( params->suback.suback_status == XI_MQTT_SUBACK_FAILED )
            {
                printf( "Subscription for topic %s failed.\n", params->suback.topic );
            }
            else
            {
                printf( "Subscription for topic %s successfull with QoS %d.\n",
                        params->suback.topic, ( int )params->suback.suback_status );
            }
            break;
        case XI_SUB_CALL_MESSAGE:
            /* A publication to the subscribe topic has been read! */
            printf( "received message:\n" );

            /* Now you have access to some detailed information about the message such as:
             *  topic name, message id and flags */
            printf( "on topic: %s retain: %s, qos: %d, dup: %s\n", params->message.topic,
                    params->message.retain == XI_MQTT_RETAIN_TRUE ? "RETAIN_TRUE"
                                                                  : "RETAIN_FALSE",
                    ( int )params->message.qos,
                    params->message.dup_flag == XI_MQTT_DUP_TRUE ? "DUP_TRUE"
                                                                 : "DUP_FALSE" );

            /* get the payload parameter that contains the data that was sent. */
            const uint8_t* payload_data = params->message.temporary_payload_data;
            XI_UNUSED( payload_data );
            size_t payload_length = params->message.temporary_payload_data_length;

            /* NOTE: Payload memory will be automaticaly released after the callback
               invokation is finished and these pointers will point to released memory
               blocks.*/
            size_t i = 0;
            for ( ; i < payload_length; ++i )
            {
                printf( "%c", payload_data[i] );
            }

            printf( "\n" );

            /* TEMPORARY mark num_messages as used until the short_topic_names branch gets
             * merged, then remove this */
            XI_UNUSED( num_messages );

            /* Just a count to see how many messages are streaming in. */
            num_messages += 1;
            printf( "total num messages: %d\n", num_messages );

            /* Ensure that our output reaches the terminal */
            fflush( stdout );

            /* You don't have to free the memory where payload is stored. LibXively will
             * automatically free the memory after the callback invocation is completed.
             */

            break;
        default:
            break;
    }
}
