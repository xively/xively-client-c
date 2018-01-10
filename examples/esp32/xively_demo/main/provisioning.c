/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client codebase,
 * it is licensed under the BSD 3-Clause license.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "provisioning.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "rom/uart.h"

/**
 * The ESP32 doesn't have scanf hooked to the UART at the moment, so we'll have
 * to poll the UART Rx until we get the characters we need. In order to do that
 * without blocking the entire RTOS scheduler, we use vTaskDelay - That way
 * FreeRTOS can continue ticking while we wait
 */
#define DELAY_RTOS_TICKS( t ) vTaskDelay( t )

static inline void provisioning_print_banner( void );
static int8_t provisioning_get_ap_credentials( user_data_t* udata );
static int8_t provisioning_get_xively_credentials( user_data_t* udata );
static int8_t get_uart_input_string_esp32( char* dst, size_t dst_size );

int8_t provisioning_gather_user_data( user_data_t* dst )
{
    provisioning_print_banner();

    /* Get WiFi AP Credentials from the user */
    switch ( provisioning_get_ap_credentials( dst ) )
    {
        case 1:
            printf( "\n>> Saving user data to flash" );
            if ( 0 > user_data_save_to_flash( dst ) )
                return -2;
            break;
        case 0:
            break;
        default:
            printf( "\n\t[ERROR] Getting AP credentials from user" );
            return -1;
    }

    /* Get Xively Credentials from the user */
    switch ( provisioning_get_xively_credentials( dst ) )
    {
        case 1:
            printf( "\n>> Saving user data to flash" );
            if ( 0 > user_data_save_to_flash( dst ) )
                return -2;
            break;
        case 0:
            break;
        default:
            printf( "\n\t[ERROR] Getting AP credentials from user" );
            return -1;
    }
    return 0;
}

/**
 * @brief  Gather WiFi credentials from the end user via UART
 *
 * @param [out] Credentials will be copied into this datastructure
 *
 * @retval <0: Error
 * @retval 0: User decided not to update these credentials
 * @retval 1: Credentials updated OK
 */
static int8_t provisioning_get_ap_credentials( user_data_t* udata )
{
    char yn_input_string[8] = "";
    int max_ssid_size = 0;
    int max_pwd_size = 0;

    ( ESP_WIFI_SSID_STR_SIZE > USER_DATA_STR_SIZE )
        ? ( max_ssid_size = ESP_WIFI_SSID_STR_SIZE )
        : ( max_ssid_size = USER_DATA_STR_SIZE );

    ( ESP_WIFI_PASSWORD_STR_SIZE > USER_DATA_STR_SIZE )
        ? ( max_pwd_size = ESP_WIFI_PASSWORD_STR_SIZE )
        : ( max_pwd_size = USER_DATA_STR_SIZE );

    printf( "\n|********************************************************|" );
    printf( "\n|              Gathering WiFi AP Credentials             |" );
    printf( "\n|********************************************************|" );

    printf( "\n>> Would you like to update your WiFi credentials? [y/N]: " );
    fflush( stdout );
    get_uart_input_string_esp32( yn_input_string, 8 );
    switch ( yn_input_string[0] )
    {
        case 'y':
        case 'Y':
            break;
        default:
            return 0;
    }

    printf( "\n>> Enter WiFi SSID: " );
    fflush( stdout );
    get_uart_input_string_esp32( udata->wifi_client_ssid, max_ssid_size );

    printf( "\n>> Enter WiFi Password: " );
    fflush( stdout );
    get_uart_input_string_esp32( udata->wifi_client_password, max_pwd_size );

    return 1;
}

/**
 * @brief  Gather MQTT credentials from the end user via UART
 *
 * @param [out] Credentials will be copied into this datastructure
 *
 * @retval <0: Error
 * @retval 0: User decided not to update these credentials
 * @retval 1: Credentials updated OK
 */
static int8_t provisioning_get_xively_credentials( user_data_t* udata )
{
    char yn_input_string[8] = "";

    printf( "\n|********************************************************|" );
    printf( "\n|              Gathering Xively Credentials              |" );
    printf( "\n|********************************************************|" );

    printf( "\n>> Would you like to update your MQTT credentials? [y/N]: " );
    fflush( stdout );
    get_uart_input_string_esp32( yn_input_string, 8 );
    switch ( yn_input_string[0] )
    {
        case 'y':
        case 'Y':
            break;
        default:
            return 0;
    }

    printf( "\n>> Enter the Xively Account ID: " );
    fflush( stdout );
    get_uart_input_string_esp32( udata->xi_account_id, USER_DATA_STR_SIZE );

    printf( "\n>> Enter the Xively Device ID: " );
    fflush( stdout );
    get_uart_input_string_esp32( udata->xi_device_id, USER_DATA_STR_SIZE );

    printf( "\n>> Enter the Xively Device Password: " );
    fflush( stdout );
    get_uart_input_string_esp32( udata->xi_device_password, USER_DATA_STR_SIZE );

    return 1;
}

static inline void provisioning_print_banner( void )
{
    printf( "\n|********************************************************|" );
    printf( "\n|               User Configuration Routine               |" );
    printf( "\n|               --------------------------               |" );
    printf( "\n| NOTE: You will only need to do this once.              |" );
    printf( "\n|                                                        |" );
    printf( "\n| NOTE: Next time you want to update these, keep the     |" );
    printf( "\n|       ESP32's GPIO0 button pressed during the 5        |" );
    printf( "\n|       seconds the LED is blinking rapidly during boot  |" );
    printf( "\n|                                                        |" );
    printf( "\n| WARNING: Copy-pasting long strings to your serial      |" );
    printf( "\n| -------  terminal may result in loss of characters.    |" );
    printf( "\n|          To solve this issue use a delay between       |" );
    printf( "\n|          characters in the terminal settings.          |" );
    printf( "\n|          E.g in coolTerm:                              |" );
    printf( "\n|   Options->Transmit->Use transmit character delay      |" );
    printf( "\n|********************************************************|" );
    fflush( stdout );
}

/**
 * @brief    Get a string from UART. Stop at '\n', strip out '\r' and '\n'
 * @detailed The IDF SDK hasn't ported scanf() for UART input yet, so we do it
 * manually.
 *
 * @param [out] Credentials will be copied into this datastructure
 *
 * @retval <0: Error
 * @retval 0: User decided not to update these credentials
 * @retval 1: Credentials updated OK
 */
static int8_t get_uart_input_string_esp32( char* dst, size_t dst_size )
{
    uint8_t input_char = '\0';
    for ( int i = 0; i < dst_size; i++ )
    {
        /* Poll until we get a character */
        input_char = uart_rx_one_char_block();

        printf( "%c", input_char );
        fflush( stdout );

        switch( input_char )
        {
            default:
                dst[i] = ( char )input_char;
                break;
            case '\0':
                break;
            case '\r':
                dst[i] = '\0';
                break;
            case '\n':
                dst[i] = '\0';
                goto ok_out;
        }
    }

    printf( "\n[ERROR] string overflow avoided. Input aborted" );
    dst[dst_size - 1] = '\0';
    return -1;

ok_out:
    return 0;
}
