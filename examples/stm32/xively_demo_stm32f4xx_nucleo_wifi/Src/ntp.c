#include "stdio.h"
#include "string.h"
#include "wifi_interface.h"
#include "wifi_module.h"
#include "wifi_globals.h"

#include "ntp.h"

#define SNTP_MSG_SIZE 48
#define MAX_AT_CMD_SIZE 128

#define SNTP_SERVER "3.pool.ntp.org"
#define SNTP_PORT   123

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
    char sntp_request[SNTP_MSG_SIZE+1];

    /* Build SNTP request message - Most of the message can be left as 0 */
    memset(sntp_request, 0, SNTP_MSG_SIZE+1);
    sntp_request[0] = '\xe3';
    sntp_request[1] = '\x00';
    sntp_request[2] = '\x03';
    sntp_request[3] = '\xfa';
    sntp_request[4] = '\x00';
    sntp_request[5] = '\x01';
    sntp_request[9] = '\x01';
    sntp_request[40] = '\xd5';
    sntp_request[41] = '\x22';
    sntp_request[42] = '\x0e';
    sntp_request[43] = '\x35';
    sntp_request[44] = '\xb8';
    sntp_request[45] = '\x76';
    sntp_request[46] = '\xab';
    sntp_request[47] = '\xea';
    sntp_request[SNTP_MSG_SIZE] = '\0';

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
   * @brief  
   * @param  
   * @retval 
   */
WiFi_Status_t sntp_read_response( uint8_t sock_id, char* sntp_response )
{
    WiFi_Status_t status = WiFi_MODULE_SUCCESS;
    printf("\r\n>>Reading SNTP response from the server");
    //status = Socket_Read(SNTP_MSG_SIZE);
    Socket_Pending_Data();
    status = Socket_Read(SNTP_MSG_SIZE);
    if(status != WiFi_MODULE_SUCCESS)
    {
        printf("\r\n\tUDP Socket Read [FAIL] Status code: %d", status);
        return status;
    }
    else
    {
        printf("\r\n\tUDP Socket Read [OK]");
        /* Print the received data */
        //printf("\r\n\tSocket Read returned %d bytes of data: ",
        //       WiFi_Counter_Variables.curr_DataLength);
        //for(uint8_t i=0; i<WiFi_Counter_Variables.curr_DataLength; i++)
        //{
        //    printf("0x%02x ", *(WiFi_Counter_Variables.curr_data+i));
        //}
        ///* Store SNTP response */
        //memcpy(sntp_response, WiFi_Counter_Variables.curr_data, SNTP_MSG_SIZE);
        //sntp_response[SNTP_MSG_SIZE] = '\0';
    }
    return status;
}

/**
   * @brief  Parses SNTP response and returns current date and time
   * @param  48 byte array with the SNTP response
   * @retval Current date and time in epoch format
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
    current_epoch_time = current_ntp_time - 2208988800;
    printf("\r\n\tParsed NTP time: %ld", current_ntp_time);

    printf("\r\n>>Calculating epoch time...");
    printf("\r\n\tepoch time: %ld", current_epoch_time);
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

    Socket_Pending_Data();
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

#if 0
/**
   * @brief  
   * @param  
   * @retval 
   */
//TODO: Return values in this function are all sorts of inconsistent
int sntp_get_datetime(void)
{
    /* SNTP Variables */
    char sntp_response[SNTP_MSG_SIZE+1];
    memset(sntp_response, 0, SNTP_MSG_SIZE+1);

    /* Socket Variables */
    uint8_t socket_id = 0; //Will be filled in on connection success

    /* Connect to SNTP server */
    if(sntp_connect(SNTP_SERVER, SNTP_PORT, &socket_id) != WiFi_MODULE_SUCCESS)
    {
        goto abort;
    }
    /* Send SNTP request */
    if(sntp_send_request(socket_id) != WiFi_MODULE_SUCCESS)
    {
        goto abort;
    }
    /* Read the response from the SNTP server */
    if(sntp_read_response(socket_id, sntp_response) != WiFi_MODULE_SUCCESS)
    {
        goto abort;
    }
    /* Close the UDP Socket */
    if(sntp_disconnect(socket_id) != WiFi_MODULE_SUCCESS)
    {
        return 0; //TODO: Remove sth else. this is WiFi_MODULE_SUCCESS
    }
    return sntp_parse_response((uint8_t*)sntp_response, SNTP_MSG_SIZE);

abort:
    printf("\r\n>>SNTP abort routine initialized");
    if(socket_id != 0)
    {
        sntp_disconnect(socket_id);
    }
    /* Return WiFi status - TODO: Return something else? the datetime? */
    return 0; //TODO: Remove sth else. this is WiFi_MODULE_SUCCESS
}
#endif

#if 0
//TODO: Remove this function; for testing purposes only
inline void print_input_buffer(void)
{
    printf("\r\n>>===============[SNTP Response Buffer?]==================<<");
    printf("\r\n>>== curr_DataLength: %d", WiFi_Counter_Variables.curr_DataLength);
    printf("\r\n>>== curr_data:       ");
    for(uint8_t i=0; i<WiFi_Counter_Variables.curr_DataLength; i++)
    {
        printf("0x%02x ", *(WiFi_Counter_Variables.curr_data+i));
    }
    printf("\r\n>>==");
    printf("\r\n>>== sockon_id_user:  %d", WiFi_Counter_Variables.sockon_id_user);
    printf("\r\n>>== message_size:    %lu", WiFi_Counter_Variables.message_size);
    printf("\r\n>>== chunk_size:      %lu", WiFi_Counter_Variables.chunk_size);
    printf("\r\n>>================[---------------------]=====================<<");
}
#endif

#if 0
/**
   * @brief  Callback function for incoming data
   * @param  
   * @retval 
   */
void ind_wifi_socket_data_received( uint8_t socket_id,
                                    uint8_t * data_ptr,
                                    uint32_t message_size,
                                    uint32_t chunk_size )
{
    printf("\r\n!!!!!!!!!!!!!!!!!!!!!!!!!!");
    printf("\r\n>>Received new socket data. Message size [%ld], Chunk size [%ld]",
           message_size, chunk_size);
    printf("\r\n\tData: ");
    for(uint8_t i=0; i<chunk_size; i++)
    {
        printf("0x%02x ", *(data_ptr+i));
    }
    printf("\r\n00000000000000000000000000");
}
#endif
