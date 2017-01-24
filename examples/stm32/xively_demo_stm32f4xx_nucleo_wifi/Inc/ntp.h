#ifndef __NTP_H__
#define __NTP_H__

WiFi_Status_t sntp_start( uint8_t* sock_id );
WiFi_Status_t sntp_connect( char* sntp_server,
                            uint32_t sntp_port,
                            uint8_t* sock_id );
WiFi_Status_t sntp_disconnect( uint8_t sock_id );
WiFi_Status_t sntp_send_request( uint8_t sock_id );
void sntp_await_response( uint8_t sock_id );
WiFi_Status_t sntp_read_response( uint8_t sock_id, char* sntp_response );
int32_t sntp_parse_response( uint8_t* response, uint32_t msg_size );
int sntp_get_datetime(void);

//inline void print_input_buffer(void); //TODO: rm this func

#endif
