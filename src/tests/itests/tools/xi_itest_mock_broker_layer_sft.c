/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_itest_mock_broker_layer_sft.h"
#include <stdio.h>

#include <xi_control_message.h>
#include <xi_cbor_codec_ct_server.h>

void xi_mock_broker_sft_on_message( const xi_data_desc_t* control_message_encoded )
{
    xi_control_message_t* control_message = xi_cbor_codec_ct_server_decode(
        control_message_encoded->data_ptr, control_message_encoded->length );

    xi_debug_control_message_dump( control_message, "mock broker --- INCOMING" );

    xi_control_message_free( &control_message );

    // - 1 liner: decode CS SFT messagem, will result in a xi_control_message_t struct
    // - channel each msg type into separate functions (3 messages) to help failure
    //   control from test case
    // - apply mock broker logic: happy broker OR controlled failure -->
    //   xi_control_message_t
    // - encode and send to client
}
