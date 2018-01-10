/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_MEMORY_TYPE_H__
#define __XI_MEMORY_TYPE_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \enum xi_memory_type_t
 * \brief describes the memory type so that datastructures know
 * if it should manage ( release / clean ) memory by itself or leave
 * it as it is.
 *
 * XI_MEMORY_TYPE_UNKNOWN - should be used as a guard, whenever the status
 * is unset this is mostly for default argument assigment protection
 *
 * XI_MEMORY_TYPE_MANAGED - buffer memory will be freed whenever destroy is
 * called.
 *
 * XI_MEMORY_TYPE_UNMANAGED - buffer memory is not managed by the entity.
 * Therefore the buffer will not be freed whenever destroy is called.
 **/
typedef enum {
    XI_MEMORY_TYPE_UNKNOWN,
    XI_MEMORY_TYPE_MANAGED,
    XI_MEMORY_TYPE_UNMANAGED
} xi_memory_type_t;

#ifdef __cplusplus
}
#endif

#endif /* __XI_MEMORY_TYPE_H_ */
