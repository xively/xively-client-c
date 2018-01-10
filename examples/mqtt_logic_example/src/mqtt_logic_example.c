/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "../../common/src/commandline.h"
#include <xively.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define XI_UNUSED( x ) ( void )( x )

xi_context_handle_t xi_context = XI_INVALID_CONTEXT_HANDLE;

static const char* const test_topic2 = "test_topic_2";
static char* shutdown_topic          = NULL;

static xi_timed_task_handle_t delayed_publish_task = XI_INVALID_TIMED_TASK_HANDLE;

void on_test_message( xi_context_handle_t in_context_handle,
                      xi_sub_call_type_t call_type,
                      const xi_sub_call_params_t* const params,
                      xi_state_t state,
                      void* user_data )
{
    XI_UNUSED( in_context_handle );
    XI_UNUSED( state );
    XI_UNUSED( user_data );

    switch ( call_type )
    {
        case XI_SUB_CALL_SUBACK:
            if ( params->suback.suback_status == XI_MQTT_SUBACK_FAILED )
            {
                printf( "topic:%s. Subscription failed.\n", params->suback.topic );
            }
            else
            {
                printf( "topic:%s. Subscription granted %d.\n", params->suback.topic,
                        ( int )params->suback.suback_status );
            }
            return;
        case XI_SUB_CALL_MESSAGE:
            printf( "topic:%s. message received.\n", params->message.topic );
            return;
        default:
            return;
    }
}

void on_shutdown_message( xi_context_handle_t in_context_handle,
                          xi_sub_call_type_t call_type,
                          const xi_sub_call_params_t* const params,
                          xi_state_t state,
                          void* user_data )
{
    XI_UNUSED( state );
    XI_UNUSED( user_data );

    switch ( call_type )
    {
        case XI_SUB_CALL_SUBACK:
            if ( params->suback.suback_status == XI_MQTT_SUBACK_FAILED )
            {
                printf( "topic:%s. Subscription failed.\n", params->suback.topic );
            }
            else
            {
                printf( "topic:%s. Subscription granted %d.\n", params->suback.topic,
                        ( int )params->suback.suback_status );
            }
            return;
        case XI_SUB_CALL_MESSAGE:
        {
            printf( "received shutdown message\n" );
            xi_shutdown_connection( in_context_handle );

            return;
        }
        default:
            return;
    }
}

void on_publish_delivered( xi_context_handle_t in_context_handle,
                           void* in_data,
                           xi_state_t state )
{
    XI_UNUSED( in_context_handle );

    int i = ( int )( intptr_t )in_data;

    XI_UNUSED( i );

    if ( state == XI_STATE_OK )
    {
        printf( "delivered msg %d!\n", i );
    }
    else
    {
        printf( "msg not delivered reason %d!\n", state );
    }
}

void delayed_publish( xi_context_handle_t context_handle,
                      xi_timed_task_handle_t timed_task_handle,
                      void* user_data )
{
    XI_UNUSED( timed_task_handle );
    XI_UNUSED( user_data );

    int i = 0;
    for ( ; i < 5; ++i )
    {
        char msg[2048] = {'\0'};
        memset( msg, 'a', sizeof( msg ) - 1 );
        msg[0] = i + '0'; /* user data so that we can tell the
                             messages apart... */

        xi_publish( context_handle, xi_publishtopic, msg, xi_example_qos,
                    XI_MQTT_RETAIN_FALSE, on_publish_delivered, ( void* )( intptr_t )i );
    }

    xi_publish_timeseries( context_handle, xi_publishtopic,
                           ( rand() / ( float )RAND_MAX ) * 256, xi_example_qos, NULL,
                           NULL );
}

void first_delay_before_publish( xi_context_handle_t context_handle,
                                 xi_timed_task_handle_t timed_task_handle,
                                 void* user_data )
{
    XI_UNUSED( timed_task_handle );
    delayed_publish( context_handle, XI_INVALID_TIMED_TASK_HANDLE, user_data );
#ifndef XI_CROSS_TARGET
    int delay = 1;
#else
    int delay = 20;
#endif

    delayed_publish_task =
        xi_schedule_timed_task( context_handle, delayed_publish, delay, 1, NULL );
}

void on_connected( xi_context_handle_t in_context_handle, void* data, xi_state_t state )
{
    xi_connection_data_t* conn_data = ( xi_connection_data_t* )data;

    switch ( conn_data->connection_state )
    {
        case XI_CONNECTION_STATE_OPEN_FAILED:
            printf( "connection to %s:%d has failed reason %d\n", conn_data->host,
                    conn_data->port, state );

            xi_connect( in_context_handle, conn_data->username, conn_data->password,
                        conn_data->connection_timeout, conn_data->keepalive_timeout,
                        conn_data->session_type, &on_connected );

            return;
        case XI_CONNECTION_STATE_OPENED:
            printf( "connected to %s:%d\n", conn_data->host, conn_data->port );
            break;
        case XI_CONNECTION_STATE_CLOSED:
            printf( "connection closed - reason %d!\n", state );

            if ( XI_INVALID_TIMED_TASK_HANDLE != delayed_publish_task )
            {
                xi_cancel_timed_task( delayed_publish_task );
                delayed_publish_task = XI_INVALID_TIMED_TASK_HANDLE;
            }

            if ( XI_STATE_OK == state )
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
                xi_connect( in_context_handle, conn_data->username, conn_data->password,
                            conn_data->connection_timeout, conn_data->keepalive_timeout,
                            conn_data->session_type, &on_connected );
            }
            return;
        default:
            printf( "invalid parameter %d\n", conn_data->connection_state );
            return;
    }

    /* publish a test message */
    xi_publish( in_context_handle, xi_publishtopic, "test message.", xi_example_qos,
                XI_MQTT_RETAIN_FALSE, NULL, NULL );

    /* You can pass any custom data to your callbacks if you want. */
    void* user_data = NULL;

    /* register delayed publish */
    delayed_publish_task = xi_schedule_timed_task(
        in_context_handle, first_delay_before_publish, 3, 0, user_data );

    xi_subscribe( in_context_handle, xi_publishtopic, xi_example_qos, &on_test_message,
                  NULL );

    /* here we subscribe for second topic that will use the same callback function */
    if ( NULL != test_topic2 )
    {
        xi_subscribe( in_context_handle, test_topic2, xi_example_qos, &on_test_message,
                      NULL );
    }

    /* here we subscribe for the third topic that uses separate callback function, you
     * might want to separate different types of logic from each other so sometimes it's
     * easier to use separate callback function */
    if ( NULL != shutdown_topic )
    {
        xi_subscribe( in_context_handle, shutdown_topic, xi_example_qos,
                      &on_shutdown_message, NULL );
    }
}

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

#ifndef XI_CROSS_TARGET
int main( int argc, char* argv[] )
#else
int xi_mqtt_logic_example_main( xi_embedded_args_t* xi_embedded_args )
#endif
{
    char* local_shutdown_topic        = "shutdown_topic";
    const uint16_t connection_timeout = 10;
    const uint16_t keepalive_timeout  = 20;
    char options[]                    = "ha:u:P:t:p:";
    int missingparameter              = 0;
    int retval                        = 0;

#ifndef XI_CROSS_TARGET
    printf( "%s\n%s\n", argv[0], xi_cilent_version_str );

    /* Parse the argv array for ONLY the options specified in the options string */
    retval = xi_parse( argc, argv, options, sizeof( options ) );
#else
    printf( "mqtt_logic_example\n%s\n", xi_cilent_version_str );
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
        printf( "-p --xi_publishtopic is required\n" );
    }
    if ( 1 == missingparameter )
    {
        exit( -1 );
    }

    if ( 0 < xi_memorylimit )
    {
        printf( "xi_memorylimit = %d\n", xi_memorylimit );

        xi_state_t result = xi_set_maximum_heap_usage( xi_memorylimit );
        if ( XI_NOT_SUPPORTED == result )
        {
            printf( "warning: the Xively Client was not built with the Memory Limiter "
                    "enabled\n" );
        }
        else if ( XI_STATE_OK != result )
        {
            printf( "xi_memory_limiter_set_limit() returned error code %d\n", result );
            exit( -1 );
        }
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

    /* create a context to use
       to connect to the Xively Service */
    xi_context = xi_create_context();
    if ( XI_INVALID_CONTEXT_HANDLE >= xi_context )
    {
        printf( " xi failed to create context, error: %d\n", -xi_context );
        return -1;
    }

    shutdown_topic = local_shutdown_topic;


    xi_connect( xi_context, xi_username, xi_password, connection_timeout,
                keepalive_timeout, XI_SESSION_CLEAN, &on_connected );


    xi_events_process_blocking();

    xi_delete_context( xi_context );

    xi_shutdown();

    size_t current_heap_usage = 0;
    xi_state_t result         = xi_get_heap_usage( &current_heap_usage );
    if ( XI_NOT_SUPPORTED != result )
    {
        if ( XI_STATE_OK == result )
        {
            printf( "--== Memory @exit: " );
            printf( "%zu ==--", current_heap_usage );
        }
        else
        {
            printf( "--== Memory @exit: " );
            printf( "failed to retrieve heap usage at shutdown\n" );
        }
    }

    return 0;
}
