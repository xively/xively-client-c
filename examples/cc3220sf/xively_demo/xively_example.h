/*
 *   Copyright (C) 2015-2016 Texas Instruments Incorporated
 *
 *   All rights reserved. Property of Texas Instruments Incorporated.
 *   Restricted rights to use, duplicate or disclose this code are
 *   granted through contract.
 *
 *   The program may not be used without the written permission of
 *   Texas Instruments Incorporated or against the terms and conditions
 *   stipulated in the agreement under which this program has been supplied,
 *   and under no circumstances can it be used with non-TI connectivity device.
 *   
 ******************************************************************************/

/*
 *  Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * 	This is part of the Xively C Client library,
 * 	it is licensed under the BSD 3-Clause license.
 *
 ******************************************************************************/

/* Standard includes */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Example/Board Header files */
#include <ti/drivers/net/wifi/simplelink.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>

/* POSIX Header files */
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

/* Headers from this Example */
#include "platformDrivers/bma222drv.h"
#include "platformDrivers/tmp006drv.h"
#include "platformDrivers/uart_term.h"

/* Application Version and Naming*/
#define APPLICATION_NAME        		"xively cc3220 example"
#define APPLICATION_VERSION             "1.1"

#define SPAWN_TASK_PRIORITY		(9)
#define TASK_STACK_SIZE         (2048)
#define CONTROL_STACK_SIZE		(1024)

#define WIFI_KEY_LEN_MAX		(64)
#define SSID_LEN_MAX 			(32)
#define BSSID_LEN_MAX			(6)
#define XIVELY_CRED_LEN_MAX (64)

#ifdef OOB_DEBUG_PRINT
#define INFO_PRINT	Report
#else
#define INFO_PRINT(x,...)
#endif


/* path to configuration file on device.  This contains
 * Xively connection credentials and wifi ssid and password
 * credentials.
 */
#define XIVELY_CFG_FILE  "/etc/xively_cfg.txt"


/* check the error code and handle it */
#define ASSERT_ON_ERROR(error_code)\
            {\
                 if(error_code < 0) \
                   {\
                        ERR_PRINT(error_code);\
                        return error_code;\
                 }\
            }


/* Tracks initialization state so that we can show progress with the LEDs */
typedef enum
{
	InitializationState_ReadingCredentials = 0,
	InitializationState_ConnectingToWifi,
	InitializationState_ConnectingToXively,
	InitializationState_SubscribingToTopics,
	InitializationState_Connected,
	InitializationState_ShuttingDownConnection,
	InitializationState_Restarting
} InitializationState;

/* Application specific status/error codes */
typedef enum
{
    /* Choosing -0x7D0 to avoid overlap w/ host-driver's error codes */
    AppStatusCodes_LanConnectionFailed = -0x7D0,
    AppStatusCodes_InternetConnectionFailed = AppStatusCodes_LanConnectionFailed - 1,
    AppStatusCodes_DeviceNotInStationMode = AppStatusCodes_InternetConnectionFailed - 1,
    AppStatusCodes_StatusCodeMax = -0xBB8
} AppStatusCodes;


typedef enum
{
    AppStatusBits_NwpInit = 0,           /* If this bit is set: Network Processor is powered up */
    AppStatusBits_Connection = 1,        /* If this bit is set: the device is connected to the AP or client is connected to device (AP) */
    AppStatusBits_IpLeased = 2,          /* If this bit is set: the device has leased IP to any connected client */
    AppStatusBits_IpAcquired = 3,        /* If this bit is set: the device has acquired an IP */
    AppStatusBits_SmartconfigStart = 4,  /* If this bit is set: the SmartConfiguration process is started from SmartConfig app */
    AppStatusBits_P2pDevFound = 5,       /* If this bit is set: the device (P2P mode) found any p2p-device in scan */
    AppStatusBits_P2pReqReceived = 6,    /* If this bit is set: the device (P2P mode) found any p2p-negotiation request */
    AppStatusBits_ConnectionFailed = 7,  /* If this bit is set: the device(P2P mode) connection to client(or reverse way) is failed */
    AppStatusBits_PingDone = 8,          /* If this bit is set: the device has completed the ping operation */
    AppStatusBits_Ipv6lAcquired = 9,     /* If this bit is set: the device has acquired an IPv6 address */
    AppStatusBits_Ipv6gAcquired = 10,    /* If this bit is set: the device has acquired an IPv6 address */
    AppStatusBits_AuthenticationFailed = 11,
    AppStatusBits_ResetRequired = 12,
} AppStatusBits;

typedef enum
{
    ControlMessageType_Switch2 = 2,
	ControlMessageType_Switch3 = 3,
	ControlMessageType_ResetRequired = 4,
	ControlMessageType_ControlMessagesMax = 0xFF
} ControlMessageType;

/* Control block definition */
typedef struct Application_CB_t
{
	InitializationState initializationState;  /* Tracks bootup progress */
	uint32_t  status;                         /* SimpleLink Status */
	uint32_t  gatewayIP;                      /* Network Gateway IP address */
	uint8_t   connectionSSID[SSID_LEN_MAX+1]; /* Connection SSID */
	uint8_t   ssidLen;                        /* Connection SSID */
	uint8_t   connectionBSSID[BSSID_LEN_MAX]; /* Connection BSSID */
	uint8_t   xivelyConnectionStatus;         /* Tracks if we're connected to Xively */
	uint8_t   button0State;                   /* Tracks Button 0 State */
	uint8_t   button1State;                   /* Tracks Button 1 State */

	/* BEGIN - Values parsed from Xively Config File */
	int8_t  desiredWifiSecurityType;              /* see SL_WLAN_SEC_TYPE in wlan.h */
 	char  	desiredWifiSSID[SSID_LEN_MAX+1];      /* user configured host wifi SSID */
 	char    desiredWifiKey[WIFI_KEY_LEN_MAX+1];   /* user configured wifi password */
 	char   	xivelyAccountId[XIVELY_CRED_LEN_MAX];
 	char    xivelyDeviceId[XIVELY_CRED_LEN_MAX];
 	char    xivelyDevicePassword[XIVELY_CRED_LEN_MAX];

 	sem_t   wifiConnectedSem;
	/* END */
} Application_CB;

/****************************************************************************
                      GLOBAL VARIABLES
****************************************************************************/
extern Application_CB	gApplicationControlBlock;


//****************************************************************************
//                      FUNCTION PROTOTYPES
//****************************************************************************

//*****************************************************************************
//
//! \brief This function reboot the M4 host processor
//!
//! \param[in]  none
//!
//! \return none
//!
//****************************************************************************
void cc3220Reboot(void);

void * MAIN_StartUpThread( void *pvParameters );

typedef void (connection_callback_t )( void );

void Callback_ConnectedToWiFi();
void Callback_LostWiFiConnection();


