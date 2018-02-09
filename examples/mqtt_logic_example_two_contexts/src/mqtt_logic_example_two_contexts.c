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

static char* test_topic2    = NULL;
static char* shutdown_topic = NULL;
static uint8_t connected    = 0;

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
            break;
        case XI_SUB_CALL_MESSAGE:
            printf( "topic:%s. message received.\n", params->message.topic );
            break;
        default:
            break;
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
            break;
        case XI_SUB_CALL_MESSAGE:
        {
            printf( "received shutdown message: \n" );

            xi_shutdown_connection( in_context_handle );

            break;
        }
        default:
            break;
    }
}

void delayed_publish( xi_context_handle_t context_handle,
                      xi_timed_task_handle_t timed_task,
                      void* user_data )
{
    XI_UNUSED( timed_task );
    XI_UNUSED( user_data );

    /* sending the connect request */
    int i = 0;
    for ( ; i < 5; ++i )
    {
        char msg[128] = {'\0'};
        sprintf( msg, "test msg delayed %d @ %d", i, ( int )time( 0 ) );
        xi_publish( context_handle, xi_publishtopic, msg, xi_example_qos,
                    XI_MQTT_RETAIN_FALSE, NULL, NULL );
    }
}

void first_delay_before_publish( xi_context_handle_t context_handle,
                                 xi_timed_task_handle_t timed_task,
                                 void* user_data )
{
    XI_UNUSED( timed_task );

    delayed_publish( context_handle, XI_INVALID_TIMED_TASK_HANDLE, user_data );
    xi_schedule_timed_task( context_handle, delayed_publish, 1, 1, user_data );
}

void on_connected2nd( xi_context_handle_t in_context_handle,
                      void* data,
                      xi_state_t state )
{
    XI_UNUSED( in_context_handle );
    XI_UNUSED( data );

    if ( state == XI_STATE_OK )
    {
        printf( "2nd connected!\n" );
    }
    else
    {
        printf( "2nd error while connecting!\n" );
    }

    xi_publish( in_context_handle, xi_publishtopic, "test_msg03", xi_example_qos,
                XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

void on_connected( xi_context_handle_t in_context_handle, void* data, xi_state_t state )
{
    XI_UNUSED( data );

    if ( state == XI_STATE_OK )
    {
        printf( "connected!\n" );
    }
    else
    {
        printf( "error while connecting!\n" );
    }

    xi_publish( in_context_handle, xi_publishtopic, "test_msg03", xi_example_qos,
                XI_MQTT_RETAIN_FALSE, NULL, NULL );

    if ( connected == 0 ) /* we want to call it only once */
    {                     /* register delayed publish */
        /* You can pass any custom data to your callbacks if you want. */
        void* user_data = NULL;
        xi_schedule_timed_task( in_context_handle, first_delay_before_publish, 3, 0,
                                user_data );
    }

    xi_subscribe( in_context_handle, xi_publishtopic, xi_example_qos, &on_test_message,
                  ( void* )1 );

    if ( test_topic2 != 0 )
    {
        xi_subscribe( in_context_handle, test_topic2, xi_example_qos, &on_test_message,
                      ( void* )2 );
    }

    connected = 1;
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

#define CTX_2ND 1

#ifndef XI_CROSS_TARGET
int main( int argc, char* argv[] )
#else
int xi_mqtt_logic_example_two_contexts( xi_embedded_args_t* xi_embedded_args )
#endif
{
    char* local_shutdown_topic = "shutdown_topic";
    char options[]             = "ha:u:P:p:";
    int missingparameter       = 0;
    int retval                 = 0;

#ifndef XI_CROSS_TARGET
    printf( "%s\n%s\n", argv[0], xi_cilent_version_str );

    /* Parse the argv array for ONLY the options specified in the options string */
    retval = xi_parse( argc, argv, options, sizeof( options ) );
#else
    printf( "mqtt_logic_example_two_contexts\n%s\n", xi_cilent_version_str );
    retval = xi_embedded_parse( xi_embedded_args, options, sizeof( options ) );
#endif /* XI_CROSS_TARGET */

    if ( -1 == retval )
    {
        exit( -1 );
    }

    /* Check to see that the required paramters were all present on the command line */
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

    xi_context_handle_t first_context = XI_INVALID_CONTEXT_HANDLE;

#if CTX_2ND == 1
    xi_context_handle_t second_context = XI_INVALID_CONTEXT_HANDLE;
#endif

    /* initialize xi library and create a context to use to connect to the Xively Service
     * Device-id must be the same as username
     */
    const xi_state_t error_init = xi_initialize( xi_account_id );

    if ( XI_STATE_OK != error_init )
    {
        printf( " xi failed to initialize, error: %d\n", error_init );
        return -1;
    }

    first_context = xi_create_context();
    if ( XI_INVALID_CONTEXT_HANDLE >= first_context )
    {
        printf( " xi failed to create first context\n" );
        return -1;
    }

#if CTX_2ND == 1
    second_context = xi_create_context();
    if ( XI_INVALID_CONTEXT_HANDLE >= second_context )
    {
        printf( "xi failed to create second context\n" );
        return -1;
    }
#endif

    const uint16_t connection_timeout = 10;
    const uint16_t keepalive_timeout  = 20;

    shutdown_topic = local_shutdown_topic;

    xi_connect( first_context, xi_username, xi_password, connection_timeout,
                keepalive_timeout, XI_SESSION_CLEAN, &on_connected );

#if CTX_2ND == 1
    xi_connect( second_context, xi_username, xi_password, connection_timeout,
                keepalive_timeout, XI_SESSION_CLEAN, &on_connected2nd );
#endif


    xi_events_process_blocking();

    xi_delete_context( first_context );

#if CTX_2ND == 1
    xi_delete_context( second_context );
#endif

    xi_shutdown();

    return 0;
}
