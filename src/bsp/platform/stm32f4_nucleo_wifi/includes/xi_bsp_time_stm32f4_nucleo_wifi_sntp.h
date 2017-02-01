#ifndef __NTP_H__
#define __NTP_H__

#define SNTP_SERVER "3.pool.ntp.org"
#define SNTP_PORT   123

typedef enum {
    SNTP_INTERNAL_ERROR  = -6,
    SNTP_NOT_AVAILABLE   = -5,
    SNTP_PARSER_ERROR    = -4,
    SNTP_REQUEST_FAILURE = -3,
    SNTP_SOCKET_ERROR    = -2,
    SNTP_TIMEOUT         = -1,
    SNTP_SUCCESS         = 0,
} sntp_status_t;

sntp_status_t sntp_get_datetime( uint8_t* socket_id, int32_t* epoch_time );
void sntp_socket_data_callback( uint8_t socket_id, uint8_t* data_ptr,
                                uint32_t message_size, uint32_t chunk_size );

#endif
