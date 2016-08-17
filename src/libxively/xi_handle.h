/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#ifndef __XI_HANDLE_H__
#define __XI_HANDLE_H__

#include "xi_types.h"
#include "xi_vector.h"

/*-----------------------------------------------------------------------
 *  TYPEDEFS
 * ----------------------------------------------------------------------- */

typedef int32_t xi_handle_t;

/*-----------------------------------------------------------------------
 *  PUBLIC FUNCTIONS
 * ----------------------------------------------------------------------- */

void* xi_object_for_handle( xi_vector_t* vector, xi_handle_t handle );
xi_state_t
xi_find_handle_for_object( xi_vector_t* vector, const void* object, xi_handle_t* handle );
xi_state_t xi_delete_handle_for_object( xi_vector_t* vector, const void* object );
xi_state_t xi_register_handle_for_object( xi_vector_t* vector,
                                          const int32_t max_object_cnt,
                                          const void* object );

#endif /* __XI_HANDLE_H__ */
