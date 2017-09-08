#ifndef __USER_DATA_H__
#define __USER_DATA_H__

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

int8_t user_data_flash_init( void );
int8_t user_data_is_valid( user_data_t* user_data );
int8_t user_data_reset_flash( void );
int8_t user_data_copy_from_flash( user_data_t* dst );
int8_t user_data_save_to_flash( user_data_t* src );
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

#endif /* __USER_DATA_H__ */
