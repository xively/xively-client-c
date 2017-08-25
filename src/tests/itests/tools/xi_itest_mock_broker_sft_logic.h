/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
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

#endif /* __XI_ITEST_MOCK_BROKER_SFT_LOGIC_H__ */
