/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_INTERNALS_H__
#define __XI_INTERNALS_H__

#include "xi_fs_api.h"

typedef struct xi_internals_s
{
    xi_fs_functions_t fs_functions;
} xi_internals_t;

extern xi_internals_t xi_internals;

#endif /* __XI_INTERNALS_H__ */
