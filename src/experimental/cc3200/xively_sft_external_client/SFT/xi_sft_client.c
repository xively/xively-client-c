/*  Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
    This is part of Xively C library. */

/*
 * HEADERS
 */

/* SFT Client Includes */
#include <xively.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "src/cbor.h"
#include "include/cn-cbor/cn-cbor.h"
#include "xi_bsp_fwu.h"
#include "xi_sft.h"

/* Simpelink EXT lib includes */
#include "flc_api.h"

/* Platform includes */
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_types.h"
#include "hw_ints.h"
#include "interrupt.h"
#include "rom.h"
#include "rom_map.h"
#include "uart.h"
#include "prcm.h"
#include "utils.h"
#include "gpio.h"
#include "gpio_if.h"
#include "uart_if.h"

/*
 * DEFINES
 */
#define UART_PRINT Report
#undef printf
#define printf Report

#define MAX_INCOMING_FILENAME_LENGTH 50
#define MAX_STRING_LENGTH 50
#define MESSAGESIZE 50
#define LOGDETAILSSIZE 400
#define XIVELY_TOPIC_LEN 128

/*
 * TYPEDEFS
 */
typedef unsigned char uchar;
typedef unsigned int uint;

typedef enum {
    sft_status_ok = 0,

    sft_status_rebooting = 1,

    sft_open_file_error  = 2,
    sft_close_file_error = 3,
    sft_write_error      = 4,

    sft_unsupported_protocol_version = 5,
    sft_unexpected_filename_error    = 6,
    sft_empty_length_array_error     = 7,
    sft_data_out_of_bounds_error     = 8,
    sft_file_data_offset_error       = 9,

    sft_cbor_missing_field_error   = 10,
    sft_cbor_encoding_error        = 11,
    sft_cbor_error_field_set_error = 12,

    sft_publish_error               = 13,
    sft_internal_error              = 14,
    sft_fingerprint_mismatch_error  = 15,
    sft_fingerprint_too_large_error = 16,

    sft_status_max

} sft_status_t;

/* stores state data for the download state machine */
typedef struct file_download_ctx_s
{
    char is_downloading;
    int file_length;
    int file_storage_offset;
    sft_status_t result;

    long file_handle;
    unsigned long file_token;

    char file_name[MAX_STRING_LENGTH];
    char file_revision[MAX_STRING_LENGTH];
    unsigned char file_sft_fingerprint_str[65];
    unsigned char* file_local_computed_fingerprint;
    uint16_t file_local_computed_fingerprint_len;

    void* checksum_context;

} file_download_ctx_t;

/* data is extracted from FILE_CHUNK cbor messages and
 * temporarily stored in this more accessible structure */
typedef struct xi_sft_file_chunk_s
{
    uint offset;
    uint chunk_length;
    uint array_length;
    uint status;
    unsigned char* byte_array;
} xi_sft_file_chunk_t;


/*
 * VARIABLES
 */
int tenpercent        = 0;
int currenttenpercent = 0;

char firmware_filename[] = "firmware";
char firmware_revision[] = "1.0";

file_download_ctx_t download_ctx;

#ifdef USE_CBOR_CONTEXT
extern cn_cbor_context* context_cbor;
#endif

/* Buffers to hold the formatted topics.
 * Topics are mangled by account and device ids. */
char xi_stopic[XIVELY_TOPIC_LEN];
char xi_logtopic[XIVELY_TOPIC_LEN];
char xi_ctopic[XIVELY_TOPIC_LEN];

/* strings that correspond to the xi_status_t enumeration.
 * used for publishing messages to the Xivel Device Logs service. */
const char* sft_status_strings[] = {"sft_status_ok",        /* 0 */
                                    "sft_status_rebooting", /* 1 */

                                    "sft_open_file_error",  /* 2 */
                                    "sft_close_file_error", /* 3 */
                                    "sft_write_error",      /* 4 */

                                    "sft_unsupported_protocol_version", /* 5 */
                                    "sft_unexpected_filename_error",    /* 6 */
                                    "sft_empty_length_array_error",     /* 7 */
                                    "sft_data_out_of_bounds_error",     /* 8 */
                                    "sft_file_data_offset_error",       /* 9 */

                                    "sft_cbor_missing_field_error",   /* 10 */
                                    "sft_cbor_encoding_error",        /* 11 */
                                    "sft_cbor_error_field_set_error", /* 12 */

                                    "sft_publish_error",               /* 13 */
                                    "sft_internal_error",              /* 14 */
                                    "sft_fingerprint_mismatch_error",  /* 15 */
                                    "sft_fingerprint_too_large_error", /* 16 */

                                    "sft_status_max"};


/*
 *  Function Declarations
 */
int xi_sft_init( xi_context_handle_t in_context_handle,
                 char* account_id,
                 char* device_id );
void on_sft_message( xi_context_handle_t in_context_handle,
                     xi_sub_call_type_t call_type,
                     const xi_sub_call_params_t* const params,
                     xi_state_t state,
                     void* user_data );

/* incoming message handling functions */
void xi_process_file_chunk( xi_context_handle_t in_context_handle, cn_cbor* in_cb );
void xi_parse_file_update_available( xi_context_handle_t in_context_handle, cn_cbor* cb );
void xi_parse_file_chunk( xi_context_handle_t in_context_handle,
                          cn_cbor* in_cb,
                          xi_sft_file_chunk_t* out_sft_file_chunk );

/* outgoing message formating and publication functions */
void xi_publish_file_info( xi_context_handle_t in_context_handle );
void xi_publish_file_chunk_request( xi_context_handle_t in_context_handle,
                                    const char* file_name,
                                    const char* file_revision,
                                    int offset );

/* Error Handling Functions */
void xi_set_sft_error( sft_status_t status );
void xi_missing_cbor_field_error( xi_context_handle_t in_context_handle,
                                  const char* cbor_message_name,
                                  const char* field_name );

/* Device Log Functions */
void xi_publish_device_log( xi_context_handle_t in_context_handle,
                            const char* message_str,
                            const char* details_str,
                            const sft_status_t status );
void xi_publish_device_log_format_int( xi_context_handle_t in_context_handle,
                                       const char* message_str,
                                       const char* details_str,
                                       const int value_int,
                                       const sft_status_t status );
void xi_publish_device_log_format_int_int( xi_context_handle_t in_context_handle,
                                           const char* message_str,
                                           const char* details_str,
                                           const int value_int1,
                                           const int value_int2,
                                           const sft_status_t status );
void xi_publish_device_log_format_str( xi_context_handle_t in_context_handle,
                                       const char* message_str,
                                       const char* details_str,
                                       const char* value_str,
                                       const sft_status_t status );
void xi_publish_device_log_format_str_str( xi_context_handle_t in_context_handle,
                                           const char* message_str,
                                           const char* details_str,
                                           const char* value_str1,
                                           const char* value_str2,
                                           const sft_status_t status );

/* Utility Functions */
void print_hash( unsigned char hash[] );
void verify_sha256( xi_context_handle_t in_context_handle );


/*
 *  Secure File Transfer Client Source Code
 */


/* @brief Secure File Transfer client initialization.
 *
 * Formats topics strings to be used for SFT, subscribes to the service and checks for any
 * pending
 * commit from the previous boot.
 */
int xi_sft_init( xi_context_handle_t in_context_handle,
                 char* account_id,
                 char* device_id )
{
    if ( NULL == account_id || NULL == device_id )
    {
        return -1;
    }

    /* xi_stopic is the MQTT topic we PUB to to send messages to the broker. */
    snprintf( xi_stopic, sizeof( xi_stopic ), "xi/ctrl/v1/%s/svc", device_id );

    /* xi_ctopic is the MQTT topic we SUBSCRIBE to to receive messages from the broker. */
    snprintf( xi_ctopic, sizeof( xi_ctopic ), "xi/ctrl/v1/%s/cln", device_id );

    /* xi_logtopic is the MQTT topic we PUB to to add log messages. */
    snprintf( xi_logtopic, sizeof( xi_logtopic ), "xi/blue/v1/%s/d/%s/_log", account_id,
              device_id );

    /* Accept the latest firmware if we get here. */
    if ( sl_extlib_FlcIsPendingCommit() )
    {
        printf( "\n********** Committing this revision:\"%s\" **********\n",
                firmware_revision );
        sl_extlib_FlcCommit( FLC_COMMITED );
        xi_publish_device_log( in_context_handle, "Firmware Committed",
                               "Boot loader will now boot to new image from now on",
                               sft_status_ok );
    }

    /* Subscribe to SFT topic.  Everything interesting that happens will occur when an
     * incoming message
     * arrives, handled by a state machine in on_sft_message(). */
    void* user_data = NULL;
    xi_subscribe( in_context_handle, xi_ctopic, XI_MQTT_QOS_AT_MOST_ONCE, on_sft_message,
                  user_data );

    return 0;
}


/* @brief Reboot the device
 * This is invoked through a timed task from the xively client after the
 * file download is complete
 */
static void reboot_mcu( const xi_context_handle_t in_context_handle,
                        const xi_timed_task_handle_t timed_task_handle,
                        void* user_data )
{
    /* Configure hibernate RTC wakeup */
    PRCMHibernateWakeupSourceEnable( PRCM_HIB_SLOW_CLK_CTR );

    /* Delay loop */
    MAP_UtilsDelay( 8000000 );

    /* Set wake up time */
    PRCMHibernateIntervalSet( 330 );

    /* Request hibernate */
    PRCMHibernateEnter();

    /* Control should never reach here */
    while ( 1 )
    {
        ;
    }
}

/**
 * @brief Function that handles messages sent from the Xively Secure File Transfer
 * Service.
 *
 * @note For more details please refer to the xively.h - xi_subscribe function.
 *
 * @param in_context_handle - xively library context required for calling API functions
 * @param call_type - describes if the handler was called carrying the subscription
 * operation result or the incoming message
 * @oaram params - structure with data
 * @param state - state of the operation
 * @oaram user_data - optional data, in this case nothing.
 */
void on_sft_message( xi_context_handle_t in_context_handle,
                     xi_sub_call_type_t call_type,
                     const xi_sub_call_params_t* const params,
                     xi_state_t state,
                     void* user_data )
{
    cn_cbor* cb                 = NULL;
    cn_cbor* cb_item            = NULL;
    const uint8_t* payload_data = NULL;
    size_t payload_length       = 0;

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
            /* get the payload parameter that contains the data that was sent. */
            payload_data   = params->message.temporary_payload_data;
            payload_length = params->message.temporary_payload_data_length;

            /* Figure out what the packet type is and then call the appropriate parser */
            cb = cn_cbor_decode( payload_data, payload_length CBOR_CONTEXT_PARAM, 0 );

            if ( cb )
            {
                cb_item = cn_cbor_mapget_string( cb, "msgtype" );
                if ( cb_item )
                {
                    switch ( cb_item->v.uint )
                    {
                        case XI_FILE_UPDATE_AVAILABLE:
                            if ( download_ctx.is_downloading )
                            {
                                printf( "WARNING: ALREADY DOWNLOADING!\n" );
                                cn_cbor_free( cb CBOR_CONTEXT_PARAM );
                                break;
                            }

                            /* Parse FILE_UPDATE_AVAILABLE messsage
                             * and format GET_FILE_CHUNK message into the outgoing cbor
                             * buffer `encoded` */
                            xi_parse_file_update_available( in_context_handle, cb );

                            /* Free the incoming cbor message context */
                            cn_cbor_free( cb CBOR_CONTEXT_PARAM );

                            if ( sft_status_ok == download_ctx.result )
                            {
                                /* Open the file for writing */
                                int open_file_result = sl_extlib_FlcOpenFile(
                                    "/sys/mcuimgA.bin", download_ctx.file_length + 1024,
                                    &download_ctx.file_token, &download_ctx.file_handle,
                                    FS_MODE_OPEN_WRITE );
                                /* Check for errors */
                                if ( open_file_result < 0 )
                                {
                                    printf( " ERROR: mcuimgA.bin open returned %d\n",
                                            open_file_result );
                                    xi_set_sft_error( sft_open_file_error );
                                    xi_publish_device_log_format_int(
                                        in_context_handle, "File update, open failed",
                                        "FlcOpenFile returned: %d", open_file_result,
                                        sft_open_file_error );
                                }
                            }
                            break;

                        case XI_FILE_CHUNK:
                            /* Received a file chunk message.  Make sure that we're not in
                             * an error state
                             * before processing the message.  */
                            if ( sft_status_ok == download_ctx.result )
                            {
                                /* Parse the FILE_CHUNK message and send another
                                 * FILE_GET_CHUNK request if we're still downloading data
                                 */
                                xi_process_file_chunk( in_context_handle, cb );
                            }
                            else
                            {
                                /* we have an error. set download to false.
                                 * Note, it might already be false.  This is just for
                                 * safety. */
                                download_ctx.is_downloading = 0;
                            }

                            /* Free the incoming cbor message context */
                            cn_cbor_free( cb CBOR_CONTEXT_PARAM );

                            /* Since we're getting file chunks then we were in a
                             * downloading state.
                             * If we're not currently downloading, that means the download
                             * finished
                             * or we've encountered an error. */
                            if ( !download_ctx.is_downloading )
                            {
                                if ( sft_status_ok == download_ctx.result )
                                {
                                    verify_sha256( in_context_handle );
                                }

                                /* Close the firmware file */
                                const int close_files_result = sl_extlib_FlcCloseFile(
                                    download_ctx.file_handle, NULL, NULL, 0 );
                                if ( 0 != close_files_result )
                                {
                                    printf( "ERROR: firmware file close via FlcCloseFile "
                                            "result: %d\n",
                                            close_files_result );
                                    xi_set_sft_error( sft_close_file_error );
                                    xi_publish_device_log_format_int(
                                        in_context_handle, "File update, close failed",
                                        "FlcCloseFile returned: %d", close_files_result,
                                        sft_open_file_error );
                                }

                                /* Log the download complete information */
                                char log_details[LOGDETAILSSIZE];
                                snprintf( log_details, sizeof( log_details ),
                                          "filename=%s revision=%s length = %d",
                                          firmware_filename, download_ctx.file_revision,
                                          download_ctx.file_length );
                                xi_publish_device_log( in_context_handle,
                                                       "Firmware Update Finished.",
                                                       log_details, download_ctx.result );

                                printf( "=================\n" );
                                printf( "Done downloading.\n" );
                                printf( "   result: %d\n", download_ctx.result );
                                printf( "=================\n" );

                                /* Set the bootloader to test the new image */
                                if ( download_ctx.result )
                                {
                                    printf( "NOT MARKING FILE TO TEST DUE TO ERRORS\n" );
                                }
                                else
                                {
                                    xi_publish_device_log(
                                        in_context_handle,
                                        "Rebooting Device to Test Firmware.",
                                        "Device will reboot in 3 seconds",
                                        sft_status_ok );
                                    sl_extlib_FlcTest( FLC_TEST_RESET_MCU |
                                                       FLC_TEST_RESET_MCU_WITH_APP );

                                    /* Schedule a callback to reboot the device to test
                                     * the image
                                     * We don't do this immediately because we want to
                                     * give enough time
                                     * for our device log publications to send. */
                                    xi_schedule_timed_task( in_context_handle, reboot_mcu,
                                                            3, 0, NULL );
                                }
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
            return;
        default:
            return;
    }
}

/* @brief invoked to have the device report its current firmware version
 * to the Secure File Transfer service.  If a different firwmare is available
 * then the service will respond with a FILE_UPDATE_AVAILABLE message. */
void xi_publish_file_info( xi_context_handle_t in_context_handle )
{
    /* Let's try making a XI_FILE_INFO packet */
    cn_cbor_errback err;
    cn_cbor* cb_map = cn_cbor_map_create( CBOR_CONTEXT_PARAM_COMMA & err );

    cn_cbor_map_put( cb_map, cn_cbor_string_create( "msgtype" CBOR_CONTEXT_PARAM, &err ),
                     cn_cbor_int_create( XI_FILE_INFO CBOR_CONTEXT_PARAM, &err ), &err );

    cn_cbor_map_put( cb_map, cn_cbor_string_create( "msgver" CBOR_CONTEXT_PARAM, &err ),
                     cn_cbor_int_create( 1 CBOR_CONTEXT_PARAM, &err ), &err );

    cn_cbor* a        = cn_cbor_array_create( CBOR_CONTEXT_PARAM_COMMA & err );
    cn_cbor* cb_file1 = cn_cbor_map_create( CBOR_CONTEXT_PARAM_COMMA & err );
    cn_cbor_map_put( cb_file1, cn_cbor_string_create( "N" CBOR_CONTEXT_PARAM, &err ),
                     cn_cbor_string_create( firmware_filename CBOR_CONTEXT_PARAM, &err ),
                     &err );

    cn_cbor_map_put( cb_file1, cn_cbor_string_create( "R" CBOR_CONTEXT_PARAM, &err ),
                     cn_cbor_string_create( "-1" CBOR_CONTEXT_PARAM, &err ), &err );

    cn_cbor_array_append( a, cb_file1, &err );
    cn_cbor_map_put( cb_map, cn_cbor_string_create( "list" CBOR_CONTEXT_PARAM, &err ), a,
                     &err );

    unsigned char encoded[128];
    const ssize_t enc_sz = cn_cbor_encoder_write( encoded, 0, sizeof( encoded ), cb_map );
    cn_cbor_free( cb_map CBOR_CONTEXT_PARAM );

    if ( 0 >= enc_sz )
    {
        xi_set_sft_error( sft_cbor_encoding_error );
        xi_publish_device_log_format_int( in_context_handle, "FILE_INFO Encoding Error",
                                          "encoding FILE_INFO cbor error: %d", enc_sz,
                                          sft_cbor_encoding_error );
        return;
    }

    /* publish the FILE_INFO message */
    xi_publish_data( in_context_handle, xi_stopic, encoded, enc_sz,
                     XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
    printf( "Published FILE_INFO to %s\n", xi_stopic );
}


void xi_parse_file_update_available( xi_context_handle_t in_context_handle, cn_cbor* cb )
{
    cn_cbor* cb_list;
    cn_cbor* cb_item;
    cn_cbor* cb_file;
    int filelistlength = 0;
    int index;

    /* Zero out download_ctx */
    memset( &download_ctx, 0, sizeof( file_download_ctx_t ) );

    /* Initialize sha256 digest of the file and tracking variables */
    xi_bsp_fwu_checksum_init( &download_ctx.checksum_context );

    /* check message version */
    cb_item = cn_cbor_mapget_string( cb, "msgver" );
    if ( cb_item )
    {
        if ( 1 != cb_item->v.uint )
        {
            xi_set_sft_error( sft_unsupported_protocol_version );
            xi_publish_device_log_format_int(
                in_context_handle, "File update available error",
                "FILE_UPDATE_AVAILABLE SFT message version: %d is unsupported",
                cb_item->v.uint, sft_unsupported_protocol_version );
            printf( "ERROR: Unsupported FILE_UPDATE_AVIALABLE message version: %d\n",
                    cb_item->v.uint );
            return;
        }
    }

    /* Get the File List.  For this demo it should be only one file */
    cb_list = cn_cbor_mapget_string( cb, "list" );
    if ( cb_list )
    {
        filelistlength = cb_list->length;
    }

    /* iterate over the file list */
    for ( index = 0; index < filelistlength; index++ )
    {
        cb_item = cn_cbor_index( cb_list, index );

        if ( cb_item )
        {
            /* File 'N'ame */
            cb_file = cn_cbor_mapget_string( cb_item, "N" );
            if ( cb_file )
            {
                if ( strlen( firmware_filename ) != cb_file->length ||
                     strncmp( firmware_filename, cb_file->v.str, cb_file->length ) != 0 )
                {
                    xi_set_sft_error( sft_unexpected_filename_error );
                    xi_publish_device_log( in_context_handle, "Unexpected Filename Error",
                                           "Unexpected filename, expected 'firmware'",
                                           sft_unexpected_filename_error );
                    printf( "ERROR: incoming file update has an unexpected name. "
                            "Expected `firmware`" );
                    return;
                }
            }
            else
            {
                xi_missing_cbor_field_error( in_context_handle, "FILE_UPDATE_AVIALABLE",
                                             "N" );
                return;
            }

            /* File 'R'evision */
            cb_file = cn_cbor_mapget_string( cb_item, "R" );
            if ( cb_file )
            {
                strncpy( download_ctx.file_revision, cb_file->v.str,
                         sizeof( download_ctx.file_revision ) );

                if ( ( unsigned int )cb_file->length <
                     sizeof( download_ctx.file_revision ) )
                {
                    download_ctx.file_revision[cb_file->length] = '\0';
                }

                download_ctx.file_revision[sizeof( download_ctx.file_revision ) - 1] =
                    '\0';
            }
            else
            {
                xi_missing_cbor_field_error( in_context_handle, "FILE_UPDATE_AVIALABLE",
                                             "R" );
                return;
            }

            /* File 'S'ize */
            cb_file = cn_cbor_mapget_string( cb_item, "S" );
            if ( cb_file )
            {
                printf( "FILE_UPDATE_AVAILABLE: file length = %d\n", cb_file->v.sint );
                download_ctx.file_length = cb_file->v.sint;

                /* initialize progress tracking */
                tenpercent        = download_ctx.file_length / 10;
                currenttenpercent = tenpercent;
            }
            else
            {
                xi_missing_cbor_field_error( in_context_handle, "FILE_UPDATE_AVIALABLE",
                                             "S" );
                return;
            }

            /* File 'F'ingerprint */
            cb_file = cn_cbor_mapget_string( cb_item, "F" );
            if ( cb_file )
            {
                if ( sizeof( download_ctx.file_sft_fingerprint_str ) - 1 <
                     cb_file->length )
                {
                    xi_set_sft_error( sft_fingerprint_too_large_error );
                    xi_publish_device_log(
                        in_context_handle, "File update available error",
                        "Fingerprint size too large.", sft_fingerprint_too_large_error );
                    printf( "ERROR: incoming fingerprint to large for local storage "
                            "buffer." );
                }
                else
                {
                    memset( download_ctx.file_sft_fingerprint_str, '\0',
                            sizeof( download_ctx.file_sft_fingerprint_str ) );
                    strncpy( ( char* )download_ctx.file_sft_fingerprint_str,
                             cb_file->v.str, cb_file->length );
                }
            }
            else
            {
                xi_missing_cbor_field_error( in_context_handle, "FILE_UPDATE_AVIALABLE",
                                             "F" );
                return;
            }
        }
        else
        {
            xi_missing_cbor_field_error( in_context_handle, "FILE_UPDATE_AVIALABLE",
                                         "list" );
            return;
        }
    }

    /* Log the FILE_UPDATE_AVAILABLE information */
    {
        char log_details_str[LOGDETAILSSIZE];
        snprintf( log_details_str, sizeof( log_details_str ),
                  "name=%s revision=%s length = %d", firmware_filename,
                  download_ctx.file_revision, download_ctx.file_length );
        xi_publish_device_log( in_context_handle, "File update available",
                               log_details_str, sft_status_ok );
    }

    /* Set Download State to Active */
    download_ctx.is_downloading = 1;

    /* Send the Chunk Request */
    xi_publish_file_chunk_request( in_context_handle, firmware_filename,
                                   download_ctx.file_revision,
                                   download_ctx.file_storage_offset );
}

/*
 * Formats a FILE_GET_CHUNK cbor message with the provided parameters and then publishes
 * the message to the SFT Service.
 *
 * Returns 0 on success, or -1 if the message could not be queued for publication
 * (probably due to a too-small encoding buffer)
 */
void xi_publish_file_chunk_request( xi_context_handle_t in_context_handle,
                                    const char* file_name,
                                    const char* file_revision,
                                    int offset )
{
    const int message_type    = XI_FILE_GET_CHUNK;
    const int message_version = 1;
    const int chunk_length    = 1024;

#if 0
    printf( "XI_FILE_GET_CHUNK request params: " );
    printf( "N: \"%s\", ", file_name );
    printf( "R: \"%s\", ", file_revision );
    printf( "O: %d, ", offset );
    printf( "L: %d\n", chunk_length );
#endif

    /* Format a CBOR message of the FILE_GET_CHUNK message type. */
    cn_cbor_errback err;
    cn_cbor* cb_map = cn_cbor_map_create( CBOR_CONTEXT_PARAM_COMMA & err );

    cn_cbor_map_put( cb_map, cn_cbor_string_create( "msgtype" CBOR_CONTEXT_PARAM, &err ),
                     cn_cbor_int_create( message_type CBOR_CONTEXT_PARAM, &err ), &err );

    cn_cbor_map_put( cb_map, cn_cbor_string_create( "msgver" CBOR_CONTEXT_PARAM, &err ),
                     cn_cbor_int_create( message_version CBOR_CONTEXT_PARAM, &err ),
                     &err );

    cn_cbor_map_put( cb_map, cn_cbor_string_create( "N" CBOR_CONTEXT_PARAM, &err ),
                     cn_cbor_string_create( file_name CBOR_CONTEXT_PARAM, &err ), &err );

    cn_cbor_map_put( cb_map, cn_cbor_string_create( "R" CBOR_CONTEXT_PARAM, &err ),
                     cn_cbor_string_create( file_revision CBOR_CONTEXT_PARAM, &err ),
                     &err );

    cn_cbor_map_put( cb_map, cn_cbor_string_create( "O" CBOR_CONTEXT_PARAM, &err ),
                     cn_cbor_int_create( offset CBOR_CONTEXT_PARAM, &err ), &err );

    cn_cbor_map_put( cb_map, cn_cbor_string_create( "L" CBOR_CONTEXT_PARAM, &err ),
                     cn_cbor_int_create( chunk_length CBOR_CONTEXT_PARAM, &err ), &err );

    unsigned char encoded[128];
    const ssize_t enc_sz = cn_cbor_encoder_write( encoded, 0, sizeof( encoded ), cb_map );
    cn_cbor_free( cb_map CBOR_CONTEXT_PARAM );

    if ( 0 >= enc_sz )
    {
        xi_set_sft_error( sft_cbor_encoding_error );
        xi_publish_device_log_format_int(
            in_context_handle, "FILE_GET_CHUNK Encoding Error",
            "encoding FILE_GET_CHUNK cbor error: %d", enc_sz, sft_cbor_encoding_error );
        return;
    }

    xi_state_t xi_state =
        xi_publish_data( in_context_handle, xi_stopic, encoded, enc_sz,
                         XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
    if ( XI_STATE_OK != xi_state )
    {
        xi_set_sft_error( sft_publish_error );
        xi_publish_device_log_format_int( in_context_handle, "Publish Error",
                                          "Error publishing GET_FILE_CHUNK request. "
                                          "xi_status code from xively client: %d",
                                          xi_state, sft_publish_error );
    }
}

/* @brief Processes incoming SFT FILE_CHUNK messages, storing the data.
 *
 * Process the cbor object as a FILE_CHUNK message, calling xi_parse_file_chunk to extract
 * the
 * cbor fields and then act upon them to store the data into the flash file system's
 * firmware image file.  If there is more of the file to be downloaded then a subsequent
 * GET_CHUNK request is encoded and published to the SFT service.
 */
void xi_process_file_chunk( xi_context_handle_t in_context_handle, cn_cbor* cb )
{
    xi_sft_file_chunk_t file_chunk_data;
    xi_parse_file_chunk( in_context_handle, cb, &file_chunk_data );
    if ( sft_status_ok != download_ctx.result )
    {
        return;
    }

    /* Ensure that this chunk starts where we left off. */
    if ( download_ctx.file_storage_offset != file_chunk_data.offset )
    {
        printf( "ERROR: unexpected Chunk Offset Delivered by SFT!\n" );
        xi_set_sft_error( sft_file_data_offset_error );
        xi_publish_device_log_format_int_int(
            in_context_handle, "File Chunk Offset Error",
            "Expected offset: %d but recieved offset: %d",
            download_ctx.file_storage_offset, file_chunk_data.offset,
            sft_file_data_offset_error );
        return;
    }

    /* Track Progress */
    if ( currenttenpercent < file_chunk_data.offset )
    {
        printf( "Downloaded %d bytes %d %%  of the file \n",
                download_ctx.file_storage_offset,
                ( 100 * ( currenttenpercent + 1000 ) ) / download_ctx.file_length );
        currenttenpercent += tenpercent;
    }

    /* Ensure that we have a buffer to write from */
    if ( 0 == file_chunk_data.array_length )
    {
        printf( "ERROR: FILE_CHUNK cbor message has zero length byte array\n" );
        xi_set_sft_error( sft_empty_length_array_error );
        xi_publish_device_log(
            in_context_handle, "File Chunk Data Error",
            "FILE_CHUNK message from SFT has a zero length data array.",
            sft_empty_length_array_error );
        return;
    }

    /* convert from unsigned to signed becuase TI's API has a signed length variable */
    if ( file_chunk_data.offset + file_chunk_data.array_length >
         download_ctx.file_length )
    {
        printf( "ERROR: Chunk offset + length extends beyond length of file\n" );
        xi_set_sft_error( sft_file_data_offset_error );
        xi_publish_device_log_format_int_int(
            in_context_handle, "File Chunk Offset Error",
            "Offset: %d length: %d goes beyond bounds of file length.",
            file_chunk_data.offset + file_chunk_data.array_length,
            download_ctx.file_length, sft_file_data_offset_error );
        return;
    }

    int write_length = ( int )file_chunk_data.array_length;
    if ( 0 == write_length )
    {
        printf( "ERROR: FILE_CHUNK zero length data array\n" );
        xi_set_sft_error( sft_empty_length_array_error );
        xi_publish_device_log( in_context_handle, "File Chunk Write Error",
                               "Casting file chunk array length is zero",
                               sft_empty_length_array_error );
        return;
    }

    if ( 0 > write_length )
    {
        /* Negative values means that we lost data in the unsigned -> signed conversion.
         */
        printf( "ERROR: WriteBuffer is too large for CC3200 sl_extlib_FlcWriteFile "
                "operations\n" );
        xi_set_sft_error( sft_write_error );
        xi_publish_device_log(
            in_context_handle, "File Chunk Write Error",
            "WriteBuffer is too large for CC3200 sl_extlib_FlcWriteFile operations",
            sft_write_error );
        return;
    }

    int bytes_written = sl_extlib_FlcWriteFile(
        download_ctx.file_handle, file_chunk_data.offset,
        ( unsigned char* )file_chunk_data.byte_array, write_length );
    if ( 0 >= bytes_written )
    {
        printf( "ERROR: sl_extlib_FlcWriteFile returned error code: %d\n",
                bytes_written );
        xi_set_sft_error( sft_write_error );
        xi_publish_device_log_format_int(
            in_context_handle, "File Write Error",
            "l_extlib_FlcWriteFile returned error code: %d.", bytes_written,
            sft_write_error );
        return;
    }
    else
    {
        /* update the SHA256 digest with the provided data */
        xi_bsp_fwu_checksum_update( download_ctx.checksum_context,
                                    ( unsigned char* )file_chunk_data.byte_array,
                                    bytes_written );

        /* Track our progress through the file */
        download_ctx.file_storage_offset = file_chunk_data.offset + bytes_written;

        /* Check for completeness */
        if ( download_ctx.file_storage_offset >= download_ctx.file_length )
        {
            /* Download Complete */
            download_ctx.is_downloading = 0;
        }
    }

    if ( 1 < file_chunk_data.status )
    {
        printf( "ERROR: SFT packet reported Chunk Status Request Error: %d\n",
                file_chunk_data.status );
        xi_set_sft_error( sft_cbor_error_field_set_error );
        xi_publish_device_log_format_int(
            in_context_handle, "File Chunk Status Error",
            "FILE_CHUNK status CBOR field contains an error status: %d",
            file_chunk_data.status, sft_cbor_error_field_set_error );
    }

    if ( download_ctx.is_downloading )
    {
        /* If still download  continuing, create another FILE_CHUNK request to the SFT
         * service */
        xi_publish_file_chunk_request( in_context_handle, firmware_filename,
                                       download_ctx.file_revision,
                                       download_ctx.file_storage_offset );
    }
}

/* @brief Parses a FILE_CHUNK cbor message into a given data structure.
 *
 * Parses the CBOR message to get the FILE_CHUNK required FILE_CHUNK fields
 * and places the data in the out_sft_file_chunk structure
 * Returns 0 on success, or -1 if there are missing fields or null parameters
 */
void xi_parse_file_chunk( xi_context_handle_t in_context_handle,
                          cn_cbor* in_cb,
                          xi_sft_file_chunk_t* out_sft_file_chunk )
{
    if ( NULL == in_cb || NULL == out_sft_file_chunk )
    {
        xi_set_sft_error( sft_internal_error );
        xi_publish_device_log( in_context_handle, "Internal Error",
                               "xi_parse_file_chunk null pointer parameter",
                               sft_internal_error );
    }

    /* use the RED LED to signal downloading progress */
    static int ledToggle = 0;
    ledToggle            = ( ledToggle + 1 ) % 2;
    if ( ledToggle )
    {
        GPIO_IF_LedOn( MCU_RED_LED_GPIO );
    }
    else
    {
        GPIO_IF_LedOff( MCU_RED_LED_GPIO );
    }

    cn_cbor* cb_item = NULL;
    memset( out_sft_file_chunk, 0, sizeof( xi_sft_file_chunk_t ) );

    /* msg version */
    cb_item = cn_cbor_mapget_string( in_cb, "msgver" );
    if ( cb_item )
    {
        if ( 1 != cb_item->v.uint )
        {
            printf( "ERROR: Unsupported FILE_CHUNK message version: %d\n",
                    cb_item->v.uint );
            xi_set_sft_error( sft_unsupported_protocol_version );
            xi_publish_device_log_format_int(
                in_context_handle, "FILE_CHUNK error",
                "FILE_CHUNK Packet version: %d is unsupported", cb_item->v.uint,
                sft_unsupported_protocol_version );
            return;
        }
    }
    else
    {
        xi_missing_cbor_field_error( in_context_handle, "FILE_CHUNK", "msgver" );
        return;
    }

    /* chunk offset */
    cb_item = cn_cbor_mapget_string( in_cb, "O" );
    if ( cb_item )
    {
        out_sft_file_chunk->offset = cb_item->v.uint;
    }
    else
    {
        xi_missing_cbor_field_error( in_context_handle, "FILE_CHUNK", "O" );
        return;
    }

    /* chunk length */
    cb_item = cn_cbor_mapget_string( in_cb, "L" );
    if ( cb_item )
    {
        out_sft_file_chunk->chunk_length = cb_item->v.uint;
    }
    else
    {
        xi_missing_cbor_field_error( in_context_handle, "FILE_CHUNK", "L" );
        return;
    }

    /* Status */
    cb_item = cn_cbor_mapget_string( in_cb, "S" );
    if ( cb_item )
    {
        out_sft_file_chunk->status = cb_item->v.uint;
    }
    else
    {
        xi_missing_cbor_field_error( in_context_handle, "FILE_CHUNK", "S" );
        return;
    }

    /* Byte Array */
    cb_item = cn_cbor_mapget_string( in_cb, "C" );
    if ( cb_item )
    {
        out_sft_file_chunk->array_length = cb_item->length;
        out_sft_file_chunk->byte_array   = ( unsigned char* )cb_item->v.str;
    }
    else
    {
        xi_missing_cbor_field_error( in_context_handle, "FILE_CHUNK", "C" );
        return;
    }
}


/* @brief verify that the fingerprint matches the download file.
 *
 * Invoked when file download is complete.
 * Converts the locally computed hash into a string
 * and compares it to the SFT hash string 'fingerprint'
 * that we received during the FILE_UPDATE_AVAILABLE message.
 * Reports an error to Device Logs if there's a mis match,
 * and marks the error state.
 */
void verify_sha256( xi_context_handle_t in_context_handle )
{
    /* Finalize SHA256 Digest */
    xi_bsp_fwu_checksum_final( &download_ctx.checksum_context,
                               &download_ctx.file_local_computed_fingerprint,
                               &download_ctx.file_local_computed_fingerprint_len );
    printf( "Calculated hash = 0x" );
    print_hash( download_ctx.file_local_computed_fingerprint );

    char local_fingerprint_str[65];
    char* buff_ptr = local_fingerprint_str;

    /* convert binary to a string matching SFT fingprint format */
    int i;
    for ( i = 0; i < download_ctx.file_local_computed_fingerprint_len; ++i )
    {
        buff_ptr +=
            sprintf( buff_ptr, "%02x", download_ctx.file_local_computed_fingerprint[i] );
    }

    /* null terminate the string */
    *buff_ptr = '\0';

    /* are they the same? If not then raise the red flag */
    if ( 0 != strncmp( local_fingerprint_str,
                       ( char* )download_ctx.file_sft_fingerprint_str, 64 ) )
    {
        printf( "File Update Fingerprint mismatch.  SFT Fingerprint: %s  "
                "device-calculated fingerprint: %s\n",
                download_ctx.file_sft_fingerprint_str, local_fingerprint_str );
        xi_set_sft_error( sft_fingerprint_mismatch_error );
        xi_publish_device_log_format_str_str(
            in_context_handle, "File Update Fingerprint Mismatch",
            "sft fingerprint: %s device-calculated fingerprint: %s ",
            ( char* )download_ctx.file_sft_fingerprint_str, local_fingerprint_str,
            sft_fingerprint_mismatch_error );
        return;
    }
}

/*
 * Error Handling functions
 */

/* @brief note an error in the download process
 *
 * This is a utility function for marking that the download has finished
 * so that the downloading state machine can recognize the error.  An
 * error code is set so that the error can later be identified and sent
 * to the Device Logs service.
 */
void xi_set_sft_error( sft_status_t status )
{
    download_ctx.is_downloading = 0;
    download_ctx.result         = status;
}

/*@brief tracks the case where a cbor field was missing, and sets the corresponding error.
 *
 * Invoke if a cbor encoded message is missing a required parameter.
 * This will set the error state of the download context, report the error
 * to serial, and report it to the Device Logs service.
 */
void xi_missing_cbor_field_error( xi_context_handle_t in_context_handle,
                                  const char* cbor_message_name,
                                  const char* field_name )
{
    xi_set_sft_error( sft_cbor_missing_field_error );

    if ( NULL == cbor_message_name || NULL == field_name )
    {
        printf( "ERROR: invalid parameter passed to missing_cbor_field_error\n" );
        return;
    }

    printf( "ERROR: SFT Message: %s missing CBOR field: %s\n", cbor_message_name,
            field_name );

    /* Send to Device Logs */
    char log_details_str[LOGDETAILSSIZE];
    snprintf( log_details_str, sizeof( log_details_str ),
              "SFT Message: %s missing CBOR field: %s", cbor_message_name, field_name );

    xi_publish_device_log( in_context_handle, "File update available error",
                           log_details_str, sft_cbor_missing_field_error );
}


/*
 * Device Log Functions
 *
 * Note: Device log entires appear in the 'Logs' tab in Xively CPM's Device Details page
 * of your
 * particular Xively provisioned device.
 *
 */

/* @brief creates an entry in the device log server, whether good or bad. */
void xi_publish_device_log( xi_context_handle_t in_context_handle,
                            const char* message_str,
                            const char* details_str,
                            const sft_status_t status )
{
    /* Formats a message in the correct Xively Device Logs format and publishes it
     * to the device logs topic. Note that, due to the asynchronous behavior
     * of the client for the platform, this requires a few Xively ticks before
     * sending, so don't reboot immediately following a logged error! */

    if ( NULL == message_str || NULL == details_str )
    {
        printf(
            "\nWarning: xi_publish_device_log invoked with a null string parameter\n" );
        return;
    }
    else if ( sft_status_max <= status )
    {
        printf( "\nWarning: xi_publish_device_log invoked with an invalid status "
                "parametern" );
        return;
    }

    char* severity_str = "";
    switch ( status )
    {
        case sft_status_rebooting:
            severity_str = "alert";
            break;
        case sft_status_ok:
            severity_str = "notice";
            break;
        default:
            severity_str = "error";
            break;
    }

    char device_log_buffer[400];
    snprintf(
        device_log_buffer, sizeof( device_log_buffer ),
        "{\"message\":\"%s\",\"severity\":\"%s\",\"code\":\"%d\",\"details\":\"%s: %s\"}",
        message_str, severity_str, status, sft_status_strings[status], details_str );
    printf( "publishing log: %s\n", device_log_buffer );
    xi_publish( in_context_handle, xi_logtopic, device_log_buffer,
                XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

/* @brief utility function that formats an integer into the device logs message. */
void xi_publish_device_log_format_int( xi_context_handle_t in_context_handle,
                                       const char* message_str,
                                       const char* details_str,
                                       const int value_int,
                                       const sft_status_t status )
{
    char log_details[LOGDETAILSSIZE];
    strncpy( log_details, details_str, LOGDETAILSSIZE );
    snprintf( log_details, LOGDETAILSSIZE, details_str, value_int );
    xi_publish_device_log( in_context_handle, message_str, log_details, status );
}

/* @brief utility function that formats two integers into the device logs message. */
void xi_publish_device_log_format_int_int( xi_context_handle_t in_context_handle,
                                           const char* message_str,
                                           const char* details_str,
                                           const int value_int1,
                                           const int value_int2,
                                           const sft_status_t status )
{
    char log_details[LOGDETAILSSIZE];
    strncpy( log_details, details_str, LOGDETAILSSIZE );
    snprintf( log_details, LOGDETAILSSIZE, details_str, value_int1, value_int2 );
    xi_publish_device_log( in_context_handle, message_str, log_details, status );
}

/* @brief utility function that formats a string into the device logs message. */
void xi_publish_device_log_format_str( xi_context_handle_t in_context_handle,
                                       const char* message_str,
                                       const char* details_str,
                                       const char* value_str,
                                       const sft_status_t status )
{
    char log_details[LOGDETAILSSIZE];
    strncpy( log_details, details_str, LOGDETAILSSIZE );
    snprintf( log_details, LOGDETAILSSIZE, details_str, value_str );
    xi_publish_device_log( in_context_handle, message_str, log_details, status );
}

/* @brief utility function that formats two strings into the device logs message. */
void xi_publish_device_log_format_str_str( xi_context_handle_t in_context_handle,
                                           const char* message_str,
                                           const char* details_str,
                                           const char* value_str1,
                                           const char* value_str2,
                                           const sft_status_t status )

{
    char log_details[LOGDETAILSSIZE];
    strncpy( log_details, details_str, LOGDETAILSSIZE );
    snprintf( log_details, LOGDETAILSSIZE, details_str, value_str1, value_str2 );
    xi_publish_device_log( in_context_handle, message_str, log_details, status );
}

/* Functions useful for debugging */
void print_hash( unsigned char hash[] )
{
    int idx;
    for ( idx = 0; idx < 32; idx++ )
    {
        printf( "%02x", hash[idx] );
    }
    printf( "\n" );
}

#ifdef XI_DEBUGSFT
void dump( const cn_cbor* cb, char* out, char** end, int indent )
{
    int i;
    cn_cbor* cp;
    char finchar = ')'; /* most likely */

#define CPY( s, l )                                                                      \
    memcpy( out, s, l );                                                                 \
    out += l;
#define OUT( s ) CPY( s, sizeof( s ) - 1 )
#define PRF( f, a ) out += sprintf( out, f, a )

    if ( !cb )
        goto done;

    for ( i = 0; i < indent; i++ )
        *out++ = ' ';
    switch ( cb->type )
    {
        case CN_CBOR_TEXT_CHUNKED:
            OUT( "(_\n" );
            goto sequence;
        case CN_CBOR_BYTES_CHUNKED:
            OUT( "(_\n\n" );
            goto sequence;
        case CN_CBOR_TAG:
            PRF( "%ld(\n", cb->v.sint );
            goto sequence;
        case CN_CBOR_ARRAY:
            finchar = ']';
            OUT( "[\n" );
            goto sequence;
        case CN_CBOR_MAP:
            finchar = '}';
            OUT( "{\n" );
            goto sequence;
        sequence:
            for ( cp = cb->first_child; cp; cp = cp->next )
            {
                dump( cp, out, &out, indent + 2 );
            }
            for ( i = 0; i < indent; i++ )
                *out++ = ' ';
            *out++     = finchar;
            break;
        case CN_CBOR_BYTES:
            OUT( "h'\n" );
            for ( i = 0; i < cb->length; i++ )
            {
                PRF( "%02x ", cb->v.str[i] & 0xff );
            }

            *out++ = '\'';
            break;
        case CN_CBOR_TEXT:
            *out++ = '"';
            CPY( cb->v.str, cb->length ); /* should escape stuff */
            *out++ = '"';
            break;
        case CN_CBOR_NULL:
            OUT( "null" );
            break;
        case CN_CBOR_TRUE:
            OUT( "true" );
            break;
        case CN_CBOR_FALSE:
            OUT( "false" );
            break;
        case CN_CBOR_UNDEF:
            OUT( "simple(23)" );
            break;
        case CN_CBOR_INT:
            PRF( "%ld", cb->v.sint );
            break;
        case CN_CBOR_UINT:
            PRF( "%lu", cb->v.uint );
            break;
        case CN_CBOR_DOUBLE:
            PRF( "%e", cb->v.dbl );
            break;
        case CN_CBOR_SIMPLE:
            PRF( "simple(%ld)", cb->v.sint );
            break;
        default:
            PRF( "???%d???", cb->type );
            break;
    }
    *out++ = '\n';
done:
    *end = out;
}
#endif
