#ifndef __NTP_H__
#define __NTP_H__

#define SNTP_SERVER "3.pool.ntp.org"
#define SNTP_PORT   123

#define SNTP_MSG_SIZE   48
#define SNTP_TIMEOUT_MS 5000

#define SNTP_DISCONNECTION_MSG "\x20\x02\x00\x03"

typedef enum {
    SNTP_INTERNAL_ERROR     = -6,
    SNTP_NOT_AVAILABLE      = -5,
    SNTP_PARSER_ERROR       = -4,
    SNTP_REQUEST_FAILURE    = -3,
    SNTP_CONNECTION_FAILURE = -2,
    SNTP_TIMEOUT            = -1,
    SNTP_SUCCESS            = 0,
} sntp_status_t;

typedef struct {
    uint8_t socket_id;
    char* response;
} sntp_response;

/* User API */
sntp_status_t sntp_get_datetime( uint8_t* socket_id, int32_t* epoch_time );
void sntp_socket_data_callback( uint8_t socket_id, uint8_t* data_ptr,
                                uint32_t message_size, uint32_t chunk_size );

/* Internal usage */
WiFi_Status_t sntp_connect( char* sntp_server, uint32_t sntp_port, uint8_t* sock_id );
WiFi_Status_t sntp_send_request( uint8_t sock_id );
WiFi_Status_t sntp_disconnect( uint8_t sock_id );
int32_t sntp_parse_response( char* response );
uint32_t sntp_ntohl( uint32_t n );

sntp_status_t sntp_await_response( uint8_t sock_id );

#endif
