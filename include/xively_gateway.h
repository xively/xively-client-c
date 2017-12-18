/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XIVELY_GATEWAY_H__
#define __XIVELY_GATEWAY_H__

#include <xively_types.h>

#ifdef __cplusplus
extern "C" {
#endif

extern xi_context_handle_t xi_create_gateway_context();

extern xi_state_t xi_delete_gateway_context( xi_context_handle_t gateway_context_handle );

#ifdef __cplusplus
}
#endif

#endif /* __XIVELY_GATEWAY_H__ */
