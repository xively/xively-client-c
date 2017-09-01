/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_SFT_REVISION_H__
#define __XI_SFT_REVISION_H__

#include <xively_error.h>

xi_state_t
xi_sft_revision_set( const char* const resource_name, const char* const revision );

xi_state_t xi_sft_revision_get( const char* const resource_name, char** revision_out );


xi_state_t
xi_sft_revision_set_firmware_uptate( const char* const resource_name_xi_firmware,
                                     const char* const revision_xi_firmware );

xi_state_t xi_sft_revision_firmware_ok();

#endif /* __XI_SFT_REVISION_H__ */
