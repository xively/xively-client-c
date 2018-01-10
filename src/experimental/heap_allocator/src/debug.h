/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <stdio.h>

#define debug_log( msg, ... )                                                            \
    printf( msg "\n", __VA_ARGS__ );                                                   \
    fflush( stdout )
