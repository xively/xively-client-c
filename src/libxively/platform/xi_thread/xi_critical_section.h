/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_CRITICAL_SECTION_H__
#define __XI_CRITICAL_SECTION_H__

#include "xi_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef XI_MODULE_THREAD_ENABLED

/* forward declaration of critical section */
struct xi_critical_section_s;

/**
 * @brief xi_init_critical_section creates and initializes the
 * xi_critical_section_t structure
 *
 * @param cs double pointer to the critiacl section object
 * @return xi_state_t
 */
extern xi_state_t xi_init_critical_section( struct xi_critical_section_s** cs );

/**
 * @brief xi_lock_critical_section locks a critical section
 * @param cs
 */
extern void xi_lock_critical_section( struct xi_critical_section_s* cs );

/**
 * @brief xi_unlock_critical_section unlocks a critical section
 * @param cs
 */
extern void xi_unlock_critical_section( struct xi_critical_section_s* cs );

/**
 * @brief xi_destroy_critical_section destroys the critical section and frees
 * it's memory
 * @param cs
 */
extern void xi_destroy_critical_section( struct xi_critical_section_s** cs );

#else

struct xi_critical_section_s
{
    char c;
};

#define xi_init_critical_section( section ) XI_STATE_OK
#define xi_lock_critical_section( section )
#define xi_unlock_critical_section( section )
#define xi_destroy_critical_section( section )

#endif

#ifdef __cplusplus
}
#endif

#endif /* __XI_CRITICAL_SECTION_H__ */
