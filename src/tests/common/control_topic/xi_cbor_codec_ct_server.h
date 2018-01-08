/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_CBOR_CODEC_CT_SERVER_H__
#define __XI_CBOR_CODEC_CT_SERVER_H__

#include <stdint.h>
#include <xi_control_message.h>

void xi_cbor_codec_ct_server_encode( const xi_control_message_t* control_message,
                                     uint8_t** out_encoded_allocated_inside,
                                     uint32_t* out_len );

xi_control_message_t*
xi_cbor_codec_ct_server_decode( const uint8_t* data, const uint32_t len );

#endif /* __XI_CBOR_CODEC_CT_SERVER_H__ */
