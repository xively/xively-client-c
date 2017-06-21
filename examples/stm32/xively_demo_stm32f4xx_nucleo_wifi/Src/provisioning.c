#include <stdint.h>
#include <stdio.h>

#include "provisioning.h"
#include "wifi_interface.h"

static inline void provisioning_print_banner( void );
static int8_t provisioning_get_ap_credentials( user_data_t* udata );
static int8_t provisioning_get_xively_credentials( user_data_t* udata );

int8_t provisioning_start( user_data_t* dst )
{
    provisioning_print_banner();

    /* Get WiFi AP Credentials from the user */
    switch ( provisioning_get_ap_credentials( dst ) )
    {
        case 1:
            printf( "\r\n>> Saving user data to flash" );
            user_data_save_to_flash( dst );
            break;
        case 0:
            break;
        default:
            printf( "\r\n\t[ERROR] Getting AP credentials from user" );
            return -1;
    }

    /* Get Xively Credentials from the user */
    switch ( provisioning_get_xively_credentials( dst ) )
    {
        case 1:
            printf( "\r\n>> Saving user data to flash" );
            user_data_save_to_flash( dst );
            break;
        case 0:
            break;
        default:
            printf( "\r\n\t[ERROR] Getting AP credentials from user" );
            return -1;
    }
    return 0;
}

/**
 * @brief  Gather WiFi credentials from the end user via UART
 * @param  udata: pointer to the user_data_t structure to be updated
 * @retval <0: Error
 * @retval 0: User decided not to update these credentials
 * @retval 1: Credentials updated OK
 */
/* TODO: This unsafe implementation is temporary until we've got HTTP config mode */
static int8_t provisioning_get_ap_credentials( user_data_t* udata )
{
    char single_char_input[2] = "0";

    printf( "\r\n|********************************************************|" );
    printf( "\r\n|              Gathering WiFi AP Credentials             |" );
    printf( "\r\n|********************************************************|" );
    fflush( stdout );

    printf( "\r\n>> Would you like to update your WiFi credentials? [y/N]: " );
    fflush( stdout );
    scanf( "%2s", single_char_input ); /* UNSAFE - DO NOT USE IN PRODUCTION */
    switch ( single_char_input[0] )
    {
        case 'y':
        case 'Y':
            break;
        default:
            return 0;
    }

    printf( "\r\n>> Enter WiFi SSID: " );
    fflush( stdout );
    scanf( "%s", udata->wifi_client_ssid ); /* UNSAFE - DO NOT USE IN PRODUCTION */

    printf( "\r\n>> Enter WiFi Password: " );
    fflush( stdout );
    scanf( "%s", udata->wifi_client_password ); /* UNSAFE - DO NOT USE IN PRODUCTION */

    printf( "\r\n>> Enter WiFi encryption mode [0:Open, 1:WEP, 2:WPA2/WPA2-Personal]: " );
    fflush( stdout );
    scanf( "%2s", single_char_input ); /* UNSAFE - DO NOT USE IN PRODUCTION */
    switch ( single_char_input[0] )
    {
        case '0':
            user_data_set_wifi_encryption( udata, None );
            break;
        case '1':
            user_data_set_wifi_encryption( udata, WEP );
            break;
        case '2':
            user_data_set_wifi_encryption( udata, WPA_Personal );
            break;
        default:
            printf( "\r\n>> Wrong Entry. Mode [%s] is not an option", single_char_input );
            return -2;
    }
    return 1;
}

/**
 * @brief  Gather MQTT credentials from the end user via UART
 * @param  udata: pointer to the user_data_t structure to be updated
 * @retval <0: Error
 * @retval 0: User decided not to update these credentials
 * @retval 1: Credentials updated OK
 */
//static int8_t get_xively_credentials( user_data_t* udata )
static int8_t provisioning_get_xively_credentials( user_data_t* udata )
{
    char single_char_input[2] = "0";

    printf( "\r\n|********************************************************|" );
    printf( "\r\n|              Gathering Xively Credentials              |" );
    printf( "\r\n|********************************************************|" );
    fflush( stdout );

    printf( "\r\n>> Would you like to update your MQTT credentials? [y/N]: " );
    fflush( stdout );
    scanf( "%2s", single_char_input ); /* UNSAFE - DO NOT USE IN PRODUCTION */
    switch ( single_char_input[0] )
    {
        case 'y':
        case 'Y':
            break;
        default:
            return 0;
    }

    printf( "\r\n>> Enter the Xively Account ID: " );
    fflush( stdout );
    scanf( "%s", udata->xi_account_id ); /* UNSAFE - DO NOT USE IN PRODUCTION */

    printf( "\r\n>> Enter the Xively Device ID: " );
    fflush( stdout );
    scanf( "%s", udata->xi_device_id ); /* UNSAFE - DO NOT USE IN PRODUCTION */

    printf( "\r\n>> Enter the Xively Device Password: " );
    fflush( stdout );
    scanf( "%s", udata->xi_device_password ); /* UNSAFE - DO NOT USE IN PRODUCTION */

    return 1;
}

static inline void provisioning_print_banner( void )
{
    printf( "\r\n|********************************************************|" );
    printf( "\r\n|               User Configuration Routine               |" );
    printf( "\r\n|               --------------------------               |" );
    printf( "\r\n| WARNING: Copy-pasting long strings to your serial      |" );
    printf( "\r\n| -------  terminal may result in loss of characters.    |" );
    printf( "\r\n|          To solve this issue use a delay between       |" );
    printf( "\r\n|          characters in the terminal settings.          |" );
    printf( "\r\n|          E.g in coolTerm:                              |" );
    printf( "\r\n|   Options->Transmit->Use transmit character delay      |" );
    printf( "\r\n|                                                        |" );
    printf( "\r\n| NOTE: You will only need to do this once.              |" );
    printf( "\r\n|                                                        |" );
    printf( "\r\n| NOTE: Next time you want to update these, keep the     |" );
    printf( "\r\n|       nucleo board's User button pressed during boot   |" );
    printf( "\r\n|********************************************************|" );
    fflush( stdout );
}
