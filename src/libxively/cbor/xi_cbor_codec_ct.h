/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_CBOR_CODEC_CT_H__
#define __XI_CBOR_CODEC_CT_H__

#include <stdint.h>

union xi_control_message_u;

void xi_cbor_codec_ct_encode( const union xi_control_message_u* control_message,
                              uint8_t** out_encoded_allocated_inside,
                              uint32_t* out_len );

union xi_control_message_u*
xi_cbor_codec_ct_decode( const uint8_t* data, const uint32_t len );

#endif /* __XI_CBOR_CODEC_CT_H__ */
