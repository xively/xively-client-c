#ifndef __USER_DATA_H
#define __USER_DATA_H

#define USE_FLASH_STORAGE 1

#define DATA_STR_LEN 64 // Must be a multiple of 4 for CRC calculations

typedef struct
{
    int32_t crc_checksum; /* For internal use */

    int32_t wifi_client_cipher; // Assigned an SDK enum value
    char wifi_client_ssid[DATA_STR_LEN];
    char wifi_client_password[DATA_STR_LEN];

    char xi_account_id[DATA_STR_LEN];
    char xi_device_id[DATA_STR_LEN];
    char xi_device_password[DATA_STR_LEN];
} user_data_t;

extern user_data_t* runtime_user_data_ptr;
extern user_data_t* flash_user_data_ptr;

#define user_data_set_checksum( data_ptr, checksum )                                     \
    ( data_ptr->crc_checksum = ( int32_t )checksum )
#define user_data_set_wifi_cipher( data_ptr, cipher )                                    \
    ( data_ptr->wifi_client_cipher = ( int32_t )cipher )
#define user_data_set_wifi_ssid( data_ptr, ssid )                                        \
    strncpy( data_ptr->wifi_client_ssid, ssid, DATA_STR_LEN )
#define user_data_set_wifi_psk( data_ptr, psk )                                          \
    strncpy( data_ptr->wifi_client_password, psk, DATA_STR_LEN )
#define user_data_set_xi_account_id( data_ptr, acc_id )                                  \
    strncpy( data_ptr->xi_account_id, acc_id, DATA_STR_LEN )
#define user_data_set_xi_device_id( data_ptr, dev_id )                                   \
    strncpy( data_ptr->xi_device_id, dev_id, DATA_STR_LEN )
#define user_data_set_xi_device_password( data_ptr, pwd )                                \
    strncpy( data_ptr->xi_device_password, pwd, DATA_STR_LEN )

user_data_t* user_data_init( void );
void user_data_free( void );
void user_data_printf( user_data_t* user_data );
user_data_t* user_data_get_ptr( void ); // Returns runtime_user_data_ptr. Must be modified
                                        // using the user_data_set_ functions
int8_t user_data_reset_flash( void );   // Removes all user data from flash
int8_t user_data_read_flash( void );
int8_t user_data_save_to_flash( void ); // TODO: How can I get this function
                                        // to run from RAM with our
                                        // compiler? `__ram`??

#endif /* __USER_DATA_H */
