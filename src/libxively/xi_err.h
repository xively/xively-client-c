/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

/* - Every function should return a value
 * - There are special values (usually `0` or `-1`) which indicate occurrence of
 *   an error
 * - User can detect and lookup errors using declarations below */

#ifndef __XI_ERR_H__
#define __XI_ERR_H__

#include <xively_error.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XI_OPT_NO_ERROR_STRINGS 1

#ifndef XI_OPT_NO_ERROR_STRINGS
extern const char* xi_err_string[XI_ERROR_COUNT];
#endif

extern const char* xi_get_state_string( xi_state_t e );

#ifdef __cplusplus
}
#endif

#endif /* __XI_ERR_H__ */
