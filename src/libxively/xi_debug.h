/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

/**
 * \file    xi_debug.h
 * \author  Olgierd Humenczuk
 * \brief   Macros to use for debugging
 */

#ifndef __XI_DEBUG_H__
#define __XI_DEBUG_H__

#include "xi_config.h"
#include "xi_data_desc.h"

#ifdef XI_PLATFORM_BASE_WMSDK
#include <wm_os.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XI_DEBUG_PRINTF
#include <stdio.h>
#define __xi_printf( ... )                                                               \
    printf( __VA_ARGS__ );                                                               \
    fflush( stdout );                                                                    \
    fflush( stderr )
#else /* XI_DEBUG_PRINTF */
#define __xi_printf( ... )                                                               \
    XI_DEBUG_PRINTF( __VA_ARGS__ );                                                      \
    fflush( stdout );                                                                    \
    fflush( stderr )
#endif /* XI_DEBUG_PRINTF */

#if XI_DEBUG_OUTPUT
void xi_debug_data_logger_impl( const char* msg, const xi_data_desc_t* data_desc );

#ifdef XI_PLATFORM_BASE_WMSDK
#define xi_debug_logger( msg ) __xi_printf( "[%d:(%s)] %s\n", __LINE__, __func__, msg )
#define xi_debug_format( fmt, ... )                                                      \
    __xi_printf( "[%d:(%s)] " fmt "\n", __LINE__, __func__, __VA_ARGS__ )
#define xi_debug_printf( ... ) __xi_printf( __VA_ARGS__ )
#define xi_debug_function_entered()                                                      \
    __xi_printf( "[%s:%d (%s)] -> entered\n", __FILE__, __LINE__, __func__ )
#define xi_debug_data_logger( msg, dsc )                                                 \
    __xi_printf( "[%s:%d (%s)] #\n", __FILE__, __LINE__, __func__ );                     \
    xi_debug_data_logger_impl( msg, dsc );
#else /* XI_PLATFORM_BASE_WMSDK */
#define xi_debug_logger( msg )                                                           \
    __xi_printf( "[%s:%d (%s)] %s\n", __FILE__, __LINE__, __func__, msg )
#define xi_debug_format( fmt, ... )                                                      \
    __xi_printf( "[%s:%d (%s)] " fmt "\n", __FILE__, __LINE__, __func__, __VA_ARGS__ )
#define xi_debug_printf( ... ) __xi_printf( __VA_ARGS__ )
#define xi_debug_function_entered()                                                      \
    __xi_printf( "[%s:%d (%s)] -> entered\n", __FILE__, __LINE__, __func__ )
#define xi_debug_data_logger( msg, dsc )                                                 \
    __xi_printf( "[%s:%d (%s)] #\n", __FILE__, __LINE__, __func__ );                     \
    xi_debug_data_logger_impl( msg, dsc );
#endif /* XI_PLATFORM_BASE_WMSDK */

#else /* XI_DEBUG_OUTPUT */
#define xi_debug_logger( ... )
#define xi_debug_format( ... )
#define xi_debug_printf( ... )
#define xi_debug_function_entered()
#define xi_debug_data_logger( ... )
#endif /* XI_DEBUG_OUTPUT */

#define XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST()
#define XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST_OFF()                                    \
    printf( "[ libxively  ] %-50s, layerchainid = %p, in_out_state = %d, "               \
            "layer_type_id = "                                                           \
            "%d, data = %p\n",                                                           \
            __func__, XI_THIS_LAYER( context )->context_data, in_out_state,              \
            XI_THIS_LAYER( context )->layer_type_id, data );                             \
    fflush( stdout );                                                                    \
    fflush( stderr );

#if XI_DEBUG_ASSERT
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#else /* XI_DEBUG_ASSERT */
/* The actual header is missing in some toolchains, so we wrap it here. */
#define assert( e ) ( ( void )0 )
#endif /* XI_DEBUG_ASSERT */

#ifdef __cplusplus
}
#endif

#endif /* __XI_DEBUG_H__ */
