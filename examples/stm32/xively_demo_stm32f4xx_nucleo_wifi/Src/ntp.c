/*TODO: This interface is currently blocking at sntp_get_datetime() and only
 *      supports 1 concurrent request. Change last_sntp_response to use a
 *      flexible array and we can make it non-blocking and support as many
 *      concurrent connections as necessary
 */
#include "stdio.h"
#include "string.h"
#include "wifi_interface.h"
#include "wifi_module.h"
#include "wifi_globals.h"

#include "ntp.h"

int32_t sntp_current_time = 0;
sntp_response* last_sntp_response = NULL;

/**
   * @brief  free the space used by an sntp_response
   * @param  *sntp_server is a char array with the relevant URL
   * @retval WiFi_Status_t: WiFi_MODULE_SUCCESS on success, see wifi_interface.h
   */
void sntp_free_response( sntp_response* r )
{
    if( NULL != r->response )
    {
        free(r->response);
    }
    free(r);
}

/**
   * @brief  Builds a char array with a basic SNTP request to be sent via UDP
   *         Packet description:
   *           - Flags 1 byte
   *               * Leap: 3 bits
   *               * Version: 3 bits
   *               * Mode: 2 bits
   *           - Stratum 1 byte
   *           - Polling 1 byte
   *           - Precision 1 byte
   *           - Root Delay 4 bytes
   *           - Root Dispersion 4 bytes
   *           - Reference Identifier 4 bytes
   *           - Reference Timestamp 8 bytes
   *           - Origin Timestamp 8 bytes
   *           - Receive Timestamp 8 bytes
   *           - Transmit Timestamp 8 bytes
   * @param  request_buf is a pointer to a pre-allocated 48 byte char array
   * @retval void
   */
void sntp_build_request( char* request_buf )
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
   * @brief  Create a UDP socket to the SNTP server, connect to it and return
   *         its ID by setting *sock_id
   * @param  *sntp_server is a char array with the relevant URL
   *         sntp_port is the port to be used for SNTP communication
   *         *sock_id will be set to the appropriate socket ID
   * @retval WiFi_Status_t: WiFi_MODULE_SUCCESS on success, see wifi_interface.h
   */
WiFi_Status_t sntp_connect( char* sntp_server, uint32_t sntp_port, uint8_t* sock_id )
{
    WiFi_Status_t status = WiFi_MODULE_SUCCESS;
    uint8_t sntp_protocol = 'u'; //UDP
    /* Send the "Open Socket" AT command to the WiFi module */
    printf("\r\n\tStablishing UDP connection to SNTP server %s:%lu",
           sntp_server,
           sntp_port);
    status = wifi_socket_client_open((uint8_t*)sntp_server, sntp_port,
                                     &sntp_protocol, sock_id);

    return status;
}

/**
   * @brief  
   * @param  sock_id is the socket ID set by sntp_connect() in stnp_start()
   * @retval WiFi_Status_t: WiFi_MODULE_SUCCESS on success, see wifi_interface.h
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
   * @brief  Send SNTP packet to the Server. Must be called after a connection
   *         has been stablished using sntp_start()
   * @param  sock_id is the socket ID set by sntp_connect() in stnp_start()
   * @retval WiFi_Status_t: WiFi_MODULE_SUCCESS on success, see wifi_interface.h
   */
WiFi_Status_t sntp_send_request( uint8_t sock_id )
{
    WiFi_Status_t status = WiFi_MODULE_SUCCESS;
    char sntp_request[SNTP_MSG_SIZE];
    sntp_build_request(sntp_request);

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
   * @brief  Convert uint32_t from Network byte order to Host byte order.
   *         i.e. Return the reverse-endian value of the received argument.
   *         Implementation lifted from the Cube SDK's LwIP
   * @param  n is a 32bit integer in network order
   * @retval Returns a 32bit integer in host order (ready to be used by the uC)
   */
uint32_t sntp_ntohl(uint32_t n)
{
    return ((n & 0xff) << 24)      |
           ((n & 0xff00) << 8)     |
           ((n & 0xff0000UL) >> 8) |
           ((n & 0xff000000UL) >> 24);
}

/**
   * @brief  Parses SNTP response and returns current date and time
   * @param  48 byte array with the SNTP response. Length MUST be validated
   *         before calling this function
   * @retval Current date and time in epoch format, -1 on error
   */
int32_t sntp_parse_response( char* response )
{
    int32_t current_ntp_time = 0; //epoch + 2208988800
    int32_t current_epoch_time = 0;

    printf("\r\n>>Parsing SNTP response...");
    memcpy(&current_ntp_time, response+40, 4); //timestamp starts at byte 40
    current_ntp_time = sntp_ntohl(current_ntp_time); //Convert endianness
    current_epoch_time = current_ntp_time - 2208988800; //Remove NTP offset
    return current_epoch_time;
}

/**
   * @brief  Creates a UDP socket, connects to the NTP server and sends an SNTP
   *         request. The response will be received at the WiFi callback
   *         ind_wifi_socket_data_received( ... );
   * @param  sock_id is a pointer to a pre-allocated uint8_t to be filled in
   *         with the ID of the created socket. This value can be used to
   *         identify messages with the SNTP server.
   * @retval WiFi_Status_t: WiFi_MODULE_SUCCESS on success, see wifi_interface.h
   */
WiFi_Status_t sntp_start(uint8_t* sock_id)
{
    WiFi_Status_t retval = WiFi_MODULE_SUCCESS;

    retval = sntp_connect(SNTP_SERVER, SNTP_PORT, sock_id);
    if( retval != WiFi_MODULE_SUCCESS )
    {
        goto abort;
    }

    retval = sntp_send_request(*sock_id);
    if( retval != WiFi_MODULE_SUCCESS )
    {
        goto abort;
    }

    return retval;

abort:
    printf("\r\n>>SNTP abort routine initialized");
    if(*sock_id >= 0)
    {
        sntp_disconnect(*sock_id);
        *sock_id = -1;
    }

    return  retval;
}

/**
   * @brief  Create new UDP socket to SNTP server, send SNTP request, await
   *         response for up to SNTP_TIMEOUT_MS, close socket and return 
   * @param  - sock_id is a pre-allocated uint8_t pointer used to filter the WiFi
   *         API callbacks. It will be set to the socket ID returned by connect()
   *         - epoch_time will be set to the current epoch time as returned by the
   *         SNTP server
   * @retval sntp_status_t: 0 means SNTP_SUCCESS, <0 means something failed.
   *         See ntp.h to handle different failure reasons in different ways
   */
sntp_status_t sntp_get_datetime( uint8_t* sock_id, int32_t* epoch_time )
{
    sntp_status_t retval = SNTP_SUCCESS; //Returned by this function
    WiFi_Status_t wifi_retval = WiFi_MODULE_SUCCESS;

    /* Create socket */
    printf("\r\n>>Getting date and time from SNTP server...");
    wifi_retval = sntp_connect(SNTP_SERVER, SNTP_PORT, sock_id);
    if( wifi_retval != WiFi_MODULE_SUCCESS )
    {
        printf("\r\n\tSNTP socket connection [FAIL] Retval: %d", wifi_retval);
        retval = SNTP_CONNECTION_FAILURE;
        goto terminate;
    }
    else if(*sock_id < 0)
    {
        printf("\r\n\tUDP Connection [FAIL] Socket ID<0: %d", *sock_id);
        retval = SNTP_CONNECTION_FAILURE;
        goto terminate;
    }
    printf("\r\n\tUDP Connection [OK] Assigned socket ID: %d", *sock_id);

    /* Send SNTP request */
    wifi_retval = sntp_send_request(*sock_id);
    if( wifi_retval != WiFi_MODULE_SUCCESS )
    {
        printf("\r\n\tSNTP send_request [FAIL] Retval: %d", wifi_retval);
        retval = SNTP_REQUEST_FAILURE;
        goto terminate;
    }

    /* Await SNTP Response */
    printf("\r\n\tAwaiting server response");
    if( sntp_await_response(*sock_id) < 0 )
    {
        printf("\r\n\tSNTP response [FAIL] Response timed out");
        retval = SNTP_TIMEOUT;
        goto terminate;
    }

    if( *sock_id != last_sntp_response->socket_id )
    {
        printf("\r\n\tSocket ID assertion [FAIL] Bug in ntp.c?");
        retval = SNTP_INTERNAL_ERROR;
        goto terminate;
    }

    /* Parse server response */
    printf("\r\n\tParsing and converting SNTP response");
    sntp_current_time = sntp_parse_response(last_sntp_response->response);
    if( sntp_current_time < 0 )
    { 
        printf("\r\n\tSNTP Parsing and conversion [FAIL]");
        retval = SNTP_PARSER_ERROR;
    }
    else
    {
        printf("\r\n\tReceived epoch time: %ld", sntp_current_time);
        retval = SNTP_SUCCESS;
    }

terminate:
    /* free() SNTP Response */
    if( NULL != last_sntp_response )
    {
        sntp_free_response(last_sntp_response);
        last_sntp_response = NULL;
    }

    /* Close socket */
    if( *sock_id > 0 )
    {
        sntp_disconnect(*sock_id);
    }
    return retval;
}

/**
   * @brief  
   * @param  sock_id we're interested in
   * @retval SNTP_SUCCESS (0) or SNTP_TIMEOUT(-1)
   */
sntp_status_t sntp_await_response( uint8_t sock_id )
{
    int32_t sntp_timeout_ms = SNTP_TIMEOUT_MS;
    const int32_t timeout_step = 250;
    while( NULL == last_sntp_response )
    {
        if( sntp_timeout_ms <= 0 )
        {
            return SNTP_TIMEOUT;
        }
        printf(".");
        HAL_Delay(timeout_step);
        sntp_timeout_ms -= timeout_step;
    }
    return SNTP_SUCCESS;
}

/**
   * @brief  This function shall be called from ind_wifi_socket_data_received(),
   *         when the sock_id is the one we got from sntp_start(*sock_id)
   * @param  See description for ind_wifi_socket_data_received
   * @retval None
   */
void sntp_socket_data_callback(uint8_t sock_id, uint8_t* data_ptr,
                               uint32_t message_size, uint32_t chunk_size)
{
    /* Verify we got an SNTP response, not just protocol data */
    if( (SNTP_MSG_SIZE != message_size) || (SNTP_MSG_SIZE != chunk_size) )
    {
        printf("\r\n\tMessage isn't an SNTP respone. It will be ignored");
        return;
    }

    /* Store SNTP response */
    if( NULL != last_sntp_response )
    {
        sntp_free_response(last_sntp_response);
    }
    last_sntp_response = calloc(1, sizeof(last_sntp_response));
    last_sntp_response->response = calloc(SNTP_MSG_SIZE, sizeof(char));
    memcpy(last_sntp_response->response, (char*)data_ptr, SNTP_MSG_SIZE);
    last_sntp_response->socket_id = sock_id;

}
