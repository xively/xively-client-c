/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_BSP_DEBUG__
#define __XI_BSP_DEBUG__

#define BSP_DEBUG_LOG 0

#ifndef XI_DEBUG_PRINTF
#include <stdio.h>
#define __xi_printf( ... )                                                               \
    printf( __VA_ARGS__ );                                                               \
    fflush( stdout )
#else /* XI_DEBUG_PRINTF */
#define __xi_printf( ... ) XI_DEBUG_PRINTF( __VA_ARGS__ );
#endif /* XI_DEBUG_PRINTF */

#if BSP_DEBUG_LOG
#define xi_bsp_debug_logger( format_string )                                             \
    __xi_printf( "%s@%d[  BSP  ] " format_string "\n", __FILE__, __LINE__ )
#define xi_bsp_debug_format( format_string, ... )                                        \
    __xi_printf( "%s@%d[  BSP  ] " format_string "\n", __FILE__, __LINE__, __VA_ARGS__ )
#else /* BSP_DEBUG_LOG */
#define xi_bsp_debug_logger( ... )
#define xi_bsp_debug_format( ... )
#endif /* BSP_DEBUG_LOG */

#endif /*  __XI_BSP_DEBUG__ */
