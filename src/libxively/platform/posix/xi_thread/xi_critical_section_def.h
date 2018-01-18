/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_POSIX_CRITICAL_SECTION_H__
#define __XI_POSIX_CRITICAL_SECTION_H__

#include "xi_critical_section.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct xi_critical_section_s
 * @brief  Holds the state of the critical section
 *
 * This implementation is for spin locks so it require
 * the compiler to respect the volatile keyword.
 */
#ifdef XI_MODULE_THREAD_ENABLED
typedef struct xi_critical_section_s
{
    volatile int cs_state;
} xi_critical_section_t;
#endif

#ifdef __cplusplus
}
#endif

#endif /* __XI_POSIX_CRITICAL_SECTION_H__ */
