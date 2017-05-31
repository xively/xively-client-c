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

#if(0)
#include "rom_map.h"
#include "hw_types.h"
#include "prcm.h"
#endif
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

extern int readBootinfo();

///// Test these individually
#define MAX_INCOMING_FILENAME_LENGTH 50

int filelength = 0;
int offset = 0;
int downloading = 0;
int tenpercent = 0;
int currenttenpercent = 0;
char filename[50] = "OS?";
char installedfilerevision[50] = "0.0";
char fileRevision[50] = "0.0";
char incomingFilename[MAX_INCOMING_FILENAME_LENGTH] ="";
unsigned char filefingerprint[32];
unsigned char hash[32];
int idx;
typedef unsigned char uchar;
typedef unsigned int uint;
long lFileHandle;
unsigned long ulToken;

SHA256_CTX ctx;

char buffer[400];

#define MESSAGESIZE 50
char message[MESSAGESIZE];
#define DETAILSSIZE 400
char details[DETAILSSIZE];

unsigned char encoded[1024];
ssize_t enc_sz;
char xi_stopic[128];
char xi_logtopic[128];
char xi_ctopic[128];

typedef struct xi_sft_file_chunk_s
{
    uint offset;
    uint chunk_length;
    uint array_length;
    uint status;
    unsigned char* byte_array;
} xi_sft_file_chunk_t;


void xi_process_file_chunk(cn_cbor* in_cb);
void xi_parse_file_update_available(cn_cbor *cb);
void xi_publish_file_info(xi_context_handle_t in_context_handle);

int xi_parse_file_chunk(cn_cbor* in_cb, xi_sft_file_chunk_t* out_sft_file_chunk );

void print_hash(unsigned char hash[])
{
   int idx;
   for (idx=0; idx < 32; idx++)
      printf("%02x",hash[idx]);
   printf("\n");
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
  PRCMHibernateWakeupSourceEnable(PRCM_HIB_SLOW_CLK_CTR);

  //
  // Delay loop
  //
  MAP_UtilsDelay(8000000);

  //
  // Set wake up time
  //
  PRCMHibernateIntervalSet(330);

  //
  // Request hibernate
  //
  PRCMHibernateEnter();

  //
  // Control should never reach here
  //
  while(1)
  {

  }
}

void on_sft_message( xi_context_handle_t in_context_handle,
                            xi_sub_call_type_t call_type,
                            const xi_sub_call_params_t* const params,
                            xi_state_t state,
                            void* user_data )
{
    cn_cbor *cb = NULL;
    char filefingerprintASCII[70];
    char *bufptr = NULL;
    int i = 0 ;
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
            //printf("PUBLISHING FILE INFO\n\r");
            //xi_publish_file_info(in_context_handle);
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
                if(cb_item) {
                    //printf("msgtype = %d\n",cb_item->v.uint);
                    switch(cb_item->v.uint) {
                        case XI_FILE_UPDATE_AVAILABLE:

							printf("Got FILE_UPDATE_AVAILABLE\n");

						    //
                            // Set Download State to Active
                            //
                            downloading = 1;

                            //
                            // Parse FILE_UPDATE_AVAILABLE messsage
                            // and format GET_FILE_CHUNK message into the outgoing cbor buffer `encoded`
                            //
                            xi_parse_file_update_available(cb);

                            //
                            // Free the incoming cbor message context
                            //
                            cn_cbor_free(cb CBOR_CONTEXT_PARAM);

                            //
                            // Publish the GET_FILE_CHUNK message, which has been encoded in the buffer `encoded`
                            //
                            xi_publish_data( in_context_handle, xi_stopic, encoded, enc_sz, XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
                            printf("logging file update available\n");

                            //
                            // Log the FILE_UPDATE_AVAILABLE information
                            //
                            snprintf(message,sizeof(message),"FILE_UPDATE_AVAILABLE");
                            bufptr = filefingerprintASCII;
                            for(i=0;i<32;i++) {
                                bufptr += sprintf(bufptr,"%02x",filefingerprint[i]);
                            }
                            *bufptr = '\0';
                            snprintf(message,sizeof(message),"File update available");
                            snprintf(details,sizeof(details),"name=%s revision=%s length = %d fingerprint = %s"
                                    ,filename,fileRevision,filelength,filefingerprintASCII);
                            snprintf(buffer,sizeof(buffer),"{\"message\":\"%s\",\"severity\":\"notice\",\"details\":\"%s\"}",message,details);
                            xi_publish( in_context_handle, xi_logtopic, buffer, XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );

                            //
                            // Initialize sha256 digest of the file and tracking variables
                            //
                            sha256_init(&ctx);
                            offset = 0;

                            //
                            // Open the file for writing
                            //
                            retVal =  sl_extlib_FlcOpenFile("/sys/mcuimgA.bin", filelength, &ulToken, &lFileHandle, FS_MODE_OPEN_WRITE);
                            printf("mcuimgA.bin open returned %d\n",retVal);

                            //
                            // Check for errors
                            //
                            if(retVal < 0)
                            {
                                downloading = 0;
                            }

                            break;
					case XI_FILE_CHUNK:
					        //
					        // Parse the FILE_CHUNK message
					        // and format a subsequent cbor chunk request in the buffer `encoded`
					        // if we're still downloading data
					        //
					        xi_process_file_chunk(cb);

					        //
					        // Free the incoming cbor message context
					        //
							cn_cbor_free(cb CBOR_CONTEXT_PARAM);

							//
							// If in downloading state, then send the next chunk request message
							//
							if( 1 == downloading )
							{
								if(offset < filelength)
								{
								    printf(" CALLBACK HANDLER Publishing for next chunk request\n");
									xi_publish_data( in_context_handle, xi_stopic, encoded, enc_sz, XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
								}
							}
							else
							{
							    //
							    // We aren't in a download state
							    // but we just were, which means the at the download is complete
							    //
                                downloading = 0;
                                sprintf(installedfilerevision, "%s",fileRevision);

                                printf("Done downloading. Offset = %d\n",offset);

                                //
                                // Log the download complete information
                                //
                                snprintf(message,sizeof(message),"File download complete");
                                snprintf(details,sizeof(details),"name=%s revision=%s length = %d",filename,installedfilerevision,filelength);
                                snprintf(buffer,sizeof(buffer),"{\"message\":\"%s\",\"severity\":\"notice\",\"details\":\"%s\"}",message,details);
                                xi_publish( in_context_handle, xi_logtopic, buffer, XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );

                                //
                                // Finalize SHA256 Digest
                                //
                                sha256_final(&ctx,hash);
                                printf("Calculated hash = 0x");
                                print_hash(hash);
                                offset = 0;

                                //
                                // Publish the new file info, though I'm not sure we should be doing this.
                                //
                                xi_publish_file_info(in_context_handle);

                                //
                                // Close the firmware file
                                //
                                int result = 0;
                                result = sl_extlib_FlcCloseFile(lFileHandle, NULL, NULL, 0);
                                printf("*** Close File Result: %d\n", result);

                                //
                                // Set the bootloader to test the new image
                                //
                                result = sl_extlib_FlcTest(FLC_TEST_RESET_MCU | FLC_TEST_RESET_MCU_WITH_APP);
                                printf("*** FLC Test Result: %d, rebooting\n", result);

                                //
                                // Reboot the device to test the image
                                //
                                RebootMCU();
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
xi_publish_file_info(xi_context_handle_t in_context_handle)
{
    unsigned char encoded[512];
    ssize_t enc_sz;

    //xi_parse_file_update_available(NULL);

    /* Let's try making a XI_FILE_INFO packet */
    cn_cbor_errback err;
    cn_cbor *cb_map = cn_cbor_map_create(&err);

    cn_cbor_map_put(cb_map,
                    cn_cbor_string_create("msgtype", &err),
                    cn_cbor_int_create(XI_FILE_INFO, &err),
                    &err);

    cn_cbor_map_put(cb_map,
                    cn_cbor_string_create("msgver", &err),
                    cn_cbor_int_create(1, &err),
                    &err);


    cn_cbor *a = cn_cbor_array_create(&err);
    cn_cbor *cb_file1 = cn_cbor_map_create(&err);

    cn_cbor_map_put(cb_file1,
                    cn_cbor_string_create("N", &err),
                    cn_cbor_string_create(filename, &err),
                    &err);
    cn_cbor_map_put(cb_file1,
                    cn_cbor_string_create("R", &err),
                    cn_cbor_string_create("-1", &err),
                    &err);

    cn_cbor_array_append(a, cb_file1, &err);

    cn_cbor_map_put(cb_map,
                    cn_cbor_string_create("list", &err),
                    a,
                    &err);

    enc_sz = cn_cbor_encoder_write(encoded, 0, sizeof(encoded), cb_map);

    cn_cbor_free(cb_map CBOR_CONTEXT_PARAM);

    /* publish the FILE_INFO message */
    xi_publish_data( in_context_handle, xi_stopic, encoded, enc_sz, XI_MQTT_QOS_AT_MOST_ONCE,
               XI_MQTT_RETAIN_FALSE, NULL, NULL );
    printf("Published FILE_INFO to %s\n",xi_stopic);
    snprintf(message,sizeof(message),"File info");
    snprintf(details,sizeof(details),"name=%s revision=%s" ,filename,installedfilerevision);
    snprintf(buffer,sizeof(buffer),"{\"message\":\"%s\",\"severity\":\"notice\",\"details\":\"%s\"}",message,details);
    xi_publish( in_context_handle, xi_logtopic, buffer, XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
    downloading = 0;
    sl_extlib_FlcCloseFile(lFileHandle, NULL, NULL, 0);
}


void xi_parse_file_update_available(cn_cbor *cb) {
    cn_cbor *cb_list;
    cn_cbor *cb_item;
    cn_cbor *cb_file;
    int filelistlength = 0;
    int i,index;

    cb_item = cn_cbor_mapget_string(cb, "msgver");
    if(cb_item) {
        printf("FILE_UPDATE_AVAILABLE: msgver = %d\n",cb_item->v.uint);
        if(1 != cb_item->v.uint) return;
    }
    cb_list = cn_cbor_mapget_string(cb, "list");
    if(cb_list) {
        filelistlength = cb_list->length;
        printf("FILE_UPDATE_AVAILABLE: list length = %d\n",filelistlength);
    }

    for(index=0;index<filelistlength;index++) {
        cb_item = cn_cbor_index(cb_list, index);

        if(cb_item) {
        	// File 'N'ame
            cb_file = cn_cbor_mapget_string(cb_item, "N");
            if(cb_file) {
            	strncpy(incomingFilename,cb_file->v.str, MAX_INCOMING_FILENAME_LENGTH);
            	if((unsigned int)cb_file->length < MAX_INCOMING_FILENAME_LENGTH) incomingFilename[cb_file->length] = '\0';
            	incomingFilename[MAX_INCOMING_FILENAME_LENGTH-1] = '\0';
            } else {
                printf("No key called N\n");
            }

            // File 'R'evision
            cb_file = cn_cbor_mapget_string(cb_item, "R");
            if(cb_file) {
            	strncpy(fileRevision,cb_file->v.str,sizeof(fileRevision));
            	if((unsigned int)cb_file->length < sizeof(fileRevision)) fileRevision[cb_file->length] = '\0';
            	fileRevision[sizeof(fileRevision)-1] = '\0';
            } else {
                printf("No key called R\n");
            }

            // File 'S'ize
            cb_file = cn_cbor_mapget_string(cb_item, "S");
            if(cb_file) {
                printf("FILE_UPDATE_AVAILABLE: file length = %d\n",cb_file->v.sint);
                filelength = cb_file->v.sint;
                tenpercent = filelength / 10;
                currenttenpercent = tenpercent;
                printf("Filelength %d\n",filelength);
            } else {
                printf("No key called S\n");
            }

            // File 'F'ingerprint
            cb_file = cn_cbor_mapget_string(cb_item, "F");
            if(cb_file) {
            	for(i=0;i<32;i++) filefingerprint[i] = cb_file->v.str[i];
                printf("FILE_UPDATE_AVAILABLE: file fingerprint = 0x");
                for(i=0;i<32;i++) printf("%02x",filefingerprint[i]);
                printf("\n");
            } else {
                printf("No key called F\n");
            }

        } else {
            printf("No item at index %d\n",i);
        }
    }
    printf("\n");
    printf("xi_parse_file_update_available { name: \"%s\", revision: \"%s\"}\n",incomingFilename,fileRevision);
    printf("\n");
    /* Let's try making a GET_CHUNK packet */
    printf("File chunk request params:\n");
    int messageType = XI_FILE_GET_CHUNK;
    int messageVersion = 1;
    int offset = 0;
    int messageLength = 1024;
    printf("msgtype: %d\n", messageType);
    printf("msgver: %d\n", messageVersion);
    printf("N: \"%s\"\n", incomingFilename);
    printf("R: \"%s\"\n", fileRevision);
    printf("O: %d\n", offset);
    printf("L: %d\n", messageLength);

#if 1
    cn_cbor_errback err;
    cn_cbor *cb_map = cn_cbor_map_create(&err);

    int cbor_status = XI_CBOR_PARSER_OK;
    cbor_status &= cn_cbor_map_put(cb_map,
                    cn_cbor_string_create("msgtype", &err),
                    cn_cbor_int_create(messageType, &err), &err);

    cbor_status &= cn_cbor_map_put(cb_map,
                    cn_cbor_string_create("msgver", &err),
                    cn_cbor_int_create(messageVersion, &err), &err);

    cbor_status &= cn_cbor_map_put(cb_map,
                    cn_cbor_string_create("N", &err),
                    cn_cbor_string_create(incomingFilename, &err),
                    &err);
    cbor_status &= cn_cbor_map_put(cb_map,
                    cn_cbor_string_create("R", &err),
                    cn_cbor_string_create(fileRevision, &err),
                    &err);
    cbor_status &= cn_cbor_map_put(cb_map,
                    cn_cbor_string_create("O", &err),
                    cn_cbor_int_create(offset, &err), &err);
    cbor_status &= cn_cbor_map_put(cb_map,
                    cn_cbor_string_create("L", &err),
                    cn_cbor_int_create(messageLength, &err), &err);

    printf("cbor status: %d\n", cbor_status );
    enc_sz = cn_cbor_encoder_write(encoded, 0, sizeof(encoded), cb_map);
    cn_cbor_free(cb_map CBOR_CONTEXT_PARAM);
#endif
}

int xi_parse_file_chunk(cn_cbor* in_cb, xi_sft_file_chunk_t* out_sft_file_chunk )
{
    if( NULL == in_cb || NULL == out_sft_file_chunk )
    {
        return -1;
    }

    static ledToggle = 0;
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

    //
    // msg version
    //
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

    //
    // chunk offset
    //
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

    //
    // chunk length
    //
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

    //
    // Status
    //
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

    //
    // Byte Array
    //
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

int total_messages = 0;

void xi_process_file_chunk( cn_cbor *cb )
{
    cn_cbor *cb_item;
    unsigned chunkoffset = 0;
    int retval;

    xi_sft_file_chunk_t file_chunk_data;
    if( 0 != xi_parse_file_chunk( cb, &file_chunk_data ) )
    {
        downloading = 0;
        return;
    }

    printf("---- FILE_CHUNK: length = %d FILE_LENGTH: %d\n", file_chunk_data.chunk_length, filelength);

    // Save the offset for THIS fine chunk
    chunkoffset = offset;

    // Build the offset for the NEXT file chunk
    offset += file_chunk_data.chunk_length;

    if(currenttenpercent < offset)
    {
        /* Yes, I know this is terrible, but it's the level of effort appropriate to the task. */
        printf("Downloaded %d bytes %d %%  of the file \n",offset, (100 * (currenttenpercent + 1000)) / filelength);
        currenttenpercent += tenpercent;
    }

    printf("cb_item->length = %d\r\n", file_chunk_data.array_length);

    if( file_chunk_data.array_length )
    {
        int writelength = file_chunk_data.array_length;
        if( chunkoffset + file_chunk_data.array_length > filelength )
        {
            writelength = filelength - chunkoffset ;
            downloading = 0;
        }

        sha256_update( &ctx, (unsigned char *) file_chunk_data.byte_array, file_chunk_data.array_length );
        retval = sl_extlib_FlcWriteFile( lFileHandle, chunkoffset, (unsigned char *) file_chunk_data.byte_array, file_chunk_data.array_length );
        if( retval < file_chunk_data.array_length )
        {
            downloading = 0;
            //sl_extlib_FlcCloseFile(lFileHandle, NULL, NULL, 0);
        }

        printf("Write returned %d\n",retval);

        if( (chunkoffset + file_chunk_data.array_length ) >= filelength )
        {
            printf("Downloaded is done\n");
            downloading = 0;
        }
    }
    else
    {
        downloading = 0;
        //sl_extlib_FlcCloseFile(lFileHandle, NULL, NULL, 0);
    }


    if(cb_item->v.uint > 1)
    {
        downloading = 0;
        	sl_extlib_FlcCloseFile(lFileHandle, NULL, NULL, 0);
    }

#if 1
    printf("File chunk request params:\n");
    int messageType = XI_FILE_GET_CHUNK;
    int messageVersion = 1;
    int messageLength = 1024;
    printf("msgtype: %d\n", messageType);
    printf("msgver: %d\n", messageVersion);
    printf("N: \"%s\"\n", incomingFilename);
    printf("R: \"%s\"\n", fileRevision);
    printf("O: %d\n", offset);
    printf("L: %d\n", messageLength);

    printf("internal downloading: %d\n", downloading);
    if(1 == downloading) {
		/* Let's try making a GET_CHUNK packet */
        printf("creating get_chunk packet\n");
		cn_cbor_errback err;
		cn_cbor *cb_map = cn_cbor_map_create(&err);

		cn_cbor_map_put(cb_map,
						cn_cbor_string_create("msgtype", &err),
						cn_cbor_int_create(messageType, &err), &err);

		cn_cbor_map_put(cb_map,
						cn_cbor_string_create("msgver", &err),
						cn_cbor_int_create(messageVersion, &err), &err);
		cn_cbor_map_put(cb_map,
						cn_cbor_string_create("N", &err),
						cn_cbor_string_create(incomingFilename, &err),
						&err);
		cn_cbor_map_put(cb_map,
						cn_cbor_string_create("R", &err),
						cn_cbor_string_create(fileRevision, &err),
						&err);
		cn_cbor_map_put(cb_map,
						cn_cbor_string_create("O", &err),
						cn_cbor_int_create(offset, &err), &err);
		cn_cbor_map_put(cb_map,
						cn_cbor_string_create("L", &err),
						cn_cbor_int_create(messageLength, &err), &err);

		enc_sz = cn_cbor_encoder_write(encoded, 0, sizeof(encoded), cb_map);
		cn_cbor_free(cb_map CBOR_CONTEXT_PARAM);
    }
#endif
}

int
xi_sft_init(xi_context_handle_t in_context_handle, char *account_id, char *device_id) {
	int r = -1;

	readBootinfo();
	// Accept the latest firmware if we get here.
	if( sl_extlib_FlcIsPendingCommit() )
	{
	    printf("******************* Committing this revision\n");
	    sl_extlib_FlcCommit(FLC_COMMITED);
	}

	readBootinfo();

	if(NULL == account_id) return(-1);
	if(NULL == device_id) return(-1);

	// xi_stopic is the MQTT topic we PUB to to send messages to the broker.
	snprintf(xi_stopic, sizeof(xi_stopic),"xi/ctrl/v1/%s/svc", device_id);

	// xi_ctopic is the MQTT topic we SUBSCRIBE to to receive messages from the broker.
	snprintf(xi_ctopic, sizeof(xi_ctopic),"xi/ctrl/v1/%s/cln", device_id);

	// xi_logtopic is the MQTT topic we PUB to to add log messages.
	snprintf(xi_logtopic, sizeof(xi_logtopic),"xi/blue/v1/%s/d/%s/_log",account_id, device_id);

	// Everything interesting happens in the on_sft_message() state machine.
	r = xi_subscribe(in_context_handle, xi_ctopic, XI_MQTT_QOS_AT_MOST_ONCE, on_sft_message, 0 );
	printf("xi_subscribe() topic: %s returns %d\n",xi_ctopic, r);

	return(0);
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
