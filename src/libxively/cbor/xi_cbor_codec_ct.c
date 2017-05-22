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

void xi_cbor_codec_ct_encode( const xi_control_message_t* control_message,
                              uint8_t** out_encoded_allocated_inside,
                              uint32_t* out_len )
{
    #if 0
    cn_cbor_errback err;
    cn_cbor* cb_map = cn_cbor_map_create( &err );

    cn_cbor_map_put( cb_map, cn_cbor_string_create( "msgtype", &err ),
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

                    cn_cbor_map_put(
                        file, cn_cbor_string_create( "N", &err ),
                        cn_cbor_string_create(
                            control_message->file_info.list[id_file].name, &err ),
                        &err );
                    cn_cbor_map_put(
                        file, cn_cbor_string_create( "R", &err ),
                        cn_cbor_string_create(
                            control_message->file_info.list[id_file].revision, &err ),
                        &err );

                    cn_cbor_array_append( files, file, &err );
                }

                cn_cbor_map_put( cb_map, cn_cbor_string_create( "list", &err ), files,
                                 &err );
            }
            break;
        case XI_CONTROL_MESSAGE_DB_FILE_GET_CHUNK:
            break;
        case XI_CONTROL_MESSAGE_DB_FILE_STATUS:
            break;
        /* the followings are not encoded by client */
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
#else
    (void) control_message;
    (void) out_encoded_allocated_inside;
    (void) out_len;
#endif
}

xi_control_message_t* xi_cbor_codec_ct_decode( const uint8_t* data, const uint32_t len )
{
    ( void )data;
    ( void )len;

    return 0;
}
