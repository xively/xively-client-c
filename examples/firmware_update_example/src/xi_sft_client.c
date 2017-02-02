/*  Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
    This is part of Xively C library. */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../cbor/cbor.h"
#include "../cbor/cn-cbor/cn-cbor.h"
#include "../sha256/sha256.h"
#include "xi_sft.h"

#ifdef XI_DEBUGSFT
void dump( const cn_cbor* cb, char* out, char** end, int indent );
#endif

///// Test these individually
int filelength                 = 0;
int offset                     = 0;
int downloading                = 0;
int tenpercent                 = 0;
int currenttenpercent          = 0;
char filename[50]              = "OS?";
char installedfilerevision[50] = "0.0";
char filerevision[50]          = "0.0";
unsigned char filefingerprint[32];
unsigned char hash[32];
int idx;
typedef unsigned char uchar;
typedef unsigned int uint;
void* lFileHandle = NULL;
unsigned long ulToken;

SHA256_CTX ctx;

char buffer[1024 * 1024 * 32];

#define MESSAGESIZE 50
char message[MESSAGESIZE];
#define DETAILSSIZE 400
char details[DETAILSSIZE];

unsigned char encoded[1024];
ssize_t enc_sz;
char xi_stopic[128];
char xi_logtopic[128];
char xi_ctopic[128];
char incomingfilename[50];
char test_firmware_filename[50];

void xi_parse_file_chunk( cn_cbor* cb );
void xi_parse_file_update_available( cn_cbor* cb );
void xi_publish_file_info( xi_context_handle_t in_context_handle );

void print_hash( unsigned char( hash )[32] )
{
    int idx;
    for ( idx = 0; idx < 32; idx++ )
        printf( "%02x", hash[idx] );
    printf( "\n" );
}

void on_sft_message( xi_context_handle_t in_context_handle,
                     xi_sub_call_type_t call_type,
                     const xi_sub_call_params_t* const params,
                     xi_state_t state,
                     void* user_data )
{
    ( void )state;
    ( void )user_data;

    cn_cbor* cb = NULL;
    char filefingerprintASCII[70];
    char* bufptr                = NULL;
    int i                       = 0;
    cn_cbor* cb_item            = NULL;
    const uint8_t* payload_data = NULL;
    size_t payload_length       = 0;
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
            xi_publish_file_info( in_context_handle );
            return;
        case XI_SUB_CALL_MESSAGE:
            // printf( "topic:%s. message received.\n", params->message.topic );

            /* get the payload parameter that contains the data that was sent. */
            payload_data   = params->message.temporary_payload_data;
            payload_length = params->message.temporary_payload_data_length;

            printf( "received on %s topic %d bytes\n", params->suback.topic,
                    params->message.temporary_payload_data_length );

            /* Figure out what the packet type is and then call the appropriate parser */
            cb = cn_cbor_decode( payload_data, payload_length, 0 );

            if ( cb )
            {
#ifdef XI_DEBUGSFT
                char* bufend = buffer + sizeof( buffer );
                dump( cb, buffer, &bufend, 0 );
                *bufend = 0;
                printf( "%s\n", buffer );
#endif

                cb_item = cn_cbor_mapget_string( cb, "msgtype" );
                if ( cb_item )
                {
                    // printf("msgtype = %d\n",cb_item->v.uint);
                    switch ( cb_item->v.uint )
                    {
                        case XI_FILE_UPDATE_AVAILABLE:
                            printf( "Got FILE_UPDATE_AVAILABLE\n" );
                            downloading = 1;
                            xi_parse_file_update_available( cb );
                            cn_cbor_free( cb CBOR_CONTEXT_PARAM );
                            xi_publish_data( in_context_handle, xi_stopic, encoded,
                                             enc_sz, XI_MQTT_QOS_AT_MOST_ONCE,
                                             XI_MQTT_RETAIN_FALSE, NULL, NULL );

                            // Log the FILE_UPDATE_AVAILABLE information
                            snprintf( message, sizeof( message ),
                                      "FILE_UPDATE_AVAILABLE" );
                            bufptr = filefingerprintASCII;
                            for ( i = 0; i < 32; i++ )
                            {
                                bufptr += sprintf( bufptr, "%02x", filefingerprint[i] );
                            }
                            *bufptr = '\0';
                            snprintf( message, sizeof( message ),
                                      "File update available" );
                            snprintf( details, sizeof( details ),
                                      "name=%s revision=%s length = %d fingerprint = %s",
                                      filename, filerevision, filelength,
                                      filefingerprintASCII );
                            snprintf( buffer, sizeof( buffer ), "{\"message\":\"%s\","
                                                                "\"severity\":\"notice\","
                                                                "\"details\":\"%s\"}",
                                      message, details );
                            xi_publish( in_context_handle, xi_logtopic, buffer,
                                        XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE,
                                        NULL, NULL );
                            sha256_init( &ctx );
                            offset = 0;
                            retVal = openFileForWrite( test_firmware_filename, filelength,
                                                       &lFileHandle );
                            printf( "%s open returned %d\n", incomingfilename, retVal );
                            if ( retVal < 0 )
                                downloading = 0;
                            break;
                        case XI_FILE_CHUNK:
                            printf( "   Got FILE_CHUNK, publish FILE_GET_CHUNK\n" );
                            printf( "o=%d\n", offset );
                            xi_parse_file_chunk( cb );
                            cn_cbor_free( cb CBOR_CONTEXT_PARAM );
                            if ( 1 == downloading )
                            {
                                if ( offset < filelength )
                                {
                                    xi_publish_data( in_context_handle, xi_stopic,
                                                     encoded, enc_sz,
                                                     XI_MQTT_QOS_AT_MOST_ONCE,
                                                     XI_MQTT_RETAIN_FALSE, NULL, NULL );
                                }
                                else
                                {
                                    downloading = 0;
                                    sprintf( installedfilerevision, "%s", filerevision );
                                    printf( "Done downloading. Offset = %d\n", offset );
                                    // Log the download complete information
                                    snprintf( message, sizeof( message ),
                                              "File download complete" );
                                    snprintf( details, sizeof( details ),
                                              "name=%s revision=%s length = %d", filename,
                                              installedfilerevision, filelength );
                                    snprintf( buffer, sizeof( buffer ),
                                              "{\"message\":\"%s\",\"severity\":"
                                              "\"notice\",\"details\":\"%s\"}",
                                              message, details );
                                    xi_publish( in_context_handle, xi_logtopic, buffer,
                                                XI_MQTT_QOS_AT_MOST_ONCE,
                                                XI_MQTT_RETAIN_FALSE, NULL, NULL );
                                    sha256_final( &ctx, hash );
                                    printf( "Calculated hash = 0x" );
                                    print_hash( hash );
                                    offset = 0;
                                    xi_publish_file_info( in_context_handle );
                                    closeFile( &lFileHandle );
                                    printf( "*** Setting image to TEST \r\n" );
                                    testFirmware();
                                    rebootDevice();
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

void xi_publish_file_info( xi_context_handle_t in_context_handle )
{
    unsigned char encoded[512];
    ssize_t enc_sz;

    // xi_parse_file_update_available(NULL);

    /* Let's try making a XI_FILE_INFO packet */
    cn_cbor_errback err;
    cn_cbor* cb_map = cn_cbor_map_create( &err );

    cn_cbor_map_put( cb_map, cn_cbor_string_create( "msgtype", &err ),
                     cn_cbor_int_create( XI_FILE_INFO, &err ), &err );

    cn_cbor_map_put( cb_map, cn_cbor_string_create( "msgver", &err ),
                     cn_cbor_int_create( 1, &err ), &err );


    cn_cbor* a        = cn_cbor_array_create( &err );
    cn_cbor* cb_file1 = cn_cbor_map_create( &err );

    cn_cbor_map_put( cb_file1, cn_cbor_string_create( "N", &err ),
                     cn_cbor_string_create( filename, &err ), &err );
    cn_cbor_map_put( cb_file1, cn_cbor_string_create( "R", &err ),
                     cn_cbor_string_create( "-1", &err ), &err );

    cn_cbor_array_append( a, cb_file1, &err );

    cn_cbor_map_put( cb_map, cn_cbor_string_create( "list", &err ), a, &err );

    enc_sz = cn_cbor_encoder_write( encoded, 0, sizeof( encoded ), cb_map );

    cn_cbor_free( cb_map CBOR_CONTEXT_PARAM );

    /* publish the FILE_INFO message */
    xi_publish_data( in_context_handle, xi_stopic, encoded, enc_sz,
                     XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
    printf( "Published FILE_INFO to %s\n", xi_stopic );
    snprintf( message, sizeof( message ), "File info" );
    snprintf( details, sizeof( details ), "name=%s revision=%s", filename,
              installedfilerevision );
    snprintf( buffer, sizeof( buffer ),
              "{\"message\":\"%s\",\"severity\":\"notice\",\"details\":\"%s\"}", message,
              details );
    xi_publish( in_context_handle, xi_logtopic, buffer, XI_MQTT_QOS_AT_MOST_ONCE,
                XI_MQTT_RETAIN_FALSE, NULL, NULL );
    downloading = 0;
    closeFile( &lFileHandle );
}

void xi_parse_file_update_available( cn_cbor* cb )
{
    cn_cbor* cb_list;
    cn_cbor* cb_item;
    cn_cbor* cb_file;
    int filelistlength = 0;
    int i, index;

    cb_item = cn_cbor_mapget_string( cb, "msgver" );
    if ( cb_item )
    {
        // printf("FILE_UPDATE_AVAILABLE: msgver = %d\n",cb_item->v.uint);
        if ( 1 != cb_item->v.uint )
            return;
    }
    cb_list = cn_cbor_mapget_string( cb, "list" );
    if ( cb_list )
    {
        filelistlength = cb_list->length;
        printf( "FILE_UPDATE_AVAILABLE: list length = %d\n", filelistlength );
    }

    for ( index = 0; index < filelistlength; index++ )
    {
        cb_item = cn_cbor_index( cb_list, index );

        if ( cb_item )
        {
            // File 'N'ame
            cb_file = cn_cbor_mapget_string( cb_item, "N" );
            if ( cb_file )
            {
                strncpy( incomingfilename, cb_file->v.str, sizeof( incomingfilename ) );
                if ( ( unsigned int )cb_file->length < sizeof( incomingfilename ) )
                    incomingfilename[cb_file->length]            = '\0';
                incomingfilename[sizeof( incomingfilename ) - 1] = '\0';
                printf( "Incomingfilename %s\n", incomingfilename );
            }
            else
            {
                printf( "No key called N\n" );
            }

            // File 'R'evision
            cb_file = cn_cbor_mapget_string( cb_item, "R" );
            if ( cb_file )
            {
                strncpy( filerevision, cb_file->v.str, sizeof( filerevision ) );
                if ( ( unsigned int )cb_file->length < sizeof( filerevision ) )
                    filerevision[cb_file->length]        = '\0';
                filerevision[sizeof( filerevision ) - 1] = '\0';
                printf( "Filerevision %s\n", filerevision );
            }
            else
            {
                printf( "No key called R\n" );
            }

            // File 'S'ize
            cb_file = cn_cbor_mapget_string( cb_item, "S" );
            if ( cb_file )
            {
                printf( "FILE_UPDATE_AVAILABLE: file length = %d\n", cb_file->v.sint );
                filelength        = cb_file->v.sint;
                tenpercent        = filelength / 10;
                currenttenpercent = tenpercent;
                printf( "Filelength %d\n", filelength );
            }
            else
            {
                printf( "No key called S\n" );
            }

            // File 'F'ingerprint
            cb_file = cn_cbor_mapget_string( cb_item, "F" );
            if ( cb_file )
            {
                for ( i                = 0; i < 32; i++ )
                    filefingerprint[i] = cb_file->v.str[i];
                printf( "FILE_UPDATE_AVAILABLE: file fingerprint = 0x" );
                for ( i = 0; i < 32; i++ )
                    printf( "%02x", filefingerprint[i] );
                printf( "\n" );
            }
            else
            {
                printf( "No key called F\n" );
            }
        }
        else
        {
            printf( "No item at index %d\n", i );
        }
    }
    printf( "->%s<- ->%s<-\n", filename, filerevision );
    /* Let's try making a GET_CHUNK packet */
    cn_cbor_errback err;
    cn_cbor* cb_map = cn_cbor_map_create( &err );

    cn_cbor_map_put( cb_map, cn_cbor_string_create( "msgtype", &err ),
                     cn_cbor_int_create( XI_FILE_GET_CHUNK, &err ), &err );

    cn_cbor_map_put( cb_map, cn_cbor_string_create( "msgver", &err ),
                     cn_cbor_int_create( 1, &err ), &err );
    cn_cbor_map_put( cb_map, cn_cbor_string_create( "N", &err ),
                     cn_cbor_string_create( incomingfilename, &err ), &err );
    cn_cbor_map_put( cb_map, cn_cbor_string_create( "R", &err ),
                     cn_cbor_string_create( filerevision, &err ), &err );
    cn_cbor_map_put( cb_map, cn_cbor_string_create( "O", &err ),
                     cn_cbor_int_create( 0, &err ), &err );
    cn_cbor_map_put( cb_map, cn_cbor_string_create( "L", &err ),
                     cn_cbor_int_create( 2048, &err ), &err );

    enc_sz = cn_cbor_encoder_write( encoded, 0, sizeof( encoded ), cb_map );
    cn_cbor_free( cb_map CBOR_CONTEXT_PARAM );
}

int total_messages = 0;

void xi_parse_file_chunk( cn_cbor* cb )
{
    cn_cbor* cb_item;
    unsigned chunkoffset = 0;
    int retval;

    cb_item = cn_cbor_mapget_string( cb, "msgver" );
    if ( cb_item )
    {
        printf( "FILE_CHUNK: msgver = %d\n", cb_item->v.uint );
    }

    cb_item = cn_cbor_mapget_string( cb, "L" );
    if ( cb_item )
    {
        printf( "FILE_CHUNK: length = %d\n", cb_item->v.uint );
        // Save the offset for THIS fine chunk
        chunkoffset = offset;

        // Build the offset for the NEXT file chunk
        offset += cb_item->v.uint;

        if ( currenttenpercent < offset )
        {
            /* Yes, I know this is terrible, but it's the level of effort appropriate to
             * the task. */
            printf( "Downloaded %d bytes %d %%  of the file \n", offset,
                    ( 100 * ( currenttenpercent + 1000 ) ) / filelength );
            currenttenpercent += tenpercent;
        }
    }
    else
    {
        printf( "No key called L\n" );
    }

    cb_item = cn_cbor_mapget_string( cb, "C" );

    if ( cb_item )
    {
        printf( "cb_item->length = %d\r\n", cb_item->length );
        if ( cb_item->length > 0 )
        {
            sha256_update( &ctx, ( BYTE* )cb_item->v.str, cb_item->length );

            printf( "lFileHandle = %p\n", lFileHandle );

            retval = writeChunk( lFileHandle, chunkoffset,
                                 ( const unsigned char* const )cb_item->v.str,
                                 cb_item->length );
            if ( retval < cb_item->length )
            {
                downloading = 0;
                closeFile( &lFileHandle );
            }
            printf( "Write returned %d\n", retval );
        }
        else
        {
            printf( "no cb_item\n" );
            downloading = 0;
            closeFile( &lFileHandle );
        }
    }
    else
    {
        printf( "No key called C\n" );
    }

    cb_item = cn_cbor_mapget_string( cb, "S" );

    if ( cb_item )
    {
        printf( "FILE_CHUNK: status = %d\n", cb_item->v.uint );

        if ( cb_item->v.uint > 1 )
        {
            downloading = 0;
            closeFile( lFileHandle );
        }
    }
    else
    {
        printf( "No key called S\n" );
    }

    if ( 1 == downloading )
    {
        /* Let's try making a GET_CHUNK packet */
        cn_cbor_errback err;
        cn_cbor* cb_map = cn_cbor_map_create( &err );

        cn_cbor_map_put( cb_map, cn_cbor_string_create( "msgtype", &err ),
                         cn_cbor_int_create( XI_FILE_GET_CHUNK, &err ), &err );

        cn_cbor_map_put( cb_map, cn_cbor_string_create( "msgver", &err ),
                         cn_cbor_int_create( 1, &err ), &err );
        cn_cbor_map_put( cb_map, cn_cbor_string_create( "N", &err ),
                         cn_cbor_string_create( incomingfilename, &err ), &err );
        cn_cbor_map_put( cb_map, cn_cbor_string_create( "R", &err ),
                         cn_cbor_string_create( filerevision, &err ), &err );
        cn_cbor_map_put( cb_map, cn_cbor_string_create( "O", &err ),
                         cn_cbor_int_create( offset, &err ), &err );
        cn_cbor_map_put( cb_map, cn_cbor_string_create( "L", &err ),
                         cn_cbor_int_create( 2048, &err ), &err );

        enc_sz = cn_cbor_encoder_write( encoded, 0, sizeof( encoded ), cb_map );
        cn_cbor_free( cb_map CBOR_CONTEXT_PARAM );
    }
}

int xi_sft_init( xi_context_handle_t in_context_handle,
                 const char* const account_id,
                 const char* const device_id,
                 const char* const test_firmware_file )
{
    int r = -1;

    printf( "******************* Committing this revision\n" );
    // Accept the latest firmware if we get here.
    commitFirmware( XI_FIRMWARE_COMMITED );

    if ( NULL == account_id )
        return ( -1 );
    if ( NULL == device_id )
        return ( -1 );

    // xi_stopic is the MQTT topic we PUB to to send messages to the broker.
    if ( snprintf( xi_stopic, sizeof( xi_stopic ), "xi/ctrl/v1/%s/svc", device_id ) >=
         ( int )sizeof( xi_stopic ) )
    {
        return -1; // truncation
    }

    // xi_ctopic is the MQTT topic we SUBSCRIBE to to receive messages from the broker.
    if ( snprintf( xi_ctopic, sizeof( xi_ctopic ), "xi/ctrl/v1/%s/cln", device_id ) >=
         ( int )sizeof( xi_ctopic ) )
    {
        return -1; // truncation
    }

    // xi_logtopic is the MQTT topic we PUB to to add log messages.
    if ( snprintf( xi_logtopic, sizeof( xi_logtopic ), "xi/blue/v1/%s/d/%s/_log",
                   account_id, device_id ) >= ( int )sizeof( xi_logtopic ) )
    {
        return -1;
    }

    if ( snprintf( test_firmware_filename, sizeof( test_firmware_filename ), "%s",
                   test_firmware_file ) >= ( int )sizeof( test_firmware_filename ) )
    {
        return -1;
    }

    // Everything interesting happens in the on_sft_message() state machine.
    r = xi_subscribe( in_context_handle, xi_ctopic, XI_MQTT_QOS_AT_MOST_ONCE,
                      on_sft_message, 0 );
    printf( "xi_subscribe() topic: %s returns %d\n", xi_ctopic, r );

    return ( 0 );
}


// Functions useful for debugging:
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
                PRF( "%02x ", ( cb->v.str[i] & 0xff ) );
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
