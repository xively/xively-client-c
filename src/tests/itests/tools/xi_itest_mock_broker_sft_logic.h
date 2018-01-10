/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_ITEST_MOCK_BROKER_SFT_LOGIC_H__
#define __XI_ITEST_MOCK_BROKER_SFT_LOGIC_H__

#include <xi_data_desc.h>

#define XI_MOCK_BROKER_SFT__FILE_CHUNK_STEP_SIZE 7777

xi_data_desc_t*
xi_mock_broker_sft_logic_on_message( const xi_data_desc_t* control_message );

void xi_mock_broker_sft_logic_get_fingerprint( uint32_t size_in_bytes,
                                               uint8_t** fingerprint_out,
                                               uint16_t* fingerprint_len_out );

#endif /* __XI_ITEST_MOCK_BROKER_SFT_LOGIC_H__ */
