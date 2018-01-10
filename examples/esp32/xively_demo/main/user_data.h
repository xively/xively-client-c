/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client codebase,
 * it is licensed under the BSD 3-Clause license.
 */
#ifndef __USER_DATA_H__
#define __USER_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif
/*! \file
 * @brief A datastructure to keep your device's credentials and manipulate them
 * at runtime, and an API to get/set them in Non-Volatile Storage

 * \copyright 2003-2018, LogMeIn, Inc.  All rights reserved.
 *
 */

#define USER_DATA_STR_SIZE 64

/* Buffer sizes hardcoded in the ESP32's SDK, available here for convenience */
#define ESP_WIFI_SSID_STR_SIZE     32
#define ESP_WIFI_PASSWORD_STR_SIZE 64

typedef struct
{
    char wifi_client_ssid[USER_DATA_STR_SIZE];
    char wifi_client_password[USER_DATA_STR_SIZE];

    char xi_account_id[USER_DATA_STR_SIZE];
    char xi_device_id[USER_DATA_STR_SIZE];
    char xi_device_password[USER_DATA_STR_SIZE];
} user_data_t;

/**
 * @brief  Initialize the device's Non-Volatile Storage
 *
 * @retval -1 Error: Something is probably misconfigured in the source code
 * @retval 0 Success
 */
int8_t user_data_flash_init( void );

/**
 * @brief      Verify all required credentials are present in the received user_data.
 * Only non-required string is the WiFi password
 *
 * @param [in] Structure to validate
 *
 * @retval -1 Any of the strings is empty. The device needs to be (re)provisioned
 * @retval  0 All required strings are present: wifi_ssid, xi_acc_id,
 *            xi_device_id, xi_device_pwd
 */
int8_t user_data_is_valid( user_data_t* user_data );

/**
 * @brief Erase ESP32's NVS
 *
 * @retval -1 Error
 * @retval  0 OK
 */
int8_t user_data_reset_flash( void );

/**
 * @brief Copy all credentials from flash into the dst datastructure
 *
 * @param [out] dst will be filled with the credentials retrieved from flash
 *
 * @retval -1 Error
 * @retval  0 OK
 */
int8_t user_data_copy_from_flash( user_data_t* dst );

/**
 * @brief Save user credentials to NVS
 *
 * @param [in] user_data_t structure to be saved
 *
 * @retval -1 Error
 * @retval  0 OK
 */
int8_t user_data_save_to_flash( user_data_t* src );

/**
 * @brief Print the contents of user_data nicely formatted to UART for debugging
 * purposes
 *
 * @param [in] user_data structure to be printed
 */
void user_data_printf( user_data_t* user_data );

#define user_data_set_wifi_ssid( data_ptr, ssid )                     \
    strncpy( data_ptr->wifi_client_ssid, ssid, USER_DATA_STR_SIZE )
#define user_data_set_wifi_password( data_ptr, password )             \
    strncpy( data_ptr->wifi_client_password, password, USER_DATA_STR_SIZE )

#define user_data_set_xi_account_id( data_ptr, acc_id )               \
    strncpy( data_ptr->xi_account_id, acc_id, USER_DATA_STR_SIZE )
#define user_data_set_xi_device_id( data_ptr, dev_id )                \
    strncpy( data_ptr->xi_device_id, dev_id, USER_DATA_STR_SIZE )
#define user_data_set_xi_device_password( data_ptr, pwd )             \
    strncpy( data_ptr->xi_device_password, pwd, USER_DATA_STR_SIZE )

#ifdef __cplusplus
}
#endif
#endif /* __USER_DATA_H__ */
