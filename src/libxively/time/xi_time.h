/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license. 
 */

#ifndef __XI_TIME_H__
#define __XI_TIME_H__

#include <xively_time.h>

#ifdef __cplusplus
extern "C" {
#endif

xi_time_t xi_getcurrenttime_seconds();

xi_time_t xi_getcurrenttime_milliseconds();

#ifdef __cplusplus
}
#endif

#endif /* __XI_TIME_H__ */
