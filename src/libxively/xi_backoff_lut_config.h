/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_BACKOFF_LUT_CONFIG_H__
#define __XI_BACKOFF_LUT_CONFIG_H__

#include "xi_vector.h"

#ifdef __cplusplus
extern "C" {
#endif

static const xi_vector_elem_t XI_BACKOFF_LUT[] = {
    XI_VEC_ELEM( XI_VEC_VALUE_UI32( 0 ) ),   XI_VEC_ELEM( XI_VEC_VALUE_UI32( 2 ) ),
    XI_VEC_ELEM( XI_VEC_VALUE_UI32( 4 ) ),   XI_VEC_ELEM( XI_VEC_VALUE_UI32( 8 ) ),
    XI_VEC_ELEM( XI_VEC_VALUE_UI32( 16 ) ),  XI_VEC_ELEM( XI_VEC_VALUE_UI32( 32 ) ),
    XI_VEC_ELEM( XI_VEC_VALUE_UI32( 64 ) ),  XI_VEC_ELEM( XI_VEC_VALUE_UI32( 128 ) ),
    XI_VEC_ELEM( XI_VEC_VALUE_UI32( 256 ) ), XI_VEC_ELEM( XI_VEC_VALUE_UI32( 512 ) )};

static const xi_vector_elem_t XI_DECAY_LUT[] = {
    XI_VEC_ELEM( XI_VEC_VALUE_UI32( 4 ) ),  XI_VEC_ELEM( XI_VEC_VALUE_UI32( 4 ) ),
    XI_VEC_ELEM( XI_VEC_VALUE_UI32( 8 ) ),  XI_VEC_ELEM( XI_VEC_VALUE_UI32( 16 ) ),
    XI_VEC_ELEM( XI_VEC_VALUE_UI32( 30 ) ), XI_VEC_ELEM( XI_VEC_VALUE_UI32( 30 ) ),
    XI_VEC_ELEM( XI_VEC_VALUE_UI32( 30 ) ), XI_VEC_ELEM( XI_VEC_VALUE_UI32( 30 ) ),
    XI_VEC_ELEM( XI_VEC_VALUE_UI32( 30 ) ), XI_VEC_ELEM( XI_VEC_VALUE_UI32( 30 ) )};

#ifdef __cplusplus
}
#endif

#endif /* __XI_BACKOFF_LUT_CONFIG_H__ */
