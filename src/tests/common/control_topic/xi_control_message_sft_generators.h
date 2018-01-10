/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_CONTROL_MESSAGE_SFT_GENERATORS_H__
#define __XI_CONTROL_MESSAGE_SFT_GENERATORS_H__

#include <xi_control_message.h>

uint8_t* xi_control_message_sft_get_reproducible_randomlike_bytes( uint32_t offset,
                                                                   uint32_t length );

xi_control_message_t*
xi_control_message_sft_generate_reply_FILE_CHUNK( xi_control_message_t* control_message );

#endif /* __XI_CONTROL_MESSAGE_SFT_GENERATORS_H__ */
