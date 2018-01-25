/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_SFT_LOGIC_INTERNAL_METHODS_H__
#define __XI_SFT_LOGIC_INTERNAL_METHODS_H__

#include <xi_sft_logic.h>
#include <xi_control_message.h>

void _xi_sft_send_file_status( const xi_sft_context_t* context,
                               const xi_control_message_file_desc_ext_t* file_desc_ext,
                               xi_control_message__sft_file_status_phase_t phase,
                               xi_control_message__sft_file_status_code_t code );

void _xi_sft_send_file_get_chunk( xi_sft_context_t* context,
                                  uint32_t offset,
                                  uint32_t length );

void _xi_sft_current_file_revision_handling( xi_sft_context_t* context );

void _xi_sft_continue_package_download( xi_sft_context_t* context );

xi_state_t _xi_sft_select_next_resource_to_download( xi_sft_context_t* context );

#endif /* __XI_SFT_LOGIC_INTERNAL_METHODS_H__ */
