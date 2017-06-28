/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_cbor_codec_ct_server.h>
#include <xi_cbor_codec_ct_keys.h>

#include <cbor.h>
#include <cn-cbor/cn-cbor.h>

#include <xively_error.h>
#include <xi_macros.h>
#include <xi_debug.h>

#ifdef USE_CBOR_CONTEXT

extern cn_cbor_context* context;

#endif

void xi_cbor_codec_ct_server_encode( const xi_control_message_t* control_message,
                                     uint8_t** out_encoded_allocated_inside,
                                     uint32_t* out_len )
{
    if ( NULL == control_message )
    {
        *out_len = 0;
        return;
    }

    cn_cbor_errback err;
    cn_cbor* cb_map = cn_cbor_map_create( CBOR_CONTEXT_PARAM_COMA & err );

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


    printf( "*** todo_atigyi: continue here with proper encoding of outgoing messages: "
            "FILE_UPDATE_AVAILABLE and FILE_CHUNK\n" );

    unsigned char encoded[512];
    *out_len = cn_cbor_encoder_write( encoded, 0, sizeof( encoded ), cb_map );

    cn_cbor_free( cb_map CBOR_CONTEXT_PARAM );

    xi_state_t state = XI_STATE_OK;
    XI_ALLOC_BUFFER_AT( uint8_t, *out_encoded_allocated_inside, *out_len, state );

    memcpy( *out_encoded_allocated_inside, encoded, *out_len );

err_handling:;
}

/* using the getvalue tool from libxively here in the mock broker */
xi_state_t xi_cbor_codec_ct_decode_getvalue( cn_cbor* source,
                                             const char* key,
                                             void* out_destination,
                                             uint16_t* out_len );


xi_control_message_t*
xi_cbor_codec_ct_server_decode( const uint8_t* data, const uint32_t len )
{
    ( void )data;
    ( void )len;

    cn_cbor_errback err;
    cn_cbor* cb_map = cn_cbor_decode( data, len CBOR_CONTEXT_PARAM, &err );

    xi_control_message_t* control_message_out = NULL;

    xi_state_t state = XI_STATE_OK;

    XI_CHECK_CND_DBGMESSAGE( NULL == cb_map, XI_ELEMENT_NOT_FOUND, state,
                             "ERROR: data is not a CBOR binary" );

    XI_ALLOC_AT( xi_control_message_t, control_message_out, state );

    cn_cbor* msgtype = cn_cbor_mapget_string( cb_map, XI_CBOR_CODEC_CT_STRING_MSGTYPE );
    XI_CHECK_CND_DBGMESSAGE( NULL == msgtype, XI_INVALID_PARAMETER, state,
                             "ERROR: no 'msgtype' found" );
    control_message_out->common.msgtype = msgtype->v.uint;


    cn_cbor* msgver = cn_cbor_mapget_string( cb_map, XI_CBOR_CODEC_CT_STRING_MSGVER );
    XI_CHECK_CND_DBGMESSAGE( NULL == msgver, XI_INVALID_PARAMETER, state,
                             "ERROR: no 'msgver' found" );
    control_message_out->common.msgver = msgver->v.uint;


    switch ( msgtype->v.uint )
    {
        case XI_CONTROL_MESSAGE_CS_FILE_INFO:
        {
            cn_cbor* list = cn_cbor_mapget_string( cb_map, XI_CBOR_CODEC_CT_STRING_LIST );

            if ( NULL != list )
            {
                control_message_out->file_info.list_len = list->length;

                if ( 0 < list->length )
                {
                    XI_ALLOC_BUFFER_AT(
                        xi_control_message_file_desc_t,
                        control_message_out->file_info.list,
                        sizeof( xi_control_message_file_desc_t ) * list->length, state );

                    uint16_t id_file = 0;
                    for ( ; id_file < list->length; ++id_file )
                    {
                        cn_cbor* file = cn_cbor_index( list, id_file );

                        xi_cbor_codec_ct_decode_getvalue(
                            file, XI_CBOR_CODEC_CT_STRING_FILE_NAME,
                            &control_message_out->file_info.list[id_file].name, NULL );

                        xi_cbor_codec_ct_decode_getvalue(
                            file, XI_CBOR_CODEC_CT_STRING_FILE_REVISION,
                            &control_message_out->file_info.list[id_file].revision,
                            NULL );
                    }
                }
            }
        }

        break;

        case XI_CONTROL_MESSAGE_CS_FILE_GET_CHUNK:

            xi_cbor_codec_ct_decode_getvalue( cb_map, XI_CBOR_CODEC_CT_STRING_FILE_NAME,
                                              &control_message_out->file_get_chunk.name,
                                              NULL );

            xi_cbor_codec_ct_decode_getvalue(
                cb_map, XI_CBOR_CODEC_CT_STRING_FILE_REVISION,
                &control_message_out->file_get_chunk.revision, NULL );

            xi_cbor_codec_ct_decode_getvalue(
                cb_map, XI_CBOR_CODEC_CT_STRING_FILECHUNK_OFFSET,
                &control_message_out->file_get_chunk.offset, NULL );

            xi_cbor_codec_ct_decode_getvalue(
                cb_map, XI_CBOR_CODEC_CT_STRING_FILECHUNK_LENGTH,
                &control_message_out->file_get_chunk.length, NULL );

            break;

        case XI_CONTROL_MESSAGE_CS_FILE_STATUS:

            xi_cbor_codec_ct_decode_getvalue( cb_map, XI_CBOR_CODEC_CT_STRING_FILE_NAME,
                                              &control_message_out->file_status.name,
                                              NULL );

            xi_cbor_codec_ct_decode_getvalue(
                cb_map, XI_CBOR_CODEC_CT_STRING_FILE_REVISION,
                &control_message_out->file_status.revision, NULL );

            xi_cbor_codec_ct_decode_getvalue(
                cb_map, XI_CBOR_CODEC_CT_STRING_FILESTATUS_PHASE,
                &control_message_out->file_status.phase, NULL );

            xi_cbor_codec_ct_decode_getvalue(
                cb_map, XI_CBOR_CODEC_CT_STRING_FILESTATUS_CODE,
                &control_message_out->file_status.code, NULL );

            break;

        case XI_CONTROL_MESSAGE_SC_FILE_UPDATE_AVAILABLE:
        case XI_CONTROL_MESSAGE_SC_FILE_CHUNK:;
    }

    cn_cbor_free( cb_map CBOR_CONTEXT_PARAM );
    return control_message_out;

err_handling:

    cn_cbor_free( cb_map CBOR_CONTEXT_PARAM );
    XI_SAFE_FREE( control_message_out );

    return control_message_out;
}
