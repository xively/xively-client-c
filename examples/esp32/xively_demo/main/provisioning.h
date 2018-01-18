/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client codebase,
 * it is licensed under the BSD 3-Clause license.
 */
#ifndef __PROVISIONING_H__
#define __PROVISIONING_H__

#include "user_data.h"

#ifdef __cplusplus
extern "C" {
#endif
/*! \file
 * @brief Runtime provisioning implementation - Used to handle the UI to interact
 * with the user to gather all required credentials.
 * @detailed In this demo, runtime provisioning is done over UART. You may replace
 * this implementation with, for instance, start the device as an AP when provisioning
 * is required and implement a webserver on the device so the user can set their
 * credentials from the browser via HTTP.
 * For demos, the Xively credentials are provisioned the same way as WiFi creds,
 * but in a production environment you'd use different methods- Ideally, flashing
 * the credentials in the factory

 * \copyright 2003-2018, LogMeIn, Inc.  All rights reserved.
 *
 */


/**
 * @brief    Ask the user for their WiFi and MQTT credentials via UART and save
 *           them to Non-Volatile Storage if requested
 * @detailed The provisioning files can be updated to provision via HTTP, etc.
 *
 * @param [out] The received credentials will be copied to the dst struct
 *
 * @retval -2 Failed to save credentials to NVS
 * @retval -1 Failed to get credentials from the user
 * @retval  0 OK
 */
int8_t provisioning_gather_user_data( user_data_t* dst );

#ifdef __cplusplus
}
#endif
#endif /* __PROVISIONING_H__ */
