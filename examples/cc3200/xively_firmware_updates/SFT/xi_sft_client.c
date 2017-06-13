/*  Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
    This is part of Xively C library. */

#include <xively.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../cbor/cbor.h"
#include "../cbor/cn-cbor/cn-cbor.h"
#include "../sha256/sha256.h"
#include "xi_sft.h"

/* Simpelink EXT lib includes */
#include "flc_api.h"

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
#define  UART_PRINT  Report
#undef   printf
#define  printf      Report

///// Test these individually
#define MAX_INCOMING_FILENAME_LENGTH 50
#define MAX_STRING_LENGTH 50

//
// TYPEDEFS
//
typedef unsigned char uchar;
typedef unsigned int uint;

typedef struct file_download_ctx_s
{
    int file_length;
    int file_storage_offset;
    int result;

    long file_handle;
    unsigned long file_token;

    char file_name[MAX_STRING_LENGTH];
    char file_revision[MAX_STRING_LENGTH];
    unsigned char file_sft_fingerprint[64];
    unsigned char file_local_computed_fingerprint[32];

    SHA256_CTX sha256_context;

} file_download_ctx_t;

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
int downloading = 0;

int tenpercent = 0;
int currenttenpercent = 0;

char firmware_filename[] = "firmware";
char firmware_revision[] = "1.0";

file_download_ctx_t download_context;

char buffer[400];

#define MESSAGESIZE 50
char message[MESSAGESIZE];
#define DETAILSSIZE 400
char details[DETAILSSIZE];

/* Buffers to hold the formatted topics.
 * Topics are mangled by account and device ids.
 */
char xi_stopic[128];
char xi_logtopic[128];
char xi_ctopic[128];

typedef enum {
    sft_status_ok = 0,                       /* 0 */
    sft_open_file_error,
    sft_close_file_error,
    sft_write_error,
    sft_empty_length_array_error,
    sft_data_out_of_bounds_error,
    sft_missing_cbor_field_error,
    sft_publish_error,
    sft_fingerprint_mismatch_error
} sft_status_t;



void xi_parse_file_update_available( xi_context_handle_t in_context_handle, cn_cbor *cb );


void xi_publish_file_info( xi_context_handle_t in_context_handle );
int xi_publish_file_chunk_request( xi_context_handle_t in_context_handle, const char* file_name, const char* file_revision, int offset );

void xi_process_file_chunk( xi_context_handle_t in_context_handle, cn_cbor* in_cb );
int xi_parse_file_chunk( cn_cbor* in_cb, xi_sft_file_chunk_t* out_sft_file_chunk );

void print_hash( unsigned char hash[] )
{
   int idx;
   for ( idx=0; idx < 32; idx++ )
   {
      printf( "%02x",hash[idx] );
   }
   printf( "\n" );
}

//****************************************************************************
//
//! Reboot the MCU by requesting hibernate for a short duration
//!
//! \return None
//
//****************************************************************************
static void RebootMCU()
{

  //
  // Configure hibernate RTC wakeup
  //
  PRCMHibernateWakeupSourceEnable( PRCM_HIB_SLOW_CLK_CTR );

  //
  // Delay loop
  //
  MAP_UtilsDelay( 8000000 );

  //
  // Set wake up time
  //
  PRCMHibernateIntervalSet( 330 );

  //
  // Request hibernate
  //
  PRCMHibernateEnter();

  //
  // Control should never reach here
  //
  while( 1 )
  {

  }
}

void publish_device_log( xi_context_handle_t context_handle, const char* message_str, const char* details_str, const sft_status_t status )
{
    static char* notice_str = "notice";
    static char* error_str = "error";

    if( NULL == message || NULL == details )
    {
        printf("\nWarning: publish_device_log invoked with a null string parameter\n\n");
        return;
    }

    const char* severity_str;
    if( sft_status_ok == status )
    {
        severity_str = notice_str;
    }
    else
    {
        severity_str = error_str;
    }

    char device_log_buffer[400];
    snprintf( device_log_buffer, sizeof(device_log_buffer),"{\"message\":\"%s\",\"severity\":\"%s\",\"details\":\"SFT Status Code: %d\n%s\"}", message_str, severity_str, status, details_str );

    xi_publish( context_handle, xi_logtopic, device_log_buffer, XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

void verify_sha256( xi_context_handle_t in_context_handle )
{
    /* Finalize SHA256 Digest */
    sha256_final( &download_context.sha256_context, download_context.file_local_computed_fingerprint );
    printf( "Calculated hash = 0x" );
    print_hash( download_context.file_local_computed_fingerprint );

    char hash_buffer[65];
    char* buff_ptr = hash_buffer;

    int i;
    for( i = 0; i < 32; ++i )
    {
        buff_ptr += sprintf( buff_ptr, "%02x", download_context.file_local_computed_fingerprint[i] );
    }
    *buff_ptr = '\0';

    for( i = 0; i < 64; ++i )
    {
        if( hash_buffer[i] != download_context.file_sft_fingerprint[i] )
        {
            printf("MISMATCH IN FILE FINGERPRINT.\n");
            printf( "index:    %d\n", i );
            printf( "computed: %c\n", hash_buffer[i] );
            printf( "sft:      %c\n", download_context.file_sft_fingerprint[i] );

            download_context.result = sft_fingerprint_mismatch_error;
            char log_details[DETAILSSIZE];
            snprintf( log_details, DETAILSSIZE,"nsft-provided fingerprint: \"%s\"\ndevice-calculated fingerprint: \"%s\"",
                     download_context.file_sft_fingerprint, download_context.file_local_computed_fingerprint );
            publish_device_log( in_context_handle, "File Update Fingerprint Mistmach", details, sft_fingerprint_mismatch_error );
            return;
        }
    }

    printf(" fingerprints match!");
}

void on_sft_message( xi_context_handle_t in_context_handle,
                            xi_sub_call_type_t call_type,
                            const xi_sub_call_params_t* const params,
                            xi_state_t state,
                            void* user_data )
{
    cn_cbor* cb = NULL;
    char* bufptr = NULL;
    cn_cbor* cb_item = NULL;
    const uint8_t* payload_data = NULL;
    size_t payload_length = 0;
    int retVal;

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
            //printf( "topic:%s. message received.\n", params->message.topic );

            /* get the payload parameter that contains the data that was sent. */
            payload_data = params->message.temporary_payload_data;
            payload_length = params->message.temporary_payload_data_length;

            /* Figure out what the packet type is and then call the appropriate parser */
            cb = cn_cbor_decode(payload_data, payload_length, 0);

            if (cb) {
#ifdef XI_DEBUGSFT
                dump(cb, buffer, &bufend, 0);
                *bufend = 0;
                printf("%s\n", buffer);
#endif

                cb_item = cn_cbor_mapget_string(cb, "msgtype");
                if(cb_item)
                {
                    switch(cb_item->v.uint)
                    {
                        case XI_FILE_UPDATE_AVAILABLE:

							printf("Got FILE_UPDATE_AVAILABLE\n");
							if( downloading )
							{
							    printf("WARNING: ALREADY DOWNLOADING!\n");
							    cn_cbor_free(cb CBOR_CONTEXT_PARAM);
							    break;
							}

                            //
                            // Parse FILE_UPDATE_AVAILABLE messsage
                            // and format GET_FILE_CHUNK message into the outgoing cbor buffer `encoded`
                            //
                            xi_parse_file_update_available( in_context_handle, cb );

                            //
                            // Free the incoming cbor message context
                            //
                            cn_cbor_free(cb CBOR_CONTEXT_PARAM);

                            /* Log the FILE_UPDATE_AVAILABLE information */
                            snprintf(details,sizeof(details),"name=%s revision=%s length = %d",
                                     firmware_filename, download_context.file_revision, download_context.file_length, bufptr );
                            publish_device_log( in_context_handle, "File update available", details, sft_status_ok );

                            /* Open the file for writing */
                            retVal =  sl_extlib_FlcOpenFile("/sys/mcuimgA.bin", download_context.file_length, &download_context.file_token, &download_context.file_handle, FS_MODE_OPEN_WRITE);

                            /* Check for errors */
                            if( retVal < 0 )
                            {
                                /*  TODO: send a log message to the xively logging service with the error */
                                printf(" ERROR: mcuimgA.bin open returned %d\n",retVal);
                                download_context.result = -1;
                                downloading = 0;
                            }
                            break;

					case XI_FILE_CHUNK:

					        /* Parse the FILE_CHUNK message and send another
					         * FILE_GET_CHUNK request if we're still downloading data */
					        xi_process_file_chunk( in_context_handle, cb );

					        /* Free the incoming cbor message context */
							cn_cbor_free(cb CBOR_CONTEXT_PARAM);

							/* Since we're getting file chunks then we were in a downloading state.
							 * If we're not currently downloading, that means the download finished
							 * or encountered an error. */
							if( ! downloading )
							{
                                printf( "=================\n");
                                printf( "Done downloading.\n");
                                printf( "   result: %d\n", download_context.result );
                                printf( "=================\n");

                                /* Log the download complete information
                                 *   TODO: report download_context.result value to the logging service */
                                snprintf( message,sizeof( message ),"File download complete" );
                                snprintf( details,sizeof( details ),"name=%s revision=%s length = %d", firmware_filename , download_context.file_revision, download_context.file_length );
                                snprintf( buffer,sizeof( buffer ),"{\"message\":\"%s\",\"severity\":\"notice\",\"details\":\"%s\"}", message,details );
                                xi_publish( in_context_handle, xi_logtopic, buffer, XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );

                                verify_sha256( in_context_handle );


                                /* Close the firmware file */
                                int result = 0;
                                result = sl_extlib_FlcCloseFile( download_context.file_handle, NULL, NULL, 0 );
                                printf("*** Close File Result: %d\n", result);



                                /* Set the bootloader to test the new image */
                                if( download_context.result || result )
                                {
                                    printf("NOT MARKING FILE TO TEST DUE TO ERRORS\n");
                                }
                                else
                                {
                                    result = sl_extlib_FlcTest( FLC_TEST_RESET_MCU | FLC_TEST_RESET_MCU_WITH_APP );
                                    printf("*** FLC Test Result: %d, rebooting\n", result);

                                    /* Reboot the device to test the image */
                                    RebootMCU();
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

void
xi_publish_file_info( xi_context_handle_t in_context_handle )
{
    unsigned char encoded[128];
    ssize_t enc_sz;

    /* Let's try making a XI_FILE_INFO packet */
    cn_cbor_errback err;
    cn_cbor *cb_map = cn_cbor_map_create( &err );

    cn_cbor_map_put( cb_map, cn_cbor_string_create( "msgtype", &err ),
                     cn_cbor_int_create( XI_FILE_INFO, &err ), &err );

    cn_cbor_map_put( cb_map, cn_cbor_string_create("msgver", &err ),
                     cn_cbor_int_create( 1, &err ), &err );

    cn_cbor *a = cn_cbor_array_create( &err );
    cn_cbor *cb_file1 = cn_cbor_map_create( &err );
    cn_cbor_map_put( cb_file1, cn_cbor_string_create( "N", &err ),
                     cn_cbor_string_create( firmware_filename, &err ), &err );

    cn_cbor_map_put( cb_file1, cn_cbor_string_create( "R", &err ),
                     cn_cbor_string_create( "-1", &err ), &err );

    cn_cbor_array_append( a, cb_file1, &err );
    cn_cbor_map_put( cb_map, cn_cbor_string_create( "list", &err ),
                     a, &err );

    enc_sz = cn_cbor_encoder_write( encoded, 0, sizeof( encoded ), cb_map );
    cn_cbor_free( cb_map CBOR_CONTEXT_PARAM );

    if( 0 >= enc_sz )
    {
        /* TODO: Log error to Device Logs. */
    }

    /* publish the FILE_INFO message */
    xi_publish_data( in_context_handle, xi_stopic, encoded, enc_sz, XI_MQTT_QOS_AT_MOST_ONCE,
                     XI_MQTT_RETAIN_FALSE, NULL, NULL );
    printf( "Published FILE_INFO to %s\n",xi_stopic );
}


void xi_parse_file_update_available( xi_context_handle_t in_context_handle, cn_cbor *cb)
{
    cn_cbor *cb_list;
    cn_cbor *cb_item;
    cn_cbor *cb_file;
    int filelistlength = 0;
    int i,index;

    /* Zero out download_context */
    memset( &download_context, 0, sizeof( file_download_ctx_t ) );

    /* Initialize sha256 digest of the file and tracking variables */
    sha256_init( &download_context.sha256_context );

    cb_item = cn_cbor_mapget_string( cb, "msgver" );
    if( cb_item )
    {
        printf( "FILE_UPDATE_AVAILABLE: msgver = %d\n",cb_item->v.uint );
        if( 1 != cb_item->v.uint )
        {
            return;
        }
    }

    /* Get the File List.  For this demo it should be only one file */
    cb_list = cn_cbor_mapget_string(cb, "list");
    if(cb_list)
    {
        filelistlength = cb_list->length;
        printf("FILE_UPDATE_AVAILABLE: list length = %d\n",filelistlength);
    }

    /* iterate over the file list */
    for( index = 0; index < filelistlength; index++ )
    {
        cb_item = cn_cbor_index(cb_list, index);

        if( cb_item )
        {
        	    // File 'N'ame
            cb_file = cn_cbor_mapget_string(cb_item, "N");
            if( cb_file )
            {
                if( strlen( firmware_filename ) != cb_file->length ||
                    strncmp( firmware_filename, cb_file->v.str, cb_file->length ) != 0 )
                {
                    printf( "ERROR: incoming file update has an unexpected name: \"" );
                    int i;
                    for( i = 0; i < cb_file->length; ++i )
                    {
                        printf( "%c", cb_file->v.str[i] );
                    }
                    printf( "\", expected: \"%s\"\n", firmware_filename );
                    return;
                }
            }
            else
            {
                printf( "No key called N\n" );
                return;
            }

            // File 'R'evision
            cb_file = cn_cbor_mapget_string(cb_item, "R");
            if( cb_file )
            {
                strncpy( download_context.file_revision, cb_file->v.str, sizeof( download_context.file_revision ) );

                if( (unsigned int) cb_file->length < sizeof( download_context.file_revision ) )
                {
                    download_context.file_revision[cb_file->length] = '\0';
                }

                download_context.file_revision[sizeof(download_context.file_revision)-1] = '\0';
            }
            else
            {
                printf("No key called R\n");
                return;
            }

            // File 'S'ize
            cb_file = cn_cbor_mapget_string(cb_item, "S");
            if(cb_file)
            {
                printf( "FILE_UPDATE_AVAILABLE: file length = %d\n",cb_file->v.sint );
                download_context.file_length = cb_file->v.sint;
                tenpercent = download_context.file_length / 10;
                currenttenpercent = tenpercent;
            }
            else
            {
                printf("No key called S\n");
                return;
            }

            // File 'F'ingerprint
            cb_file = cn_cbor_mapget_string(cb_item, "F");
            if( cb_file )
            {
                printf( "Storing fingerprint: ");
                printf( "length: %d\n", cb_file->length );
                printf(" string: %s\n", cb_file->v.str );

                if( sizeof(download_context.file_sft_fingerprint) < cb_file->length )
                {
                    /* TODO:
                     * log error
                     * set error flag for downoading state */
                }
                else
                {
                    for( i = 0 ; i < cb_file->length; i++ )
                    {
                        download_context.file_sft_fingerprint[i] = cb_file->v.str[i];
                    }
                }
            }
            else
            {
                printf("No key called F\n");
                return;
            }

        }
        else
        {
            printf("No item at index %d\n",i);
            return;
        }
    }

    /* Set Download State to Active */
    downloading = 1;

    /* Fetch the initial chunk of the file */
    if( 0 != xi_publish_file_chunk_request( in_context_handle, firmware_filename, download_context.file_revision,
                                   download_context.file_storage_offset ) )
    {
        downloading = 0;
        download_context.result = -1;

        /* Warning: the likelihood that the following Device Logs publish works after the previous
         * publish failing its quite slim.
         */
        /* TODO: Report error to SFT
         * TODO: Report error to Device Logs */
    }
}

/*
 * Formats a FILE_GET_CHUNK cbor message with the provided parameters and then publishes
 * the message to the SFT Service.
 *
 * Returns 0 on success, or -1 if the message could not be queued for publication
 * (probably due to a too-small encoding buffer)
 */
int xi_publish_file_chunk_request( xi_context_handle_t in_context_handle, const char* file_name, const char* file_revision, int offset )
{
    const int message_type = XI_FILE_GET_CHUNK;
    const int message_version = 1;
    const int chunk_length = 1024;

#if 0
    printf( "XI_FILE_GET_CHUNK request params: " );
    printf( "N: \"%s\", ", file_name );
    printf( "R: \"%s\", ", file_revision );
    printf( "O: %d, ", offset );
    printf( "L: %d\n", chunk_length );
#endif

    /* Format a CBOR message of the FILE_GET_CHUNK message type. */
    cn_cbor_errback err;
    cn_cbor *cb_map = cn_cbor_map_create(&err);

    cn_cbor_map_put( cb_map, cn_cbor_string_create( "msgtype", &err ),
                     cn_cbor_int_create( message_type, &err ), &err );

    cn_cbor_map_put( cb_map, cn_cbor_string_create( "msgver", &err ),
                     cn_cbor_int_create( message_version, &err ), &err );

    cn_cbor_map_put( cb_map, cn_cbor_string_create( "N", &err ),
                     cn_cbor_string_create( file_name, &err ), &err);

    cn_cbor_map_put( cb_map, cn_cbor_string_create( "R", &err ),
                     cn_cbor_string_create( file_revision, &err ), &err);

    cn_cbor_map_put( cb_map, cn_cbor_string_create("O", &err ),
                     cn_cbor_int_create(offset, &err ), &err );

    cn_cbor_map_put( cb_map, cn_cbor_string_create("L", &err ),
                     cn_cbor_int_create(chunk_length, &err ), &err );

    unsigned char encoded_cbor_msg[128];
    ssize_t encoded_size = cn_cbor_encoder_write( encoded_cbor_msg, 0, sizeof( encoded_cbor_msg ), cb_map );
    cn_cbor_free( cb_map CBOR_CONTEXT_PARAM );

    if( 0 >= encoded_size )
    {
        printf("Could not encode the FILE_CHUNK request in the specified buffer\n");
        return -1;
    }

    if( 0 != xi_publish_data( in_context_handle, xi_stopic, encoded_cbor_msg, encoded_size,
                                         XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL ) )
    {
        printf("Xively Publish returned an error\n");
        return -1;
    }

    return 0;
}

/* Process the cbor object as a FILE_CHUNK message, calling xi_parse_file_chunk to extract the
 * cbor fields and then acting on them to store the data into the flash file system's
 * firmware image file.  If there is more to be downloaded then a subsequent
 * GET_CHUNK request is formatted and published.
 */
void xi_process_file_chunk( xi_context_handle_t in_context_handle, cn_cbor *cb )
{
    xi_sft_file_chunk_t file_chunk_data;
    if( 0 != xi_parse_file_chunk( cb, &file_chunk_data ) )
    {
        download_context.result = -1;
        downloading = 0;
        return;
    }

    /* printf( "---- FILE_CHUNK: length = %d FILE_LENGTH: %d\n", file_chunk_data.chunk_length, download_context.file_length ); */

    /* Ensure that this chunk starts where we left off. */
    if( download_context.file_storage_offset != file_chunk_data.offset )
    {
        /* TODO: Report error to SFT
         * TODO: Report error to Device Logs */
        printf("ERROR: unexpected Chunk Offset Delivered by SFT!\n");
        printf("  expected: %d  sft chunk offset: %d\n", download_context.file_storage_offset, file_chunk_data.offset );
        download_context.result = -1;
        downloading = 0;
    }

    /* Track Progress */
    if( currenttenpercent < file_chunk_data.offset )
    {
        printf("Downloaded %d bytes %d %%  of the file \n", download_context.file_storage_offset,
                 (100 * (currenttenpercent + 1000) ) / download_context.file_length );
        currenttenpercent += tenpercent;
    }

    /* Ensure that we have a buffer to write from */
    if( file_chunk_data.array_length )
    {
        /* convert from unsigned to signed becuase TI's API has a signed length variable */
        if( file_chunk_data.offset + file_chunk_data.array_length > download_context.file_length )
        {
            /* Request to write past the file length
             * TODO: Report error to SFT
             * TODO: Report error to Device Logs */
            downloading = 0;
        }

        /* update the SHA256 digest with the provided data */
        sha256_update( &download_context.sha256_context, (unsigned char *) file_chunk_data.byte_array, file_chunk_data.array_length );

        int write_length = (int) file_chunk_data.array_length;
        if( 0 == write_length )
        {
            /* TODO: check the status code
             * TODO: Report error to SFT
             * TODO: Report error to Device Logs */
            downloading = 0;
        }

        if( 0 > write_length )
        {
            /* Negative values means that we lost data in the unsigned -> signed conversion.
             * TODO: Report error to SFT
             * TODO: Report error to Device Logs */
            printf("ERROR: WriteBuffer is too large for CC3200 sl_extlib_FlcWriteFile operations\n");
            downloading = 0;
            download_context.result = -1;
        }
        else
        {
            int bytes_written = sl_extlib_FlcWriteFile( download_context.file_handle, file_chunk_data.offset,
                                                        (unsigned char *) file_chunk_data.byte_array, write_length );
            if( 0 >= bytes_written )
            {
                /* Write Failed
                 * TODO: Report error to SFT
                 * TODO: Report error to Device Logs */
                printf("ERROR: Write Returned error code: %d\n", bytes_written );
                download_context.result = -1;
                downloading = 0;
            }
            else
            {
                /* Track our progress through the file */
                download_context.file_storage_offset = file_chunk_data.offset + bytes_written;

                /* Check for completeness */
                if( download_context.file_storage_offset >= download_context.file_length )
                {
                    /* Download Complete */
                    downloading = 0;
                }
            }
        }
    }
    else
    {
        /* Zero Length Write Buffer
         * TODO: check for status of 1 for EoF
         * TODO: Report error to SFT
         * TODO: Report error to Device Logs */
        printf("ERROR: Write Buffer from SFT is of zero length\n" );
        download_context.result = -1;
        downloading = 0;
    }

    /* TODO: check for status of 1 for EoF */
    if( file_chunk_data.status > 1 )
    {
        printf("ERROR: SFT packet reported Chunk Status Request Error: %d\n", file_chunk_data.status );

        /* TODO: Report error to SFT
         * TODO: Report error to Device Logs */
        download_context.result = -1;
        downloading = 0;
    }

    if( 1 == downloading )
    {
        /* If still download  continuing, create another FILE_CHUNK request to the SFT service */
        xi_publish_file_chunk_request( in_context_handle, firmware_filename,
                                           download_context.file_revision, download_context.file_storage_offset );
    }
}

/* Parses the CBOR message to get the FILE_CHUNK required FILE_CHUNK fields
 * and places the data in the out_sft_file_chunk structure
 * Returns 0 on success, or -1 if there are missing fields or null parameters
 */
int xi_parse_file_chunk( cn_cbor* in_cb, xi_sft_file_chunk_t* out_sft_file_chunk )
{
    if( NULL == in_cb || NULL == out_sft_file_chunk )
    {
        return -1;
    }

    /* use the RED LED to signal downloading progress */
    static int ledToggle = 0;
    ledToggle = ( ledToggle + 1 ) % 2;
    if( ledToggle )
    {
        GPIO_IF_LedOn( MCU_RED_LED_GPIO );
    }
    else
    {
        GPIO_IF_LedOff( MCU_RED_LED_GPIO );
    }

    cn_cbor *cb_item = NULL;
    memset( out_sft_file_chunk, 0, sizeof(xi_sft_file_chunk_t) );

    /* msg version */
    cb_item = cn_cbor_mapget_string( in_cb, "msgver" );
    if(cb_item)
    {
        if( 1 != cb_item->v.uint )
        {
            printf( "FILE_CHUNK unsupported message version: %d\n", cb_item->v.uint );
            return -1;
        }
    }
    else
    {
       printf( "FILE_CHUNK message missing message version field\n" );
       return -1;
    }

    /* chunk offset */
    cb_item = cn_cbor_mapget_string( in_cb, "O" );
    if( cb_item )
    {
       out_sft_file_chunk->offset = cb_item->v.uint;
    }
    else
    {
        printf("FILE_CHUNK message missing offset field\n");
        return -1;
    }

    /* chunk length */
    cb_item = cn_cbor_mapget_string( in_cb, "L" );
    if( cb_item )
    {
        out_sft_file_chunk->chunk_length = cb_item->v.uint;
    }
    else
    {
        printf( "FILE_CHUNK message missing length field\n" );
        return -1;
    }

    /* Status */
    cb_item = cn_cbor_mapget_string( in_cb, "S" );
    if( cb_item )
    {
        out_sft_file_chunk->status = cb_item->v.uint;
    }
    else
    {
        printf( "FILE_CHUNK message missing status field\n" );
        return -1;
    }

    /* Byte Array */
    cb_item = cn_cbor_mapget_string( in_cb, "C" );
    if(cb_item)
    {
        out_sft_file_chunk->array_length = cb_item->length;
        out_sft_file_chunk->byte_array = (unsigned char *)cb_item->v.str;
    }
    else
    {
        printf( "FILE_CHUNK message missing byte array field\n" );
    }

    return 0;
}


int
xi_sft_init(xi_context_handle_t in_context_handle, char *account_id, char *device_id)
{
	/* Accept the latest firmware if we get here. */
	if( sl_extlib_FlcIsPendingCommit() )
	{
	    printf("******************* Committing this revision\n");
	    sl_extlib_FlcCommit(FLC_COMMITED);
	}

	if( NULL == account_id )
	{
	    return -1;
	}

	if( NULL == device_id )
    {
	    return -1;
    }

	/*
	 * Mangle the SFT and Device Logs topics with the provided credentials
	 */

	/* xi_stopic is the MQTT topic we PUB to to send messages to the broker. */
	snprintf(xi_stopic, sizeof(xi_stopic),"xi/ctrl/v1/%s/svc", device_id);

	/* xi_ctopic is the MQTT topic we SUBSCRIBE to to receive messages from the broker. */
	snprintf(xi_ctopic, sizeof(xi_ctopic),"xi/ctrl/v1/%s/cln", device_id);

	/* xi_logtopic is the MQTT topic we PUB to to add log messages. */
	snprintf(xi_logtopic, sizeof(xi_logtopic),"xi/blue/v1/%s/d/%s/_log",account_id, device_id);

	/* Everything interesting happens in the on_sft_message() state machine. */
	void* user_data = NULL;
	xi_subscribe(in_context_handle, xi_ctopic, XI_MQTT_QOS_AT_MOST_ONCE, on_sft_message, user_data );

	return 0;
}


// Functions useful for debugging:
#ifdef XI_DEBUGSFT
void dump(const cn_cbor* cb, char* out, char** end, int indent) {
    int i;
    cn_cbor* cp;
    char finchar = ')';           /* most likely */

#define CPY(s, l) memcpy(out, s, l); out += l;
#define OUT(s) CPY(s, sizeof(s)-1)
#define PRF(f, a) out += sprintf(out, f, a)

    if (!cb)
        goto done;

    for (i = 0; i < indent; i++) *out++ = ' ';
    switch (cb->type) {
        case CN_CBOR_TEXT_CHUNKED:   OUT("(_\n");                  goto sequence;
        case CN_CBOR_BYTES_CHUNKED:  OUT("(_\n\n");                goto sequence;
        case CN_CBOR_TAG:            PRF("%ld(\n", cb->v.sint);    goto sequence;
        case CN_CBOR_ARRAY:  finchar = ']'; OUT("[\n");            goto sequence;
        case CN_CBOR_MAP:    finchar = '}'; OUT("{\n");            goto sequence;
        sequence:
            for (cp = cb->first_child; cp; cp = cp->next) {
                dump(cp, out, &out, indent+2);
            }
            for (i=0; i<indent; i++) *out++ = ' ';
            *out++ = finchar;
            break;
        case CN_CBOR_BYTES:   OUT("h'\n");
            for (i=0; i<cb->length; i++) {
                PRF("%02x ", cb->v.str[i] & 0xff);
            }

            *out++ = '\'';
            break;
        case CN_CBOR_TEXT:    *out++ = '"';
            CPY(cb->v.str, cb->length); /* should escape stuff */
            *out++ = '"';
            break;
        case CN_CBOR_NULL:   OUT("null");                      break;
        case CN_CBOR_TRUE:   OUT("true");                      break;
        case CN_CBOR_FALSE:  OUT("false");                     break;
        case CN_CBOR_UNDEF:  OUT("simple(23)");                break;
        case CN_CBOR_INT:    PRF("%ld", cb->v.sint);           break;
        case CN_CBOR_UINT:   PRF("%lu", cb->v.uint);           break;
        case CN_CBOR_DOUBLE: PRF("%e", cb->v.dbl);             break;
        case CN_CBOR_SIMPLE: PRF("simple(%ld)", cb->v.sint);   break;
        default:             PRF("???%d???", cb->type);        break;
    }
    *out++ = '\n';
done:
    *end = out;
}
#endif
