/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client codebase,
 * it is licensed under the BSD 3-Clause license.
 */
#ifndef __PROVISIONING_H__
#define __PROVISIONING_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "user_data.h"

int8_t provisioning_gather_user_data( user_data_t* dst );

#ifdef __cplusplus
}
#endif
#endif /* __PROVISIONING_H__ */
