/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_SFT_LOGIC_H__
#define __XI_SFT_LOGIC_H__

#include <xively_error.h>
#include <xi_control_message.h>

typedef struct
{
    void* callback_user_data;
} xi_sft_context_t;

xi_state_t xi_sft_make_context( xi_sft_context_t** context );
xi_state_t xi_sft_free_context( xi_sft_context_t** context );

xi_state_t
xi_sft_on_message( xi_sft_context_t* context, const xi_control_message_t* sft_message );

#endif /* __XI_SFT_LOGIC_H__ */
