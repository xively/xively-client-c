/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <wm_os.h>
#include <app_framework.h>
#include <wmtime.h>
#include <cli.h>
#include <wmstdio.h>
#include <board.h>
#include <wmtime.h>
#include <psm.h>
#include <led_indicator.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include <stdint.h>

// from libxively-v3
#include <xively.h>
#include <xi_helpers.h>
#include <xi_mqtt_message.h>
#include <xi_globals.h>
#include <xi_platform_event_loop.h>
#include <xi_memory_limiter.h>

/* Thread handle */
static os_thread_t app_thread;

/* Buffer to be used as stack */
static os_thread_stack_define( app_stack, 64 * 1024 );

static const char username[]   = "username";
static const char password[]   = "password";
static const char test_topic[] = "topic";

/* delayed publish event */
static xi_heap_element_t* delayed_publish_event = 0;

/* This function is defined for handling critical error.
 * For this application, we just stall and do nothing when
 * a critical error occurs.
 *
 */
void appln_critical_error_handler( void* data )
{
    while ( 1 )
        ;
    /* do nothing -- stall */
}

xi_state_t on_test_message( void* context, void* data, xi_state_t state )
{
    XI_UNUSED( context );

    switch ( state )
    {
        case XI_MQTT_SUBSCRIPTION_FAILED:
            wmprintf( "Subscription failed.\n" );
            return XI_STATE_OK;
        case XI_MQTT_SUBSCRIPTION_SUCCESSFULL:
        {
            xi_mqtt_suback_status_t status = ( xi_mqtt_suback_status_t )( intptr_t )data;
            wmprintf( "Subscription successfull with QoS %d\n", status );
            XI_UNUSED( status );
            return XI_STATE_OK;
        }
        case XI_STATE_OK:
        {
            xi_mqtt_message_t* msg = ( xi_mqtt_message_t* )data;
            wmprintf( "received message: size: [%d]\n", msg->publish.content->length );

            if ( msg->publish.content )
            {
                int i = 0;
                for ( ; i < msg->publish.content->length; ++i )
                {
                    wmstdio_putchar( ( char* )&msg->publish.content->data_ptr[i] );
                }
            }

            wmprintf( "\n" );

            wmprintf( "allocated space: [%d]\n",
                      xi_memory_limiter_get_allocated_space() );

            xi_mqtt_message_free( &msg );

            return XI_STATE_OK;
        }

        default:
            return state;
    }
}

xi_state_t delayed_publish( void* in_context )
{
    delayed_publish_event = 0;

    // sending the connect request
    char msg[64] = {'\0'};

    int i = 0;
    for ( ; i < sizeof( msg ) - 1; ++i )
    {
        msg[i] = ( rand() % 64 ) + 32;
    }

    msg[sizeof( msg ) - 1] = '\0';

    xi_publish_to_topic( test_topic, msg, XI_MQTT_QOS_AT_LEAST_ONCE );

    { // register delayed publish again
        delayed_publish_event = xi_evtd_execute_in(
            xi_globals.evtd_instance, xi_make_handle( &delayed_publish, in_context ), 1 );
    }

    return XI_STATE_OK;
}

xi_state_t on_connected( void* in_context, void* data, xi_state_t state )
{
    xi_connection_data_t* conn_data      = ( xi_connection_data_t* )data;
    xi_evtd_instance_t* event_dispatcher = xi_globals.evtd_instance;

    wmprintf( "on_connected\n" );

    switch ( conn_data->connection_state )
    {
        case XI_CONNECTION_STATE_OPEN_FAILED:
            wmprintf( "connection to %s:%d has failed reason %d\n", conn_data->host,
                      conn_data->port, state );

            xi_connect_with_callback(
                conn_data->username, conn_data->password, conn_data->connection_timeout,
                conn_data->keepalive_timeout, conn_data->session_type, &on_connected );

            return XI_STATE_OK;
        case XI_CONNECTION_STATE_OPENED:
            wmprintf( "connected to %s:%d\n", conn_data->host, conn_data->port );
            break;
        case XI_CONNECTION_STATE_CLOSED:
            wmprintf( "connection closed - reason %d!\n", state );

            if ( delayed_publish_event != NULL )
            {
                delayed_publish_event =
                    xi_evtd_cancel( event_dispatcher, delayed_publish_event );
            }

            if ( state == XI_STATE_OK )
            {
                xi_evtd_stop( event_dispatcher );
            }
            else
            {
                xi_connect_with_callback( conn_data->username, conn_data->password,
                                          conn_data->connection_timeout,
                                          conn_data->keepalive_timeout,
                                          conn_data->session_type, &on_connected );
            }

            return XI_STATE_OK;
        default:
            wmprintf( "invalid parameter %d\n", conn_data->connection_state );
            return XI_INVALID_PARAMETER;
    }

    // register delayed publish
    delayed_publish_event = xi_evtd_execute_in(
        event_dispatcher, xi_make_handle( &delayed_publish, in_context ), 3 );

    xi_subscribe_to_topic( test_topic, XI_MQTT_QOS_AT_LEAST_ONCE, &on_test_message );

    return XI_STATE_OK;
}

/**
 * @brief xively_test_main
 * @param data
 *
 * Main thread of the xively test application
 */
static void xively_test_main( os_thread_arg_t data )
{
    wmprintf( "\n --- Xively Test Application Started---\n" );

    // initialize xi library.
    if ( XI_STATE_OK != xi_create_default_context() )
    {
        wmprintf( " xi failed initialization\n" );
        appln_critical_error_handler( ( void* )-WM_FAIL );
    }
    else
    {
        wmprintf( "xi initialised!\n" );
    }

    xi_connect_with_callback( username, password, 20, 20, XI_SESSION_CLEAN,
                              &on_connected );

    xi_platform_event_loop();

    wmprintf( "done!\n" );

    xi_delete_default_context();

    xi_shutdown();
}

/* This is the main event handler for this project. The application framework
 * calls this function in response to the various events in the system.
 */
int common_event_handler( int event, void* data )
{
    int ret;
    static bool is_cloud_started;

    switch ( event )
    {
        case AF_EVT_WLAN_INIT_DONE:
            ret = psm_cli_init();
            if ( ret != WM_SUCCESS )
                wmprintf( "Error: psm_cli_init failed\n" );
            int i = ( int )data;

            if ( i == APP_NETWORK_NOT_PROVISIONED )
            {
                wmprintf( "\nPlease provision the device "
                          "and then reboot it:\n\n" );
                wmprintf( "psm-set network ssid <ssid>\n" );
                wmprintf( "psm-set network security <security_type>"
                          "\n" );
                wmprintf( "    where: security_type: 0 if open,"
                          " 3 if wpa, 4 if wpa2\n" );
                wmprintf( "psm-set network passphrase <passphrase>"
                          " [valid only for WPA and WPA2 security]\n" );
                wmprintf( "psm-set network configured 1\n" );
                wmprintf( "pm-reboot\n\n" );
            }
            else
                app_sta_start();

            break;
        case AF_EVT_NORMAL_CONNECTED:
            if ( !is_cloud_started )
            {
                ret = os_thread_create( &app_thread,          /* thread handle */
                                        "xively_demo_thread", /* thread name */
                                        xively_test_main,     /* entry function */
                                        0,                    /* argument */
                                        &app_stack,           /* stack */
                                        OS_PRIO_2 );          /* priority - medium low */
                is_cloud_started = true;
            }
            break;
        default:
            break;
    }

    return 0;
}

static void modules_init()
{
    int ret;

    ret = wmstdio_init( UART0_ID, 0 );
    if ( ret != WM_SUCCESS )
    {
        wmprintf( "Error: wmstdio_init failed\n" );
        appln_critical_error_handler( ( void* )-WM_FAIL );
    }

    ret = cli_init();
    if ( ret != WM_SUCCESS )
    {
        wmprintf( "Error: cli_init failed\n" );
        appln_critical_error_handler( ( void* )-WM_FAIL );
    }

    ret = pm_cli_init();
    if ( ret != WM_SUCCESS )
    {
        wmprintf( "Error: pm_cli_init failed\n" );
        appln_critical_error_handler( ( void* )-WM_FAIL );
    }
    /* Initialize time subsystem.
     *
     * Initializes time to 1/1/1970 epoch 0.
     */
    ret = wmtime_init();
    if ( ret != WM_SUCCESS )
    {
        wmprintf( "Error: wmtime_init failed\n" );
        appln_critical_error_handler( ( void* )-WM_FAIL );
    }
    return;
}

int main()
{
    modules_init();

    wmprintf( "Build Time: " __DATE__ " " __TIME__ "\n" );

    /* Start the application framework */
    if ( app_framework_start( common_event_handler ) != WM_SUCCESS )
    {
        wmprintf( "Failed to start application framework\n" );
        appln_critical_error_handler( ( void* )-WM_FAIL );
    }
    return 0;
}
