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
 * This example application is meant as a companion example to the
 * mqtt_logic_consumer example.
 */

#include "../../common/src/commandline.h"
#include <xively.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define XI_UNUSED( x ) ( void )( x )

xi_context_handle_t xi_context = XI_INVALID_CONTEXT_HANDLE;

static xi_timed_task_handle_t delayed_publish_task = XI_INVALID_TIMED_TASK_HANDLE;

void on_connection_state_changed( xi_context_handle_t in_context_handle,
                                  void* data,
                                  xi_state_t state );
void delayed_publish( xi_context_handle_t context_handle,
                      xi_timed_task_handle_t timed_task,
                      void* user_data );

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

/*  -main-
    The main entry point for this example binary.
    There are two different ways of starting this application:
      1: If you already have a username and password stored in
         libxively_producer.creds (see below) then you need only
         specify on the topic name that you want the mqtt_logic_consumer to pubilsh to.
         For instance:
            ./mqtt_logic_producer -p test/topic -m "Test Message"
      2: If the file libxively_producer.creds does not exist or is empty then you must
         specify the username and password on the command line, followed by the
         topic that you want to subscribe to.
         For instance:
            ./mqtt_logic_producer -u your_username -P your_password -p test/topic \
                                  -m "Test Message"

       This example will publish a message to the test/topic and print incoming messages.

    For information on creating username and password credentials for your devices
    to access to Xively Service during development then please contact Xively Client
    Services. */

#ifndef XI_CROSS_TARGET
int main( int argc, char* argv[] )
#else
int xi_mqtt_logic_producer_main( xi_embedded_args_t* xi_embedded_args )
#endif
{
    char options[]       = "ha:u:P:p:m:";
    int missingparameter = 0;
    int retval           = 0;

#ifndef XI_CROSS_TARGET
    printf( "%s\n%s\n", argv[0], xi_cilent_version_str );

    /* Parse the argv array for ONLY the options specified in the options string */
    retval = xi_parse( argc, argv, options, sizeof( options ) );
#else
    printf( "mqtt_logic_producer\n%s\n", xi_cilent_version_str );
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
    if ( 1 == missingparameter )
    {
        exit( -1 );
    }

    /* initialize xi library and create a context to use to connect to the Xively Service
     * Device-id must be the same as username
     */
    const xi_state_t error_init = xi_initialize( xi_account_id, xi_username );

    if ( XI_STATE_OK != error_init )
    {
        printf( " xi failed to initialize, error: %d\n", error_init );
        return -1;
    }

    /*  Create a connection context. A context represents a Connection
        on a single socket, and can be used to publish and subscribe
        to numerous topics.  If you would like to use more than one
        connection then please use xi_create_context to create more
        contexts. */
    xi_context = xi_create_context();
    if ( XI_INVALID_CONTEXT_HANDLE >= xi_context )
    {
        printf( " xi failed to create context, error: %d\n", -xi_context );
        return -1;
    }

    /*  Create a connection request with the credentials.
        The topic name will be used for publication requests
        only after the connecton has been established.
        The 'on_connection_state_changed' parameter is the name of the callback
        function after the connection request completes, and its
        implementation should handle both succesfull connections
        and unsuccesfull connections as well as disconnections. */
    const uint16_t connection_timeout = 10;
    const uint16_t keepalive_timeout  = 20;

    xi_connect( xi_context, xi_username, xi_password, connection_timeout,
                keepalive_timeout, XI_SESSION_CLEAN, &on_connection_state_changed );

    /* The Xively Client was designed for single threaded devices. As such
        it does not have its own event loop thread. Instead you must regularly call
        the function xi_events_process_blocking() to process connection requeusts, and to
        regularly check the sockets for incoming data.
        This implemetnation has the loop operate endlessly. The loop will stop after
        closing the connection, using xi_shutdown_connection(). And from
        the on_connection_state_changed() handler by calling xi_events_stop(); */
    xi_events_process_blocking();

    /*  Cleanup the default context, releasing its memory */
    xi_delete_context( xi_context );

    /* Cleanup Xively global allocations */
    xi_shutdown();

    return 0;
}

/* A callback function that will be invoked whenever the connection state
   has changed.
   The provided information will alert the client application to the status of its
   connection to the Xively service.

   DATA:
    contains the xi_connection_data_t which holds a connection_state parameter
    that reflects the current state of a connection.

    There are three states that has to be served in order to maintain proper
    connection state change handling.

   POTENTIAL CONNECTION STATES ( values of data->connection_state ):

    XI_CONNECTION_STATE_OPENED - the connection has been succesfully established,
        and TLS handshaking has verified the server certificate.
    XI_CONNECTION_STATE_OPEN_FAILED - there was an error in the connection
        process and the connection could not be made. You can determine the
        reason in the xi_state_t which contains a standard Xively error code.
    XI_CONNECTION_STATE_CLOSED - disconnected from Xively service, reason in xi_state_t
        Note: there are two possible reasons of being disconnected:
            a) user intentionally initiated disconnection procedure
            b) there has been a problem with maintaining the connnection

        Each of those reasons can be easly checked using xi_state_t

        if state == XI_STATE_OK  a) user intentionally disconnected
        if state != XI_STATE_OK  b) there was a problem

        In the case of this example,
        the response to a successful connection is that the application creates a
                publication request to a topic that was defined in the command line
                    parameters. */
void on_connection_state_changed( xi_context_handle_t in_context_handle,
                                  void* data,
                                  xi_state_t state )
{
    xi_connection_data_t* conn_data = ( xi_connection_data_t* )data;

    switch ( conn_data->connection_state )
    {
        /* XI_CONNECTION_STATE_OPENED means that the connection has been
           established and the Xively Client is ready to send/recv messages */
        case XI_CONNECTION_STATE_OPENED:
            printf( "connected!\n" );
            /* You can pass any custom data to your callbacks if you want. */
            void* user_data = NULL;
            delayed_publish( in_context_handle, XI_INVALID_TIMED_TASK_HANDLE, user_data );
            delayed_publish_task = xi_schedule_timed_task(
                in_context_handle, delayed_publish, 5, 1, user_data );
            break;
        /* XI_CONNECTION_STATE_OPEN_FAILED is set when there was a problem
           with establishing the connection to the server and the reason why
           has can be read from the state variable. Xively Client hasn't established
           a connection and the usual behaviour should be to re-try. "LibXively"
           has built in backoff system so no harm is done to the servers. */
        case XI_CONNECTION_STATE_OPEN_FAILED:
            printf( "connection has failed reason %d\n", state );

            /* this will cause the Xively Client to try to connect to the servers
               with previously set configuration */
            xi_connect( in_context_handle, conn_data->username, conn_data->password,
                        conn_data->connection_timeout, conn_data->keepalive_timeout,
                        conn_data->session_type, &on_connection_state_changed );

            /* return because we don't want to call any api function at this stage */
            break;
        /* XI_CONNECTION_STATE_CLOSED is set when the Xively Client has been
           disconnected. The disconnection may have been caused by some external
           issue, or user may have requested a disconnection. In order to distinguish
           between those two situation it is advised to check the state variable
           value.
           If the state == XI_STATE_OK it means that user has requested a
           disconnection
           if the state != XI_STATE_OK it meanst that the connection has been closed
           from
           one side.
          */
        case XI_CONNECTION_STATE_CLOSED:
            /*
              When the connection is closed it's better to cancel some of previously
              registered activities. Using cancel function on handler will remove the
              handler from the timed queue which prevents the registered handle to be
              called when there is no connection.
            */
            if ( XI_INVALID_TIMED_TASK_HANDLE != delayed_publish_task )
            {
                xi_cancel_timed_task( delayed_publish_task );
                delayed_publish_task = XI_INVALID_TIMED_TASK_HANDLE;
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
                printf( "connection closed - reason %d!\n", state );

                /* this will cause the Xively Client to try to connect to the servers
                    with previously set configuration */
                xi_connect( in_context_handle, conn_data->username, conn_data->password,
                            conn_data->connection_timeout, conn_data->keepalive_timeout,
                            conn_data->session_type, &on_connection_state_changed );
            }

            break;
        default:
            printf( "wrong value\n" );
            break;
    }
}

/* A function that publishes to the topic specified on the command
   line.  The function is named delayed_publish not becuase the publish
   requeust that it makes is delayed, but because after publication it
   schedules the Xively event loop to invoked it again in 5 seconds.
   So in this way, the example application will publish 'Hello From Xively!'
   in a constant loop every 5 seconds. */
void delayed_publish( xi_context_handle_t context_handle,
                      xi_timed_task_handle_t timed_task,
                      void* user_data )
{
    XI_UNUSED( timed_task );
    XI_UNUSED( user_data );

    printf( "publishing msg\n" );

    /* sending the connect request */
    xi_publish( context_handle, xi_publishtopic, "Hello From Xively!", xi_example_qos,
                XI_MQTT_RETAIN_FALSE, NULL, NULL );
}
