#ifndef __USER_DATA_H
#define __USER_DATA_H

#define DATA_STR_LEN 64 // Must be a multiple of 4 for CRC calculations

enum
{
    USER_DATA_NOT_INIT        = -4, // Module still needs to be initialized
    USER_DATA_CORRUPT         = -3, // Data was present but corrupt (CRC mismatch)
    USER_DATA_NOT_FOUND       = -2, // There was no saved data
    USER_DATA_READY           = 0, // Data successfully read and not modified
    USER_DATA_UNSAVED_CHANGES = 1 // Data updated but not saved to flash
} user_data_status_e;

typedef struct
{
    char flash_data_checksum[DATA_STR_LEN];

    int8_t wifi_client_cipher; // Enum; casts to int8_t
    char wifi_client_ssid[DATA_STR_LEN];
    char wifi_client_password[DATA_STR_LEN];
    char wifi_ap_ssid[DATA_STR_LEN];

    char xi_account_id[DATA_STR_LEN];
    char xi_device_id[DATA_STR_LEN];
    char xi_device_password[DATA_STR_LEN];
} user_data_t;

int8_t flash_init( void );
int8_t flash_get_user_data( user_data_t* user_data );
int8_t flash_set_user_data( user_data_t* user_data ); // TODO: How can I get this function
                                                      // to run from RAM with our
                                                      // compiler? `__ram`??
int8_t flash_user_data_saved_status( user_data_t* user_data );
int8_t flash_delete_user_data( void );

#endif /* __USER_DATA_H */
