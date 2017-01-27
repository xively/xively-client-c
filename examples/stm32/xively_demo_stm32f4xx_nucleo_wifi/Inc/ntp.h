#ifndef __NTP_H__
#define __NTP_H__

#define SNTP_SERVER "3.pool.ntp.org"
#define SNTP_PORT   123

#define SNTP_MSG_SIZE 48
#define MAX_AT_CMD_SIZE 128

WiFi_Status_t sntp_start( uint8_t* sock_id );
int32_t sntp_parse_response( uint8_t* response, uint32_t msg_size );

WiFi_Status_t sntp_connect( char* sntp_server,
                            uint32_t sntp_port,
                            uint8_t* sock_id );
WiFi_Status_t sntp_disconnect( uint8_t sock_id );
WiFi_Status_t sntp_send_request( uint8_t sock_id );
void sntp_await_response( uint8_t sock_id );
int sntp_get_datetime(void);

#endif
