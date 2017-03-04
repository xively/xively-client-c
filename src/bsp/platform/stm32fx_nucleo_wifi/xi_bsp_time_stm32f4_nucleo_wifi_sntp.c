#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "xi_bsp_debug.h"
#include "wifi_interface.h"
#include "xi_bsp_time_stm32f4_nucleo_wifi_sntp.h"

#define SNTP_MSG_SIZE 48
#define SNTP_TIMEOUT_MS 5000
#define SNTP_SERVER_TIME_OFFSET 2208988800
#define SNTP_RESPONSE_TIMESTAMP_OFFSET 40
#define SNTP_DISCONNECTION_MSG  "\x20\x02\x00\x03"

typedef struct {
    uint8_t socket_id;
    char* response;
} sntp_response_t;

sntp_response_t* last_sntp_response = NULL;
wifi_bool sntp_awaiting_response = WIFI_FALSE;
int32_t sntp_current_time = 0;
uint8_t sntp_sock_id = 0xff; //Default to 'no socket'

/**
   * Packet description:
   *  - Flags 1 byte
   *      * Leap: 3 bits
   *      * Version: 3 bits
   *      * Mode: 2 bits
   *  - Stratum 1 byte
   *  - Polling 1 byte
   *  - Precision 1 byte
   *  - Root Delay 4 bytes
   *  - Root Dispersion 4 bytes
   *  - Reference Identifier 4 bytes
   *  - Reference Timestamp 8 bytes
   *  - Origin Timestamp 8 bytes
   *  - Receive Timestamp 8 bytes
   *  - Transmit Timestamp 8 bytes
   */
static const char SNTP_REQUEST[SNTP_MSG_SIZE] = {
0xe3, 0x00, 0x03, 0xfa, 0x00, 0x01, 0x00, 0x00,
0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xd5, 0x22, 0x0e, 0x35, 0xb8, 0x76, 0xab, 0xea};

extern void HAL_Delay(uint32_t delay_ms);
static WiFi_Status_t    sntp_start( uint32_t sntp_port, uint8_t* sock_id );
static WiFi_Status_t    sntp_send_request( uint8_t sock_id );
static sntp_status_t    sntp_await_response( uint8_t sock_id );
static sntp_response_t* sntp_malloc_response( void );
static int32_t          sntp_parse_response( char* response );
static void             sntp_free_response( sntp_response_t** r );
static WiFi_Status_t    sntp_stop( uint8_t sock_id );
static uint32_t sntp_ntohl( uint32_t n );

/**
 * @function
 * @brief Returns epoch time (seconds since 1970/01/01 00:00:00)
 * @retval epoch time as an int32_t
 */
posix_time_t xi_bsp_time_sntp_getseconds_posix( void )
{
    return sntp_current_time;
}

/**
   * @brief  initialize a new sntp_response_t. malloc() the struct and the
   *         48 byte char* response inside it.
   * @param  None
   * @retval WiFi_Status_t: WiFi_MODULE_SUCCESS on success, see wifi_interface.h
   */
static sntp_response_t* sntp_malloc_response( void )
{
    sntp_response_t* new_response = NULL;
    new_response = malloc(sizeof(sntp_response_t));
    if( NULL == new_response )
    {
        goto malloc_error;
    }
    new_response->socket_id = -1; //Default to invalid socket ID
    new_response->response = malloc(SNTP_MSG_SIZE);
    if( NULL == new_response->response )
    {
        goto malloc_error;
    }
    memset(new_response->response, 0, sizeof(sntp_response_t));
    return new_response;

malloc_error:
    sntp_free_response(&new_response);
    return NULL;
}

/**
   * @brief  free the space used by an sntp_response_t and set it to NULL
   * @param  **sntp_response_t we want to free()
   * @retval None
   */
static void sntp_free_response( sntp_response_t** r )
{
    if( NULL == r )
    {
        return;
    }
    if( NULL == *r )
    {
        return;
    }
    if( NULL != (*r)->response )
    {
        free((*r)->response);
    }
    free(*r);
    *r = NULL;
}

/**
   * @brief  Create a UDP socket to the SNTP server and return its ID by
   *         setting *sock_id
   * @param  *sntp_server is a char array with the relevant URL
   *         sntp_port is the port to be used for SNTP communication
   *         *sock_id will be set to the appropriate socket ID
   * @retval WiFi_Status_t: WiFi_MODULE_SUCCESS on success, see wifi_interface.h
   *
   * @TODO: sntp.h should have a list of SNTP servers. This function should
   *               rotate through them, changing to the next one every time this
   *               function is called
   */
static WiFi_Status_t sntp_start( uint32_t sntp_port, uint8_t* sock_id )
{
    WiFi_Status_t status = WiFi_MODULE_SUCCESS;
    uint8_t sntp_protocol = 'u'; //UDP
    if( NULL == sock_id )
    {
        xi_bsp_debug_logger("[ERROR] NULL pointer received as SNTP sock_id");
        return WiFi_START_FAILED_ERROR;
    }
    xi_bsp_debug_format("Opening UDP socket to SNTP server %s:%lu",
           SNTP_SERVER,
           sntp_port);
    status = wifi_socket_client_open((uint8_t*)SNTP_SERVER, sntp_port,
                                     &sntp_protocol, sock_id);

    return status;
}

/**
   * @brief
   * @param  sock_id is the socket ID set by sntp_start() in stnp_start()
   * @retval WiFi_Status_t: WiFi_MODULE_SUCCESS on success, see wifi_interface.h
   */
static WiFi_Status_t sntp_stop( uint8_t sock_id )
{
    WiFi_Status_t status = WiFi_MODULE_SUCCESS;
    xi_bsp_debug_logger("Closing UDP socket to SNTP server");
    status = wifi_socket_client_close(sock_id);
    if(status != WiFi_MODULE_SUCCESS)
    {
        xi_bsp_debug_format("UDP Socket close [ERROR] Status code: %d", status);
        return status;
    }
    return status;
}

/**
   * @brief  Send SNTP packet to the Server. Must be called after a socket has
   *         has been created from xi_bsp_time_sntp_init()->sntp_start()
   * @param  sock_id is the socket ID set by sntp_start()
   * @retval WiFi_Status_t: WiFi_MODULE_SUCCESS on success, see wifi_interface.h
   */
static WiFi_Status_t sntp_send_request( uint8_t sock_id )
{
    WiFi_Status_t status = WiFi_MODULE_SUCCESS;

    status = wifi_socket_client_write(sock_id, SNTP_MSG_SIZE, (char*)SNTP_REQUEST);

    if(status != WiFi_MODULE_SUCCESS)
    {
        xi_bsp_debug_format("SNTP Message Write [FAIL] Status code: %d", status);
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
static uint32_t sntp_ntohl(uint32_t n)
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
static int32_t sntp_parse_response( char* response )
{
    int32_t current_ntp_time = 0;
    int32_t current_epoch_time = 0;
    if( NULL == response )
    {
        xi_bsp_debug_logger("[ERROR] Got a NULL pointer as SNTP response");
        return -1;
    }

    memcpy(&current_ntp_time, response+SNTP_RESPONSE_TIMESTAMP_OFFSET, sizeof(int32_t));
    current_ntp_time = sntp_ntohl(current_ntp_time);
    current_epoch_time = current_ntp_time - SNTP_SERVER_TIME_OFFSET;
    return current_epoch_time;
}

/**
   * @brief  Create new UDP socket to SNTP server, send SNTP request, await
   *         response for up to SNTP_TIMEOUT_MS, close socket and return
   * @param  - sock_id is a pre-allocated uint8_t pointer used to filter the WiFi
   *         API callbacks. It will be set to the socket ID returned by open()
   *         - epoch_time will be set to the current epoch time as returned by the
   *         SNTP server
   * @retval sntp_status_t: 0 means SNTP_SUCCESS, <0 means something failed.
   *         See ntp.h to handle different failure reasons in different ways
   */
sntp_status_t xi_bsp_time_sntp_init( void )
{
    uint8_t sock_id;
    sntp_status_t retval = SNTP_SUCCESS; //Returned by this function
    WiFi_Status_t wifi_retval = WiFi_MODULE_SUCCESS;

    /* Create socket */
    xi_bsp_debug_logger("Getting date and time from SNTP server");
    wifi_retval = sntp_start(SNTP_PORT, &sock_id);
    if( wifi_retval != WiFi_MODULE_SUCCESS )
    {
        xi_bsp_debug_format("SNTP socket creation [FAIL] Retval: %d", wifi_retval);
        return SNTP_SOCKET_ERROR;
    }
    sntp_sock_id = sock_id;

    /* Send SNTP request */
    wifi_retval = sntp_send_request(sock_id);
    if( wifi_retval != WiFi_MODULE_SUCCESS )
    {
        xi_bsp_debug_format("SNTP send_request [FAIL] Retval: %d", wifi_retval);
        retval = SNTP_REQUEST_FAILURE;
        goto terminate;
    }
    sntp_awaiting_response = WIFI_TRUE;

    /* Await SNTP Response */
    if( sntp_await_response(sock_id) < 0 )
    {
        xi_bsp_debug_logger("SNTP [ERROR] Response timed out");
        retval = SNTP_TIMEOUT;
        goto terminate;
    }

    if( sock_id != last_sntp_response->socket_id )
    {
        xi_bsp_debug_logger("Socket ID assertion [FAIL]");
        retval = SNTP_INTERNAL_ERROR;
        goto terminate;
    }

    /* Parse server response */
    sntp_current_time = sntp_parse_response(last_sntp_response->response);
    if( sntp_current_time < 0 )
    {
        xi_bsp_debug_logger("SNTP Parsing and conversion [FAIL]");
        retval = SNTP_PARSER_ERROR;
    }
    else
    {
        xi_bsp_debug_format("Received epoch time: %ld", sntp_current_time);
        retval = SNTP_SUCCESS;
    }

terminate:
    sntp_free_response(&last_sntp_response);

    /* Close socket */
    sntp_stop(sock_id);
    sntp_sock_id = 0xff;
    return retval;
}

/**
   * @brief
   * @param  sock_id we're interested in
   * @retval SNTP_SUCCESS (0) or SNTP_TIMEOUT(-1)
   */
static sntp_status_t sntp_await_response( uint8_t sock_id )
{
    (void)sock_id;
    int32_t sntp_timeout_ms = SNTP_TIMEOUT_MS;
    const int32_t timeout_step = 250;
    while( NULL == last_sntp_response )
    {
        if( sntp_timeout_ms <= 0 )
        {
            sntp_awaiting_response = WIFI_FALSE;
            return SNTP_TIMEOUT;
        }
        HAL_Delay(timeout_step);
        sntp_timeout_ms -= timeout_step;
    }
    return SNTP_SUCCESS;
}

/**
   * @brief  This function shall be called from ind_wifi_socket_data_received(),
   *         when the sock_id is the one we got from xi_bsp_time_sntp_init()
   * @param  See description for ind_wifi_socket_data_received
   * @retval None
   */
void sntp_socket_data_callback(uint8_t sock_id, uint8_t* data_ptr,
                               uint32_t message_size, uint32_t chunk_size)
{
    if( NULL == data_ptr )
    {
        xi_bsp_debug_logger("[ERROR] Got NULL pointer as socket message");
        return;
    }
    /* Verify we got an SNTP response, not just protocol data */
    if( (SNTP_MSG_SIZE != message_size) || (SNTP_MSG_SIZE != chunk_size) )
    {
        xi_bsp_debug_logger("Message isn't an SNTP respone. It will be ignored");
        return;
    }

    if( WIFI_FALSE == sntp_awaiting_response )
    {
        xi_bsp_debug_logger("Got an [UNEXPECTED] SNTP response. It will be ignored");
        return;
    }
    sntp_awaiting_response = WIFI_FALSE;

    /* Store SNTP response */
    sntp_free_response(&last_sntp_response);
    last_sntp_response = sntp_malloc_response();
    if( NULL == last_sntp_response )
    {
        xi_bsp_debug_logger("Memory allocation for SNTP response [FAIL] Msg ignored");
        return;
    }
    memcpy(last_sntp_response->response, (char*)data_ptr, SNTP_MSG_SIZE);
    last_sntp_response->socket_id = sock_id;
}
