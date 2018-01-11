/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

/*       OTA Firmware Update notifications from the Xively Client's BSP
 *
 * Note:
 * This is a custom extension of the Xively Client's BSP, used to report the
 * status of ongoing firmware downloads to the application layer.
 *
 * Relevant:
 *    - xi_bsp_fwu_esp32.h
 *    - xi_bsp_io_fs_esp32.h
 *    - xi_bsp_fwu_notifications_esp32.h
 *    - ESP32 example
 */

#include "xi_bsp_fwu_notifications_esp32.h"
xi_bsp_fwu_notification_callbacks_t xi_bsp_fwu_notification_callbacks = {NULL, NULL,
                                                                         NULL};
