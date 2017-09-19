/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_CONTROL_MESSAGE_SFT_H__
#define __XI_CONTROL_MESSAGE_SFT_H__

#include <xi_control_message.h>

xi_control_message_t*
xi_control_message_create_file_info( const char** filenames, uint16_t count );

xi_control_message_t* xi_control_message_create_file_get_chunk( const char* filename,
                                                                const char* revision,
                                                                uint32_t offset,
                                                                uint32_t length );

xi_control_message_t* xi_control_message_create_file_status( const char* filename,
                                                             const char* revision,
                                                             uint8_t phase,
                                                             int8_t code );

const xi_control_message_file_desc_ext_t*
xi_control_message_file_update_available_get_next_file_desc_ext(
    const struct file_update_available_s* message_fua, const char* filename );

#define XI_CONTROL_MESSAGE_SFT_GENERATED_REVISION                                        \
    "revision is not available on the device, this is a generated revision 0"

#endif /* __XI_CONTROL_MESSAGE_H__SFT_ */
