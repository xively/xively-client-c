#ifndef __XI_BSP_TIME_STM32F4_NUCLEO_WIFI_SNTP_H__
#define __XI_BSP_TIME_STM32F4_NUCLEO_WIFI_SNTP_H__

#include <stdint.h>

typedef int32_t posix_time_t;
typedef enum {
    SNTP_INTERNAL_ERROR  = -6,
    SNTP_NOT_AVAILABLE   = -5,
    SNTP_PARSER_ERROR    = -4,
    SNTP_REQUEST_FAILURE = -3,
    SNTP_SOCKET_ERROR    = -2,
    SNTP_TIMEOUT         = -1,
    SNTP_SUCCESS         = 0,
} sntp_status_t;

extern uint8_t sntp_sock_id; //0xff means no SNTP socket ID available

/**
 * @function
 * @brief Acquires date/time from NTP service. Cycles through multiple NTP
 *        servers until date/time is acquired.
 * @param pvParameters ?
 * 
 */
sntp_status_t xi_bsp_time_sntp_init( void );


/**
 * @function 
 * @brief To be called from the received socket data callback. It validates
 *           the data and stores the SNTP response to a buffer to be processed
 */
void sntp_socket_data_callback( uint8_t socket_id, uint8_t* data_ptr,
                                uint32_t message_size, uint32_t chunk_size );

/**
 * @function
 * @brief Returns seconds since 1970/01/01 00:00:00. Must be called after a
 *        successful init()
 */
posix_time_t xi_bsp_time_sntp_getseconds_posix( void );
#endif
