/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_cbor_codec_ct.h>

#include <cbor.h>
#include <cn-cbor/cn-cbor.h>

#include <xively_error.h>
#include <xi_macros.h>

#define XI_CBOR_CODEC_CT_STRING_MSGTYPE "msgtype"
#define XI_CBOR_CODEC_CT_STRING_MSGVER "msgver"
#define XI_CBOR_CODEC_CT_STRING_LIST "list"
#define XI_CBOR_CODEC_CT_STRING_FILE_NAME "N"
#define XI_CBOR_CODEC_CT_STRING_FILE_REVISION "R"
#define XI_CBOR_CODEC_CT_STRING_FILE_OPERATION "O"
#define XI_CBOR_CODEC_CT_STRING_FILE_IMAGESIZE "S"
#define XI_CBOR_CODEC_CT_STRING_FILE_FINGERPRINT "F"


void xi_cbor_put_name_and_revision( cn_cbor* cb_map,
                                    const char* name,
                                    const char* revision,
                                    cn_cbor_errback* errp )
{
    if ( NULL != name )
    {
        cn_cbor_map_put( cb_map,
                         cn_cbor_string_create( XI_CBOR_CODEC_CT_STRING_FILE_NAME, errp ),
                         cn_cbor_string_create( name, errp ), errp );
    }

    if ( NULL != revision )
    {
        cn_cbor_map_put(
            cb_map, cn_cbor_string_create( XI_CBOR_CODEC_CT_STRING_FILE_REVISION, errp ),
            cn_cbor_string_create( revision, errp ), errp );
    }
}

void xi_cbor_codec_ct_encode( const xi_control_message_t* control_message,
                              uint8_t** out_encoded_allocated_inside,
                              uint32_t* out_len )
{
    cn_cbor_errback err;
    cn_cbor* cb_map = cn_cbor_map_create( &err );

    cn_cbor_map_put( cb_map,
                     cn_cbor_string_create( XI_CBOR_CODEC_CT_STRING_MSGTYPE, &err ),
                     cn_cbor_int_create( control_message->common.msgtype, &err ), &err );

    cn_cbor_map_put( cb_map,
                     cn_cbor_string_create( XI_CBOR_CODEC_CT_STRING_MSGVER, &err ),
                     cn_cbor_int_create( control_message->common.msgver, &err ), &err );

    switch ( control_message->common.msgtype )
    {
        case XI_CONTROL_MESSAGE_DB_FILE_INFO:

            if ( 0 < control_message->file_info.list_len )
            {
                cn_cbor* files = cn_cbor_array_create( &err );

                uint16_t id_file = 0;
                for ( ; id_file < control_message->file_info.list_len; ++id_file )
                {
                    cn_cbor* file = cn_cbor_map_create( &err );

                    xi_cbor_put_name_and_revision(
                        file, control_message->file_info.list[id_file].name,
                        control_message->file_info.list[id_file].revision, &err );

                    cn_cbor_array_append( files, file, &err );
                }

                cn_cbor_map_put(
                    cb_map, cn_cbor_string_create( XI_CBOR_CODEC_CT_STRING_LIST, &err ),
                    files, &err );
            }
            break;

        case XI_CONTROL_MESSAGE_DB_FILE_GET_CHUNK:

            xi_cbor_put_name_and_revision( cb_map, control_message->file_get_chunk.name,
                                           control_message->file_get_chunk.revision,
                                           &err );

            cn_cbor_map_put(
                cb_map, cn_cbor_string_create( "O", &err ),
                cn_cbor_int_create( control_message->file_get_chunk.offset, &err ),
                &err );

            cn_cbor_map_put(
                cb_map, cn_cbor_string_create( "L", &err ),
                cn_cbor_int_create( control_message->file_get_chunk.length, &err ),
                &err );

            break;

        case XI_CONTROL_MESSAGE_DB_FILE_STATUS:

            break;

        /* the followings are encoded by the broker and decoded by the client */
        case XI_CONTROL_MESSAGE_BD_FILE_UPDATE_AVAILABLE:
        case XI_CONTROL_MESSAGE_BD_FILE_CHUNK:
        default:
            cn_cbor_free( cb_map CBOR_CONTEXT_PARAM );
            return;
    }

    unsigned char encoded[512];
    *out_len = cn_cbor_encoder_write( encoded, 0, sizeof( encoded ), cb_map );

    cn_cbor_free( cb_map CBOR_CONTEXT_PARAM );

    xi_state_t state = XI_STATE_OK;
    XI_ALLOC_BUFFER_AT( uint8_t, *out_encoded_allocated_inside, *out_len, state );

    memcpy( *out_encoded_allocated_inside, encoded, *out_len );

err_handling:;
}

#if 0
#include <xi_debug.h>
#else
#define xi_debug_printf( ... )
#endif

xi_state_t
xi_cbor_codec_ct_decode_getstring( cn_cbor* source, const char* key, void* destination )
{
    xi_state_t state = XI_INVALID_PARAMETER;

    if ( NULL == source )
    {
        return state;
    }

    cn_cbor* source_value = cn_cbor_mapget_string( source, key );

    if ( NULL != source_value )
    {
        xi_debug_printf( "type: %d, ", source_value->type );

        switch ( source_value->type )
        {
            case CN_CBOR_UINT:

                xi_debug_printf( "source_value: %lu\n", source_value->v.uint );

                *( ( uint32_t* )destination ) = source_value->v.uint;

                break;

            case CN_CBOR_TEXT:

                xi_debug_printf( "source_value: %s, length: %d\n", source_value->v.str,
                                 source_value->length );

                XI_ALLOC_BUFFER_AT( char, *( char** )destination,
                                    source_value->length + 1, state );
                memcpy( *( char** )destination, source_value->v.str,
                        source_value->length );

                state = XI_STATE_OK;
                break;

            default:
                break;
        }
    }
    else
    {
        state = XI_ELEMENT_NOT_FOUND;
    }

err_handling:

    return state;
}

xi_control_message_t* xi_cbor_codec_ct_decode( const uint8_t* data, const uint32_t len )
{
    cn_cbor_errback err;
    cn_cbor* cb_map  = cn_cbor_decode( data, len, &err );
    cn_cbor* msgtype = cn_cbor_mapget_string( cb_map, XI_CBOR_CODEC_CT_STRING_MSGTYPE );

    xi_control_message_t* control_message_out = NULL;
    xi_state_t state                          = XI_STATE_OK;

    if ( NULL != msgtype )
    {
        switch ( msgtype->v.uint )
        {
            case XI_CONTROL_MESSAGE_BD_FILE_UPDATE_AVAILABLE:
            {
                XI_ALLOC_AT( xi_control_message_t, control_message_out, state );

                cn_cbor* msgver =
                    cn_cbor_mapget_string( cb_map, XI_CBOR_CODEC_CT_STRING_MSGVER );

                control_message_out->common.msgtype = msgtype->v.uint;
                control_message_out->common.msgver  = msgver->v.uint;

                cn_cbor* list =
                    cn_cbor_mapget_string( cb_map, XI_CBOR_CODEC_CT_STRING_LIST );

                if ( NULL != list )
                {
                    control_message_out->file_update_available.list_len = list->length;

                    if ( 0 < list->length )
                    {
                        XI_ALLOC_BUFFER_AT(
                            xi_control_message_file_desc_ext_t,
                            control_message_out->file_update_available.list,
                            sizeof( xi_control_message_t ) * list->length, state );

                        uint16_t id_file = 0;
                        for ( ; id_file < list->length; ++id_file )
                        {
                            cn_cbor* file = cn_cbor_index( list, id_file );

                            xi_cbor_codec_ct_decode_getstring(
                                file, XI_CBOR_CODEC_CT_STRING_FILE_NAME,
                                &control_message_out->file_update_available.list[id_file]
                                     .name );

                            xi_cbor_codec_ct_decode_getstring(
                                file, XI_CBOR_CODEC_CT_STRING_FILE_REVISION,
                                ( void** )&control_message_out->file_update_available
                                    .list[id_file]
                                    .revision );

                            xi_cbor_codec_ct_decode_getstring(
                                file, XI_CBOR_CODEC_CT_STRING_FILE_OPERATION,
                                &control_message_out->file_update_available.list[id_file]
                                     .file_operation );

                            xi_cbor_codec_ct_decode_getstring(
                                file, XI_CBOR_CODEC_CT_STRING_FILE_IMAGESIZE,
                                &control_message_out->file_update_available.list[id_file]
                                     .size_in_bytes );

                            xi_cbor_codec_ct_decode_getstring(
                                file, XI_CBOR_CODEC_CT_STRING_FILE_FINGERPRINT,
                                ( void** )&control_message_out->file_update_available
                                    .list[id_file]
                                    .fingerprint );
                        }
                    }
                }
            }

            break;

            case XI_CONTROL_MESSAGE_BD_FILE_CHUNK:

                break;

            case XI_CONTROL_MESSAGE_DB_FILE_INFO:
            case XI_CONTROL_MESSAGE_DB_FILE_GET_CHUNK:
            case XI_CONTROL_MESSAGE_DB_FILE_STATUS:
            default:
                cn_cbor_free( cb_map CBOR_CONTEXT_PARAM );
                return NULL;
        }
    }

    cn_cbor_free( cb_map CBOR_CONTEXT_PARAM );
    return control_message_out;

err_handling:

    cn_cbor_free( cb_map CBOR_CONTEXT_PARAM );
    XI_SAFE_FREE( control_message_out );

    return control_message_out;
}
