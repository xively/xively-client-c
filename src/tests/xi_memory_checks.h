/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_MEMORY_CHECKS_H__
#define __XI_MEMORY_CHECKS_H__

#ifdef XI_MEMORY_LIMITER_ENABLED
#include "xi_memory_limiter.h"
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef XI_MEMORY_LIMITER_ENABLED
#define xi_is_whole_memory_deallocated() ( xi_memory_limiter_get_allocated_space() == 0 )

/* real implementation of the tearup and teardown for tests */
void _xi_memory_limiter_tearup();
uint8_t _xi_memory_limiter_teardown();

/* this is the macro for  */
#define xi_memory_limiter_tearup _xi_memory_limiter_tearup
#define xi_memory_limiter_teardown _xi_memory_limiter_teardown

#else
#define xi_is_whole_memory_deallocated() 1
#define xi_memory_limiter_tearup()
#define xi_memory_limiter_teardown() 1
#endif

#ifdef __cplusplus
}
#endif

#endif /* __XI_MEMORY_CHECKS_H__ */
