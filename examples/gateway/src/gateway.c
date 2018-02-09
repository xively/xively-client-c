/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "../../common/src/commandline.h"
#include <xively.h>
#include <xively_gateway.h>

#include <stdio.h>

static xi_context_handle_t xi_context;

void on_connection_state_changed( xi_context_handle_t in_context_handle,
                                  void* data,
                                  xi_state_t state );

int main( int argc, char* argv[] )
{
    char options[]       = "ha:u:P:p:m:";
    int missingparameter = 0;
    int retval           = 0;

#ifndef XI_CROSS_TARGET
    printf( "%s\n%s\n", argv[0], xi_cilent_version_str );

    /* Parse the argv array for ONLY the options specified in the options string */
    retval = xi_parse( argc, argv, options, sizeof( options ) );
#else
    printf( "firmware_update_main\n%s\n", xi_cilent_version_str );
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
    const xi_state_t error_init = xi_initialize( xi_account_id );

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
        the on_connection_state_changed() handler by calling xi_events_stop();
    */
    xi_events_process_blocking();

    /* Cleanup the default context, releasing its memory */
    xi_delete_context( xi_context );

    /* Cleanup Xively global allocations */
    xi_shutdown();

    return 0;
}

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
