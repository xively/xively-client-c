/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_COROUTINE_H__
#define __XI_COROUTINE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* This block of code below must not be clang-formatted */
// clang-format off

#define XI_CR_START( state )\
    switch( state )\
    { \
        default:

#define XI_CR_YIELD( state, ret )\
    state = __LINE__; return ret; case __LINE__:

#define XI_CR_YIELD_ON( state, expression, ret )\
    if ( (expression) ) \
    { \
      state = __LINE__; return ret; case __LINE__:; \
    }

#define XI_CR_YIELD_UNTIL( state, expression, ret )\
    if ( (expression) ) \
    { \
      state = __LINE__; return ret; case __LINE__:; \
      continue; \
    }

#define XI_CR_EXIT( state, ret )\
    state = 1; return ret;

#define XI_CR_RESTART( state, ret )\
    state = 0; return ret;

#define XI_CR_END()\
    };

// clang-format on

#define XI_CR_IS_RUNNING( state ) ( state > 2 )

#define XI_CR_RESET( state ) state = 2

#ifdef __cplusplus
}
#endif

#endif /* __XI_COROUTINE_H__ */
