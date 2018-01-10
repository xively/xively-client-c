/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_SFT_LOGIC_APPLICATION_CALLBACK_H__
#define __XI_SFT_LOGIC_APPLICATION_CALLBACK_H__

#include <stdint.h>
#include <xively_types.h>

void xi_sft_on_file_downloaded_application_callback(
    void* sft_context_void,
    const char* filename,
    uint8_t flag_download_finished_successfully );

#endif /* __XI_SFT_LOGIC_APPLICATION_CALLBACK_H__ */
