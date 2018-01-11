/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_ESP32_FWU_NOTIFICATIONS_H__
#define __XI_ESP32_FWU_NOTIFICATIONS_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>

typedef void( xi_bsp_fwu_notifications_update_started_callback_t )( const char* filename,
                                                                    size_t file_size );
typedef void( xi_bsp_fwu_notifications_chunk_written_callback_t )( size_t chunk_size,
                                                                   size_t offset );
typedef void( xi_bsp_fwu_notifications_update_applied_callback_t )( void );

typedef struct
{
    xi_bsp_fwu_notifications_update_started_callback_t* update_started;
    xi_bsp_fwu_notifications_chunk_written_callback_t* chunk_written;
    xi_bsp_fwu_notifications_update_applied_callback_t* update_applied;
} xi_bsp_fwu_notification_callbacks_t;

extern xi_bsp_fwu_notification_callbacks_t xi_bsp_fwu_notification_callbacks;

#ifdef __cplusplus
}
#endif

#endif /* __XI_ESP32_FWU_NOTIFICATIONS_H__ */
