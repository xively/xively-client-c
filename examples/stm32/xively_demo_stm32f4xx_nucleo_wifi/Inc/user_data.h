#ifndef __USER_DATA_H__
#define __USER_DATA_H__

#define USER_DATA_STR_SIZE 64 /* Must be a multiple of sizeof(int32_t) for CRC periph */

typedef struct
{
    int32_t crc_checksum; /* For internal use only - Set by user_data_save_to_flash() */

    int32_t wifi_client_encryption_mode; /* Casted from WiFi_Priv_Mode */
    char wifi_client_ssid[USER_DATA_STR_SIZE];
    char wifi_client_password[USER_DATA_STR_SIZE];

    char xi_account_id[USER_DATA_STR_SIZE];
    char xi_device_id[USER_DATA_STR_SIZE];
    char xi_device_password[USER_DATA_STR_SIZE];
} user_data_t;

int8_t user_data_flash_init( void );
int8_t user_data_validate_checksum( user_data_t* user_data );
int8_t user_data_reset_flash( void );
int8_t user_data_copy_from_flash( user_data_t* dst );
int8_t user_data_save_to_flash( user_data_t* src );
void user_data_printf( user_data_t* user_data );

#define user_data_set_wifi_encryption( data_ptr, encryption )         \
    ( data_ptr->wifi_client_encryption_mode = ( int32_t )encryption )
#define user_data_set_wifi_ssid( data_ptr, ssid )                     \
    strncpy( data_ptr->wifi_client_ssid, ssid, USER_DATA_STR_SIZE )
#define user_data_set_wifi_psk( data_ptr, psk )                       \
    strncpy( data_ptr->wifi_client_password, psk, USER_DATA_STR_SIZE )
#define user_data_set_xi_account_id( data_ptr, acc_id )               \
    strncpy( data_ptr->xi_account_id, acc_id, USER_DATA_STR_SIZE )
#define user_data_set_xi_device_id( data_ptr, dev_id )                \
    strncpy( data_ptr->xi_device_id, dev_id, USER_DATA_STR_SIZE )
#define user_data_set_xi_device_password( data_ptr, pwd )             \
    strncpy( data_ptr->xi_device_password, pwd, USER_DATA_STR_SIZE )

#endif /* __USER_DATA_H__ */
