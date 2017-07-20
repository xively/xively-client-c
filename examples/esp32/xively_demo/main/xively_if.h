/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client codebase,
 * it is licensed under the BSD 3-Clause license.
 */

void xif_set_device_info( char* xi_acc_id, char* xi_dev_id, char* xi_dev_pwd );
void xif_rtos_task( void* param );

/* This callback is __weak__ in xively_if.c so you can overwrite it with your own.
 * For this demo, we permanently shut down the Xively Interface when we get
 * unrecoverable errors, but you may want to handle that differently
 */
extern void xif_state_machine_aborted_callback( void );
