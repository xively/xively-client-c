#include "stdio.h"
#include "string.h"
#include "wifi_interface.h"
#include "wifi_module.h"
#include "wifi_globals.h"

#include "ntp.h"

uint32_t sntp_ntohl(uint32_t n)
{
    return ((n & 0xff) << 24)      |
           ((n & 0xff00) << 8)     |
           ((n & 0xff0000UL) >> 8) |
           ((n & 0xff000000UL) >> 24);
}

/**
   * @brief  Builds a char array with a basic SNTP request to be sent via UDP
             Packet description:
               - Flags 1 byte
                   * Leap: 3 bits
                   * Version: 3 bits
                   * Mode: 2 bits
               - Stratum 1 byte
               - Polling 1 byte
               - Precision 1 byte
               - Root Delay 4 bytes
               - Root Dispersion 4 bytes
               - Reference Identifier 4 bytes
               - Reference Timestamp 8 bytes
               - Origin Timestamp 8 bytes
               - Receive Timestamp 8 bytes
               - Transmit Timestamp 8 bytes
   * @param  request_buf is a pointer to a pre-allocated 48 byte char array
   * @retval void
   */
void build_sntp_request( char* request_buf )
{
    char sntp_pkt[SNTP_MSG_SIZE] = {
    0xe3, 0x00, 0x03, 0xfa, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xd5, 0x22, 0x0e, 0x35, 0xb8, 0x76, 0xab, 0xea};
    memset(request_buf, 0, SNTP_MSG_SIZE);
    memcpy(request_buf, sntp_pkt, SNTP_MSG_SIZE);
}

/**
   * @brief  
   * @param  
   * @retval 
   */
WiFi_Status_t sntp_connect( char* sntp_server, uint32_t sntp_port, uint8_t* sock_id )
{
    WiFi_Status_t status = WiFi_MODULE_SUCCESS;
    uint8_t sntp_protocol = 'u'; //UDP
    /* Send the "Open Socket" AT command to the WiFi module */
    printf("\r\n>>Stablishing UDP connection to SNTP server %s:%lu",
           sntp_server,
           sntp_port);
    status = wifi_socket_client_open((uint8_t*)sntp_server, sntp_port,
                                     &sntp_protocol, sock_id);

    if(status != WiFi_MODULE_SUCCESS)
    {
        printf("\r\n\tUDP Connection [FAIL] Status code: %d", status);
    }
    else if(*sock_id < 0)
    {
        printf("\r\n\tUDP Connection [FAIL] Returned Socket ID: %d", *sock_id);
    }
    else
    {
        printf("\r\n\tUDP Connection [OK] Assigned socket ID: %d", *sock_id);
    }
    return status;
}

/**
   * @brief  
   * @param  
   * @retval 
   */
WiFi_Status_t sntp_disconnect( uint8_t sock_id )
{
    WiFi_Status_t status = WiFi_MODULE_SUCCESS;
    printf("\r\n>>Closing UDP connection to SNTP server...");
    status = wifi_socket_client_close(sock_id);
    if(status != WiFi_MODULE_SUCCESS)
    {
        printf("\r\n\tUDP Socket Close [FAIL] Status code: %d", status);
        return status;
    }
    else
    {
        printf("\r\n\tUDP Socket Close [OK]");
    }
    return status;
}

/**
   * @brief  
   * @param  
   * @retval 
   */
WiFi_Status_t sntp_send_request( uint8_t sock_id )
{
    WiFi_Status_t status = WiFi_MODULE_SUCCESS;
    char sntp_request[SNTP_MSG_SIZE];
    build_sntp_request(sntp_request);

    /* Send the "Socket Write" AT command to the WiFi module */
    printf("\r\n>>Sending SNTP request to the server");
    status = wifi_socket_client_write(sock_id, SNTP_MSG_SIZE, sntp_request);

    if(status != WiFi_MODULE_SUCCESS)
    {
        printf("\r\n\tSNTP Message Write [FAIL] Status code: %d", status);
    }
    else
    {
        printf("\r\n\tSNTP Message Write [OK]");
    }
    return status;
}

/**
   * @brief  
   * @param  
   * @retval 
   */
//TODO: What to do with this func? I don't understand Socket_Pending_Data...
void sntp_await_response( uint8_t sock_id )
{
    Socket_Pending_Data();
}

/**
   * @brief  Parses SNTP response and returns current date and time
   * @param  48 byte array with the SNTP response
   * @retval Current date and time in epoch format, -1 on error
   */
int32_t sntp_parse_response( uint8_t* response, uint32_t msg_size )
{
    int32_t current_ntp_time = 0; //epoch + 2208988800
    int32_t current_epoch_time = 0;

    if(msg_size != SNTP_MSG_SIZE)
    {
        printf("\r\n>>ERROR: Message size mismatch in sntp_parse_response. ");
        printf("Expected [%d] bytes, received [%lu]", SNTP_MSG_SIZE, msg_size);
        return -1;
    }

    printf("\r\n>>Parsing SNTP response...");
    memcpy(&current_ntp_time, response+40, 4);
    current_ntp_time = sntp_ntohl(current_ntp_time);
    current_epoch_time = current_ntp_time - 2208988800;
    return current_epoch_time;
}

/**
   * @brief  
   * @param  
   * @retval 
   */
WiFi_Status_t sntp_start(uint8_t* sock_id)
{
    /* Connect to SNTP server */
    if(sntp_connect(SNTP_SERVER, SNTP_PORT, sock_id) != WiFi_MODULE_SUCCESS)
    {
        goto abort;
    }

    /* Send SNTP request */
    if(sntp_send_request(*sock_id) != WiFi_MODULE_SUCCESS)
    {
        goto abort;
    }

    return WiFi_MODULE_SUCCESS;

abort:
    printf("\r\n>>SNTP abort routine initialized");
    if(*sock_id >= 0)
    {
        sntp_disconnect(*sock_id);
        *sock_id = -1;
    }
    /* Return WiFi status - TODO: Return something else? the datetime? */
    return 0;
}
