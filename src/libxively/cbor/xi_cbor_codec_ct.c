/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_cbor_codec_ct.h>
#include <xi_cbor_codec_ct_keys.h>

#include <cbor.h>
#include <cn-cbor/cn-cbor.h>

#include <xi_control_message.h>
#include <xively_error.h>
#include <xi_macros.h>

#include <xi_debug.h>

#ifdef USE_CBOR_CONTEXT

#include <xi_allocator.h>

void* xi_cn_calloc_func_wrapper( size_t count, size_t size, void* context )
{
    ( void )context;

    return xi_calloc( count, size );
}

void xi_cn_free_func_wrapper( void* ptr, void* context )
{
    ( void )context;

    xi_free( ptr );
}

static cn_cbor_context cn_cbor_context_object = {.calloc_func = xi_cn_calloc_func_wrapper,
                                                 .free_func   = xi_cn_free_func_wrapper,
                                                 .context     = NULL};

cn_cbor_context* context_cbor = &cn_cbor_context_object;

#endif

void xi_cbor_put_name_and_revision( cn_cbor* cb_map,
                                    const char* name,
                                    const char* revision,
                                    cn_cbor_errback* errp )
{
    if ( NULL != name )
    {
        cn_cbor_map_put( cb_map,
                         cn_cbor_string_create(
                             XI_CBOR_CODEC_CT_STRING_FILE_NAME CBOR_CONTEXT_PARAM, errp ),
                         cn_cbor_string_create( name CBOR_CONTEXT_PARAM, errp ), errp );
    }

    if ( NULL != revision )
    {
        cn_cbor_map_put(
            cb_map, cn_cbor_string_create(
                        XI_CBOR_CODEC_CT_STRING_FILE_REVISION CBOR_CONTEXT_PARAM, errp ),
            cn_cbor_string_create( revision CBOR_CONTEXT_PARAM, errp ), errp );
    }
}

void xi_cbor_codec_ct_encode_generate_buffer( cn_cbor* cb_map,
                                              uint8_t** out_encoded_allocated_inside,
                                              uint32_t* out_len,
                                              uint32_t buffer_size_min,
                                              uint32_t buffer_size_max )
{
    /* This function generates CBOR encoded message buffer. Starts at a minimum buffer
     * size and doubles it until message fits into the buffer or the maximum allowed size
     * is reached. At the end output binary is trimmed at exactly required size. */

    xi_state_t state = XI_STATE_OK;

    uint8_t* encoded     = NULL;
    uint32_t encoded_len = buffer_size_min;

    /* doubling buffer size until it fits or ceiling is reached */
    for ( ; encoded_len < buffer_size_max; encoded_len *= 2 )
    {
        XI_ALLOC_BUFFER_AT( uint8_t, encoded, encoded_len, state );

        const ssize_t encode_result =
            cn_cbor_encoder_write( encoded, 0, encoded_len, cb_map );

        if ( encode_result <= 0 )
        {
            /* failure during encoding, probably buffer size is not enough */
            XI_SAFE_FREE( encoded );
        }
        else
        {
            /* successful encoding */
            *out_len = encode_result;
            break;
        }
    }

    cn_cbor_free( cb_map CBOR_CONTEXT_PARAM );

    if ( 0 < *out_len )
    {
        XI_ALLOC_BUFFER_AT( uint8_t, *out_encoded_allocated_inside, *out_len, state );

        memcpy( *out_encoded_allocated_inside, encoded, *out_len );
    }
    XI_SAFE_FREE( encoded );

    return;

err_handling:;

    XI_SAFE_FREE( encoded );
    XI_SAFE_FREE( *out_encoded_allocated_inside );
}

void xi_cbor_codec_ct_encode( const xi_control_message_t* control_message,
                              uint8_t** out_encoded_allocated_inside,
                              uint32_t* out_len )
{
    if ( NULL == control_message )
    {
        *out_len = 0;
        return;
    }

    cn_cbor_errback err;
    cn_cbor* cb_map = cn_cbor_map_create( CBOR_CONTEXT_PARAM_COMMA & err );

    cn_cbor_map_put(
        cb_map,
        cn_cbor_string_create( XI_CBOR_CODEC_CT_STRING_MSGTYPE CBOR_CONTEXT_PARAM, &err ),
        cn_cbor_int_create( control_message->common.msgtype CBOR_CONTEXT_PARAM, &err ),
        &err );

    cn_cbor_map_put(
        cb_map,
        cn_cbor_string_create( XI_CBOR_CODEC_CT_STRING_MSGVER CBOR_CONTEXT_PARAM, &err ),
        cn_cbor_int_create( control_message->common.msgver CBOR_CONTEXT_PARAM, &err ),
        &err );

    switch ( control_message->common.msgtype )
    {
        case XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO:

            if ( 0 < control_message->file_info.list_len )
            {
                cn_cbor* files = cn_cbor_array_create( CBOR_CONTEXT_PARAM_COMMA & err );

                uint16_t id_file = 0;
                for ( ; id_file < control_message->file_info.list_len; ++id_file )
                {
                    cn_cbor* file = cn_cbor_map_create( CBOR_CONTEXT_PARAM_COMMA & err );

                    xi_cbor_put_name_and_revision(
                        file, control_message->file_info.list[id_file].name,
                        control_message->file_info.list[id_file].revision, &err );

                    cn_cbor_array_append( files, file, &err );
                }

                cn_cbor_map_put(
                    cb_map, cn_cbor_string_create(
                                XI_CBOR_CODEC_CT_STRING_LIST CBOR_CONTEXT_PARAM, &err ),
                    files, &err );

                cn_cbor* cn_cbor_bool = CN_CALLOC_CONTEXT();
                if ( NULL != cn_cbor_bool )
                {
                    cn_cbor_bool->type =
                        ( 0 != control_message->file_info.flag_accept_download_link )
                            ? CN_CBOR_TRUE
                            : CN_CBOR_FALSE;

                    cn_cbor_map_put(
                        cb_map,
                        cn_cbor_string_create(
                            XI_CBOR_CODEC_CT_STRING_FILE_DOWNLOADLINK CBOR_CONTEXT_PARAM,
                            &err ),
                        cn_cbor_bool, &err );
                }
            }
            break;

        case XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK:

            xi_cbor_put_name_and_revision( cb_map, control_message->file_get_chunk.name,
                                           control_message->file_get_chunk.revision,
                                           &err );

            cn_cbor_map_put(
                cb_map,
                cn_cbor_string_create(
                    XI_CBOR_CODEC_CT_STRING_FILECHUNK_OFFSET CBOR_CONTEXT_PARAM, &err ),
                cn_cbor_int_create(
                    control_message->file_get_chunk.offset CBOR_CONTEXT_PARAM, &err ),
                &err );

            cn_cbor_map_put(
                cb_map,
                cn_cbor_string_create(
                    XI_CBOR_CODEC_CT_STRING_FILECHUNK_LENGTH CBOR_CONTEXT_PARAM, &err ),
                cn_cbor_int_create(
                    control_message->file_get_chunk.length CBOR_CONTEXT_PARAM, &err ),
                &err );

            break;

        case XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS:

            xi_cbor_put_name_and_revision( cb_map, control_message->file_status.name,
                                           control_message->file_status.revision, &err );

            cn_cbor_map_put(
                cb_map,
                cn_cbor_string_create(
                    XI_CBOR_CODEC_CT_STRING_FILESTATUS_PHASE CBOR_CONTEXT_PARAM, &err ),
                cn_cbor_int_create( control_message->file_status.phase CBOR_CONTEXT_PARAM,
                                    &err ),
                &err );

            cn_cbor_map_put(
                cb_map,
                cn_cbor_string_create(
                    XI_CBOR_CODEC_CT_STRING_FILESTATUS_CODE CBOR_CONTEXT_PARAM, &err ),
                cn_cbor_int_create( control_message->file_status.code CBOR_CONTEXT_PARAM,
                                    &err ),
                &err );

            break;

        /* the followings are encoded by the broker and decoded by the client */
        case XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE:
        case XI_CONTROL_MESSAGE_SC__SFT_FILE_CHUNK:
        default:

            xi_debug_format(
                "WARNING: CBOR encoder was called with server to client message type %d",
                control_message->common.msgtype );
    }

    xi_cbor_codec_ct_encode_generate_buffer( cb_map, out_encoded_allocated_inside,
                                             out_len, XI_CBOR_MESSAGE_MIN_BUFFER_SIZE,
                                             XI_CBOR_MESSAGE_MAX_BUFFER_SIZE );
}

xi_state_t xi_cbor_codec_ct_decode_getvalue( cn_cbor* source,
                                             const char* key,
                                             void* out_destination,
                                             uint16_t* out_len )
{
    xi_state_t state = XI_INVALID_PARAMETER;

    if ( NULL == source )
    {
        return state;
    }

    cn_cbor* source_value = cn_cbor_mapget_string( source, key );

    if ( NULL != source_value )
    {
        // xi_debug_printf( "[ CBOR ] type: %d,\n", source_value->type );

        state = XI_STATE_OK;

        switch ( source_value->type )
        {
            case CN_CBOR_FALSE:
                *( ( uint32_t* )out_destination ) = 0;
                break;
            case CN_CBOR_TRUE:
                *( ( uint32_t* )out_destination ) = 1;
                break;
            case CN_CBOR_NULL:
            case CN_CBOR_UNDEF:
            case CN_CBOR_UINT:
            case CN_CBOR_INT:
            case CN_CBOR_TAG:
            case CN_CBOR_SIMPLE:
            case CN_CBOR_DOUBLE:

                // xi_debug_printf( "source_value: %lu\n", source_value->v.uint );

                *( ( uint32_t* )out_destination ) = source_value->v.uint;

                break;

            case CN_CBOR_BYTES:

                if ( 0 == source_value->length )
                {
                    break;
                }
                // xi_debug_printf( "CN_CBOR_BYTES / source_value: %s, length: %d\n",
                //                  ( char* )source_value->v.bytes, source_value->length
                //                  );

                XI_ALLOC_BUFFER_AT( uint8_t, *( uint8_t** )out_destination,
                                    source_value->length, state );

                memcpy( *( uint8_t** )out_destination, source_value->v.bytes,
                        source_value->length );

                if ( NULL != out_len )
                {
                    *out_len = source_value->length;
                }

                break;

            case CN_CBOR_TEXT:

                // xi_debug_printf( "source_value: %s, length: %d\n", source_value->v.str,
                //                  source_value->length );

                XI_ALLOC_BUFFER_AT( char, *( char** )out_destination,
                                    source_value->length + 1, state );

                memcpy( *( char** )out_destination, source_value->v.str,
                        source_value->length );

                break;

            case CN_CBOR_BYTES_CHUNKED:
            case CN_CBOR_TEXT_CHUNKED:
            case CN_CBOR_ARRAY:
            case CN_CBOR_MAP:
            case CN_CBOR_INVALID:
                state = XI_NOT_IMPLEMENTED;
                break;
        }
    }
    else
    {
        xi_debug_printf( "[ CBOR ] element not found for key: '%s'\n", key );
        state = XI_ELEMENT_NOT_FOUND;
    }

err_handling:

    return state;
}

xi_control_message_t* xi_cbor_codec_ct_decode( const uint8_t* data, const uint32_t len )
{
    cn_cbor_errback err;
    cn_cbor* cb_map = cn_cbor_decode( data, len CBOR_CONTEXT_PARAM, &err );

    xi_control_message_t* control_message_out = NULL;

    xi_state_t state = XI_STATE_OK;
    cn_cbor* msgtype = NULL;
    cn_cbor* msgver  = NULL;

    XI_CHECK_CND_DBGMESSAGE( NULL == cb_map, XI_ELEMENT_NOT_FOUND, state,
                             "ERROR: data is not a CBOR binary" );

    XI_ALLOC_AT( xi_control_message_t, control_message_out, state );

    msgtype = cn_cbor_mapget_string( cb_map, XI_CBOR_CODEC_CT_STRING_MSGTYPE );
    XI_CHECK_CND_DBGMESSAGE( NULL == msgtype, XI_INVALID_PARAMETER, state,
                             "ERROR: no 'msgtype' found" );
    control_message_out->common.msgtype = ( xi_control_message_type_t )msgtype->v.uint;


    msgver = cn_cbor_mapget_string( cb_map, XI_CBOR_CODEC_CT_STRING_MSGVER );
    XI_CHECK_CND_DBGMESSAGE( NULL == msgver, XI_INVALID_PARAMETER, state,
                             "ERROR: no 'msgver' found" );
    control_message_out->common.msgver = msgver->v.uint;


    switch ( ( xi_control_message_type_t )msgtype->v.uint )
    {
        case XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE:
        {
            cn_cbor* list = cn_cbor_mapget_string( cb_map, XI_CBOR_CODEC_CT_STRING_LIST );

            if ( NULL != list )
            {
                control_message_out->file_update_available.list_len = list->length;

                if ( 0 < list->length )
                {
                    XI_ALLOC_BUFFER_AT( xi_control_message_file_desc_ext_t,
                                        control_message_out->file_update_available.list,
                                        sizeof( xi_control_message_file_desc_ext_t ) *
                                            list->length,
                                        state );

                    uint16_t id_file = 0;
                    for ( ; id_file < list->length; ++id_file )
                    {
                        cn_cbor* file = cn_cbor_index( list, id_file );

                        xi_cbor_codec_ct_decode_getvalue(
                            file, XI_CBOR_CODEC_CT_STRING_FILE_NAME,
                            &control_message_out->file_update_available.list[id_file]
                                 .name,
                            NULL );

                        xi_cbor_codec_ct_decode_getvalue(
                            file, XI_CBOR_CODEC_CT_STRING_FILE_REVISION,
                            &control_message_out->file_update_available.list[id_file]
                                 .revision,
                            NULL );

                        xi_cbor_codec_ct_decode_getvalue(
                            file, XI_CBOR_CODEC_CT_STRING_FILE_OPERATION,
                            &control_message_out->file_update_available.list[id_file]
                                 .file_operation,
                            NULL );

                        xi_cbor_codec_ct_decode_getvalue(
                            file, XI_CBOR_CODEC_CT_STRING_FILE_IMAGESIZE,
                            &control_message_out->file_update_available.list[id_file]
                                 .size_in_bytes,
                            NULL );

                        xi_cbor_codec_ct_decode_getvalue(
                            file, XI_CBOR_CODEC_CT_STRING_FILE_FINGERPRINT,
                            &control_message_out->file_update_available.list[id_file]
                                 .fingerprint,
                            &control_message_out->file_update_available.list[id_file]
                                 .fingerprint_len );

                        xi_cbor_codec_ct_decode_getvalue(
                            file, XI_CBOR_CODEC_CT_STRING_FILE_DOWNLOADLINK,
                            &control_message_out->file_update_available.list[id_file]
                                 .download_link,
                            NULL );

                        xi_cbor_codec_ct_decode_getvalue(
                            file, XI_CBOR_CODEC_CT_STRING_FILE_MQTT_DL_SUPPORTED,
                            &control_message_out->file_update_available.list[id_file]
                                 .flag_mqtt_download_also_supported,
                            NULL );
                    }
                }
            }
        }

        break;

        case XI_CONTROL_MESSAGE_SC__SFT_FILE_CHUNK:

            xi_cbor_codec_ct_decode_getvalue( cb_map, XI_CBOR_CODEC_CT_STRING_FILE_NAME,
                                              &control_message_out->file_chunk.name,
                                              NULL );

            xi_cbor_codec_ct_decode_getvalue(
                cb_map, XI_CBOR_CODEC_CT_STRING_FILE_REVISION,
                &control_message_out->file_chunk.revision, NULL );

            xi_cbor_codec_ct_decode_getvalue(
                cb_map, XI_CBOR_CODEC_CT_STRING_FILECHUNK_OFFSET,
                &control_message_out->file_chunk.offset, NULL );

            /*  This 'length' field is redundant since the CBOR encoding contains
                the length of the byte array right in the array. So here we don't
                rely on protocol's length field but the CBOR's array size.
                Thus ignoring the 'length' field itself.

            xi_cbor_codec_ct_decode_getvalue(
                cb_map, XI_CBOR_CODEC_CT_STRING_FILECHUNK_LENGTH,
                &control_message_out->file_chunk.length, NULL );*/

            xi_cbor_codec_ct_decode_getvalue(
                cb_map, XI_CBOR_CODEC_CT_STRING_FILECHUNK_STATUS,
                &control_message_out->file_chunk.status, NULL );

            xi_cbor_codec_ct_decode_getvalue(
                cb_map, XI_CBOR_CODEC_CT_STRING_FILECHUNK_CHUNK,
                &control_message_out->file_chunk.chunk,
                ( uint16_t* )&control_message_out->file_chunk.length );

            break;

        case XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO:
        case XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK:
        case XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS:
        default:

            xi_debug_format(
                "WARNING: CBOR decoder was called with client to server message type %lu",
                msgtype->v.uint );
    }

    cn_cbor_free( cb_map CBOR_CONTEXT_PARAM );
    return control_message_out;

err_handling:

    cn_cbor_free( cb_map CBOR_CONTEXT_PARAM );
    XI_SAFE_FREE( control_message_out );

    return control_message_out;
}
