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

void xi_cbor_put_name_and_revision( cn_cbor* cb_map,
                                    const char* name,
                                    const char* revision,
                                    cn_cbor_errback* errp )
{
    if ( NULL != name )
    {
        cn_cbor_map_put( cb_map, cn_cbor_string_create( "N", errp ),
                         cn_cbor_string_create( name, errp ), errp );
    }

    if ( NULL != revision )
    {
        cn_cbor_map_put( cb_map, cn_cbor_string_create( "R", errp ),
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

    cn_cbor_map_put( cb_map, cn_cbor_string_create( "msgver", &err ),
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

                cn_cbor_map_put( cb_map, cn_cbor_string_create( "list", &err ), files,
                                 &err );
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

// #include <stdio.h>

xi_control_message_t* xi_cbor_codec_ct_decode( const uint8_t* data, const uint32_t len )
{
    cn_cbor_errback err;
    cn_cbor* cb_map = cn_cbor_decode( data, len, &err );

    cn_cbor* msgtype = cn_cbor_mapget_string( cb_map, XI_CBOR_CODEC_CT_STRING_MSGTYPE );

    if ( NULL != msgtype )
    {
        switch ( msgtype->v.uint )
        {
            case XI_CONTROL_MESSAGE_BD_FILE_UPDATE_AVAILABLE:
                // printf( "helloka\n" );

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

    return NULL;
}
