/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "../../common/src/commandline.h"
#include <xively.h>

#include <stdio.h>
#include <pthread.h>
#include <string.h>

#include <xi_bsp_io_fs.h>
#include <xi_bsp_fwu.h>

static xi_context_handle_t xi_context;

void on_connection_state_changed( xi_context_handle_t in_context_handle,
                                  void* data,
                                  xi_state_t state );

typedef struct download_thread_context_s
{
    pthread_t thread;

    const char* url;
    const char* filename;

    uint8_t* checksum;
    uint16_t checksum_len;

    xi_sft_on_file_downloaded_callback_t* fn_on_file_downloaded;
    void* callback_data;

    uint8_t file_downloaded_POLL_flag;
    uint8_t file_downloaded_successfully_flag;

} download_thread_context_t;

static uint8_t _local_checksum_validation( download_thread_context_t* download_context );

/*
 * Thread main function doing the actual HTTP file download. The `curl` command is
 * used to download the file. After download the checksum validation is also done,
 * storing the result in the `download_context`.
 */
void* download_thread_run( void* ctx )
{
    download_thread_context_t* download_context = ( download_thread_context_t* )ctx;

    if ( NULL == download_context )
    {
        return NULL;
    }

    /* execute a curl command in the console to download the content of the URL
       under the proper fileanme */
    char system_command[512] = {0};
    sprintf( system_command, "curl -L -o %s %s", download_context->filename,
             download_context->url );

    printf( "[ APPLICATION ] executing command:\n%s\n", system_command );

    const int system_command_result = system( system_command );

    printf( "[ APPLICATION ] command returned: %d\n", system_command_result );

    /* local checksum validation, this result will be reported back to Xively C Client
       as the download result */
    download_context->file_downloaded_successfully_flag =
        _local_checksum_validation( ctx );

    /* notify the polling task about the download finish */
    download_context->file_downloaded_POLL_flag = 1;

    return NULL;
}

/*
 * This function polls the status of the download. Able to do this with the
 * `download_context` struct received in the `user_data` argument.
 *
 * If file download has not yet finished, then does nothing since it is scheduled as a
 * `repeat forever` task.
 *
 * If file download has finished the task is canceled and the file download result is
 * reported back to the Xively C Client by calling the function pointer received at
 * the URL download request function: `url_handler_callback`.
 */
void poll_if_download_finished( const xi_context_handle_t context_handle,
                                const xi_timed_task_handle_t timed_task_handle,
                                void* user_data )
{
    ( void )context_handle;

    download_thread_context_t* download_context = ( download_thread_context_t* )user_data;

    /* test the inter-thread :) communication byte if download finished */
    if ( download_context->file_downloaded_POLL_flag )
    {
        printf( "[ APPLICATION ] file downloaded\n" );

        xi_cancel_timed_task( timed_task_handle );

        /* report the file download result to the Xively C Client */
        ( *download_context->fn_on_file_downloaded )(
            download_context->callback_data, download_context->filename,
            download_context->file_downloaded_successfully_flag );
    }
    else
    {
        printf( "[ APPLICATION ] file not downloaded yet\n" );
    }
}

static download_thread_context_t download_thread_context;

/*
 * This function - if set through `xi_set_updateable_files` - gets called
 * with each file in an update package. For details and doxygen docs please
 * visit the function's type declaration in `xively_types.h`.
 *
 * This functions spawns a POSIX thread which starts the download. Does this to avoid
 * blocking this function. A task is also scheduled to poll the download status on the
 * Xively C Client's main thread. Then the function returns 1 indicating a successful
 * download start.
 *
 * There stands return 0 - indicating a failed download start - in case of any error.
 * If 0 is returned and the `flag_mqtt_download_available` is `1` the Xively C Client
 * will fallback to MQTT file download.
 */
uint8_t url_handler_callback( const char* url,
                              const char* filename,
                              uint8_t* checksum,
                              uint16_t checksum_len,
                              uint8_t flag_mqtt_download_available,
                              xi_sft_on_file_downloaded_callback_t* fn_on_file_downloaded,
                              void* callback_data )
{
    printf( "[ APPLICATION ] - %s, %s, %s, checksum: [%s]:%d, %d fnptr: %p\n",
            __FUNCTION__, url, filename, checksum, checksum_len,
            flag_mqtt_download_available, fn_on_file_downloaded );

    /* existence of function parameters is assured by the Xively C Client at least
     * until fn_on_file_downloaded gets called back */
    download_thread_context.url                       = url;
    download_thread_context.filename                  = filename;
    download_thread_context.checksum                  = checksum;
    download_thread_context.checksum_len              = checksum_len;
    download_thread_context.fn_on_file_downloaded     = fn_on_file_downloaded;
    download_thread_context.callback_data             = callback_data;
    download_thread_context.file_downloaded_POLL_flag = 0;

    /* spawn and run the file downloader thread */
    const int ret_pthread_create =
        pthread_create( &download_thread_context.thread, NULL, download_thread_run,
                        &download_thread_context );

    if ( ret_pthread_create != 0 )
    {
        printf( "creation of pthread instance failed with error: %d",
                ret_pthread_create );

        /* return a fallback request to Client's internal MQTT download.
           Fallback will happen if `flag_mqtt_download_available` is 1 */
        return 0;
    }

    /* create a polling task which checks in every second if file download finished */
    const xi_time_t seconds_from_now = 1;
    const xi_time_t repeats_forever  = 1;
    xi_schedule_timed_task( xi_context, poll_if_download_finished, seconds_from_now,
                            repeats_forever, &download_thread_context );

    /* return successful start of HTTP download, no MQTT fallback will take place */
    return 1;
}

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

    /* Setting the update package file list. This list must be set before `xi_connect`
       is called. These files and their revisions will be reported to SFT service and
       kept up-to-date.
       This example fills in the customer URL handler function parameter which will
       result in the function to be called with each file in the update package sent
       down by the SFT service. Please see the function `url_handler_callback`
       documentation and implementation above. */
    xi_set_updateable_files( xi_context, ( const char* [] ){"file.cfg", "firmware.bin"},
                             2, url_handler_callback );

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

    /*  Cleanup the default context, releasing its memory */
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

/*
 * Calculates and validates the checksum. Reads up the just downloaded file from disc,
 * generates the checksum implemented in the BSP FWU module (SHA256 by default) and
 * compares this locally computed checksum against the received one. Writes out
 * message to stdout and returns accordingly.
 */
uint8_t _local_checksum_validation( download_thread_context_t* download_context )
{
    if ( NULL == download_context )
    {
        return 0;
    }

    xi_bsp_io_fs_resource_handle_t resource_handle = XI_BSP_IO_FS_OPEN_READ;
    xi_bsp_io_fs_open( download_context->filename, 0 /* not used */,
                       XI_BSP_IO_FS_OPEN_READ, &resource_handle );

    void* checksum_context = NULL;
    xi_bsp_fwu_checksum_init( &checksum_context );

    size_t read_offset         = 0;
    const uint8_t* read_buffer = NULL;
    size_t read_buffer_size    = 0;

    while ( XI_BSP_IO_FS_STATE_OK == xi_bsp_io_fs_read( resource_handle, read_offset,
                                                        &read_buffer,
                                                        &read_buffer_size ) )
    {
        /* printf( "[ APPLICATION ] read_offset: %lu, read_buffer_size: %lu\n",
           read_offset,
                read_buffer_size ); */

        xi_bsp_fwu_checksum_update( checksum_context, read_buffer, read_buffer_size );
        read_offset += read_buffer_size;

        read_buffer = NULL;
    }

    uint8_t* local_checksum_buffer     = NULL;
    uint16_t local_checksum_buffer_len = 0;
    xi_bsp_fwu_checksum_final( &checksum_context, &local_checksum_buffer,
                               &local_checksum_buffer_len );

    xi_bsp_io_fs_close( resource_handle );

    printf( "[ APPLICATION ] checksum lengths: local: %d, remote: %d\n",
            local_checksum_buffer_len, download_context->checksum_len );

    const int checksum_memcmp_result = memcmp(
        local_checksum_buffer, download_context->checksum, local_checksum_buffer_len );

    printf( "[ APPLICATION ] checksums %s.\n",
            ( 0 == checksum_memcmp_result ) ? "MATCH" : "MISMATCH" );


    return ( 0 == checksum_memcmp_result ) ? 1 : 0;
}
