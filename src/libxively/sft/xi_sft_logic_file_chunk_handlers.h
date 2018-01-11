/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_SFT_LOGIC_FILE_CHUNK_HANDLERS_H__
#define __XI_SFT_LOGIC_FILE_CHUNK_HANDLERS_H__

#include <xi_sft_logic.h>
#include <xi_control_message.h>

xi_control_message__sft_file_status_code_t
xi_sft_on_message_file_chunk_process_file_chunk( xi_sft_context_t* context,
                                                 xi_control_message_t* sft_message_in );

xi_control_message__sft_file_status_code_t
xi_sft_on_message_file_chunk_checksum_final( xi_sft_context_t* context );

#endif /* __XI_SFT_LOGIC_FILE_CHUNK_HANDLERS_H__ */
