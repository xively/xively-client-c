/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xively.h>
#include <xi_helpers.h>
#include <xi_mqtt_message.h>
#include <xi_globals.h>
#include "commandline.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

xi_context_handle_t xi_context = XI_INVALID_CONTEXT_HANDLE;

static char* test_topic2    = NULL;
static char* shutdown_topic = NULL;
static uint8_t should_stop  = 0;

static xi_timed_task_handle_t delayed_publish_task = XI_INVALID_TIMED_TASK_HANDLE;

xi_state_t
on_test_message( xi_context_handle_t in_context_handle, void* data, xi_state_t state )
{
    XI_UNUSED( in_context_handle );

    switch ( state )
    {
        case XI_MQTT_SUBSCRIPTION_FAILED:
            xi_debug_logger( "Subscription failed." );
            return XI_STATE_OK;
        case XI_MQTT_SUBSCRIPTION_SUCCESSFULL:
        {
            xi_mqtt_suback_status_t status = ( xi_mqtt_suback_status_t )( intptr_t )data;
            xi_debug_format( "Subscription successfull with QoS %d", status );
            XI_UNUSED( status );
            return XI_STATE_OK;
        }
        case XI_STATE_OK:
        {
            xi_mqtt_message_t* msg = ( xi_mqtt_message_t* )data;
            xi_debug_logger( "received message: " );
            xi_debug_mqtt_message_dump( msg );
            xi_debug_logger( "" );

            if ( msg->publish.content )
                xi_debug_data_desc_dump( msg->publish.content );

            printf( "\n" );

            fflush( stdout );

            xi_mqtt_message_free( &msg );

            return XI_STATE_OK;
        }

        default:
            return state;
    }
}

xi_state_t
on_test2_message( xi_context_handle_t in_context_handle, void* data, xi_state_t state )
{
    XI_UNUSED( in_context_handle );

    switch ( state )
    {
        case XI_MQTT_SUBSCRIPTION_FAILED:
            xi_debug_logger( "Subscription failed." );
            return XI_STATE_OK;
        case XI_MQTT_SUBSCRIPTION_SUCCESSFULL:
        {
            xi_mqtt_suback_status_t status = ( xi_mqtt_suback_status_t )( intptr_t )data;
            xi_debug_format( "Subscription successfull with QoS %d", status );
            XI_UNUSED( status );
            return XI_STATE_OK;
        }
        case XI_STATE_OK:
        {
            xi_mqtt_message_t* msg = ( xi_mqtt_message_t* )data;
            xi_debug_logger( "received message: " );
            xi_debug_mqtt_message_dump( msg );
            xi_debug_logger( "" );

            xi_debug_data_desc_dump( msg->publish.content );

            printf( "\n" );

            fflush( stdout );

            xi_mqtt_message_free( &msg );

            return XI_STATE_OK;
        }

        default:
            return state;
    }
}

void delayed_publish( xi_context_handle_t context_handle,
                      xi_timed_task_handle_t timed_task_handle,
                      void* user_data )
{
    XI_UNUSED( user_data );

    static unsigned int counter = 0;

    if ( counter++ == 2 )
    {
        xi_cancel_timed_task( timed_task_handle );
        delayed_publish_task = XI_INVALID_TIMED_TASK_HANDLE;
        xi_shutdown_connection( context_handle );
        counter = 0;
        return;
    }

    // sending the connect request
    int i = 0;
    for ( ; i < 5; ++i )
    {
        char msg[2048] = {'\0'};
        memset( msg, 'a', sizeof( msg ) - 1 );
        xi_publish( context_handle, publishtopic, msg, XI_MQTT_QOS_AT_LEAST_ONCE,
                    XI_MQTT_RETAIN_FALSE, NULL, NULL );
    }

    xi_publish_timeseries( context_handle, publishtopic,
                           ( rand() / ( float )RAND_MAX ) * 256,
                           XI_MQTT_QOS_AT_LEAST_ONCE, NULL, NULL );
}

void first_delayed_call_before_publish( xi_context_handle_t context_handle,
                                        xi_timed_task_handle_t timed_task_handle,
                                        void* user_data )
{
    XI_UNUSED( timed_task_handle );
    XI_UNUSED( user_data );
    delayed_publish_task =
        xi_schedule_timed_task( context_handle, delayed_publish, 1, 1, NULL );
}

xi_state_t
on_connected( xi_context_handle_t in_context_handle, void* data, xi_state_t state )
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

            return XI_STATE_OK;
        case XI_CONNECTION_STATE_OPENED:
            printf( "connected to %s:%d\n", conn_data->host, conn_data->port );
            break;
        case XI_CONNECTION_STATE_CLOSED:
            printf( "connection has been closed reason %d\n", state );

            if ( XI_INVALID_TIMED_TASK_HANDLE != delayed_publish_task )
            {
                xi_cancel_timed_task( delayed_publish_task );
                delayed_publish_task = XI_INVALID_TIMED_TASK_HANDLE;
            }

            if ( state == XI_STATE_OK )
            {
                if ( should_stop > 10 )
                {
                    xi_evtd_instance_t* event_dispatcher = xi_globals.evtd_instance;
                    xi_evtd_stop( event_dispatcher );
                }
                else
                {
                    xi_connect( in_context_handle, conn_data->username,
                                conn_data->password, conn_data->connection_timeout,
                                conn_data->keepalive_timeout, conn_data->session_type,
                                &on_connected );

                    /*  add one to should_stop to make
                        it disconnect after some number
                        of tries */
                    should_stop += 1;
                }
            }
            else
            {
                xi_connect( in_context_handle, conn_data->username, conn_data->password,
                            conn_data->connection_timeout, conn_data->keepalive_timeout,
                            conn_data->session_type, &on_connected );
            }

            return XI_STATE_OK;
        default:
            printf( "invalid parameter %d\n", conn_data->connection_state );
            return XI_INVALID_PARAMETER;
    }

    // sending the connect request
    // xi_publish_to_topic( publishtopic, "test_msg01", XI_MQTT_QOS_AT_LEAST_ONCE
    // );
    // xi_publish_to_topic( publishtopic, "test_msg02", XI_MQTT_QOS_AT_LEAST_ONCE
    // );
    xi_publish( in_context_handle, publishtopic, "test_msg03", XI_MQTT_QOS_AT_LEAST_ONCE,
                XI_MQTT_RETAIN_FALSE, NULL, NULL );

    /* You can pass any custom data to your callbacks if you want. */
    void* user_data = NULL;
    // register delayed publish
    xi_schedule_timed_task( in_context_handle, &first_delayed_call_before_publish, 2, 0,
                            user_data );

    if ( conn_data->session_type == XI_SESSION_CLEAN ||
         should_stop == 0 ) // means we are connecting for the first time
    {
        xi_subscribe( in_context_handle, publishtopic, XI_MQTT_QOS_AT_LEAST_ONCE,
                      &on_test_message );

        if ( test_topic2 != 0 )
        {
            xi_subscribe( in_context_handle, test_topic2, XI_MQTT_QOS_AT_LEAST_ONCE,
                          &on_test2_message );
        }
    }


    return XI_STATE_OK;
}

const char* get_unique_device_id()
{
    // this needs to return an identifier unique to this particlar device. For
    // instance, the device's serial number in string form.
    return "";
}

#ifndef XI_CROSS_TARGET
int main( int argc, char* argv[] )
#else
int xi_mqtt_logic_cont_sess_example_main( int argc, char* argv[] )
#endif
{
    char* local_shutdown_topic        = "shutdown_topic";
    const uint16_t connection_timeout = 10;
    const uint16_t keepalive_timeout  = 20;
    char options[]                    = "hu:P:t:";
    printf( "\n%s\nMajor = %d Minor = %d Revision = %d\n", xi_cilent_version_str,
            xi_major, xi_minor, xi_revision );

    /* Parse the argv array for ONLY the options specified in the options string */
    parse( argc, argv, options, sizeof( options ) );

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

    shutdown_topic = local_shutdown_topic;


    xi_connect( xi_context, username, password, connection_timeout, keepalive_timeout,
                XI_SESSION_CONTINUE, &on_connected );

    xi_platform_event_loop();

    xi_delete_context( xi_context );

    xi_shutdown();

    return 0;
}
