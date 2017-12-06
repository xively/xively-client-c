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
 */


//*****************************************************************************
//
//! \addtogroup xively_example
//! @{
//
//*****************************************************************************

/* standard includes */
#include <stdlib.h>
#include <string.h>

/* driverlib Header files */
#include <ti/devices/cc32xx/inc/hw_types.h>
#include <ti/devices/cc32xx/inc/hw_memmap.h>
#include <ti/devices/cc32xx/driverlib/rom.h>
#include <ti/devices/cc32xx/driverlib/rom_map.h>
#include <ti/devices/cc32xx/driverlib/prcm.h>
#include <ti/devices/cc32xx/driverlib/timer.h>

/* TI-DRIVERS Header files */
#include <board.h>
#include <platformDrivers/platform.h>
#include <platformDrivers/uart_term.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/net/wifi/simplelink.h>
#include <ti/drivers/net/wifi/wlan.h>

/* Example/Board Header files */
#include "platformDrivers/CC3220SF_LAUNCHXL.h"
#include "Board.h"

/* POSIX Header files */
#include <pthread.h>
#include <simplelinkWifi/provisioning_task.h>
#include <time.h>

#include <xi_bsp_rng.h>
#include <xi_bsp_time.h>
#include <xively.h>
#include <xively_example.h>



#define ASYNC_EVT_TIMEOUT               	(5000)  /* In msecs. Used to detect good/bad sl_start() */
#define PROFILE_ASYNC_EVT_TIMEOUT			(5)	/*In sec. Used for connecting to stored profile */
#define LED_TOGGLE_CONFIRMATION_TIMEOUT     (200)   /* In msecs */


#define PROVISIONING_INACTIVITY_TIMEOUT		(600)	/* Provisioning inactivity timeout in seconds */

#define ROLE_SELECTION_BY_SL     				(0xFF)

/*!
 *  \brief  Provisioning modes
 */
typedef enum
{
    PrvsnMode_AP,       /* AP provisioning (AP role) */
    PrvsnMode_SC,       /* Smart Config provisioning (STA role) */
    PrvsnMode_APSC      /* AP + Smart Config provisioning (AP role) */

}PrvsnMode;

/** By default, setting the provisioning mode to AP + Smart Config.
  * Other values could be PrvsnMode_SC or PrvsnMode_AP
  */
#define PROVISIONING_MODE   PrvsnMode_APSC

/*!
 *  \brief  Provisioning status
 */
typedef enum
{
    PrvsnStatus_Stopped,
    PrvsnStatus_InProgress

}PrvsnStatus;

/*
 *  \brief  Application state's context
 */
typedef struct	_Provisioning_AppContext_t_
{
    PrvnState  currentState;       /* Current state of provisioning */
    _u32        pendingEvents;      /* Events pending to be processed */

    _u8             role;               /* SimpleLink's role - STATION/AP/P2P */

    _u8             defaultRole;        /* SimpleLink's default role, try not to change this */
    PrvsnMode     provisioningMode;   /* Provisioning Mode */
    PrvsnStatus   provisioningStatus; /* */

    _u32            asyncEvtTimeout;    /* Timeout value*/

    _u32        ledToggleTimeout;   /* Timeout value */

}Provisioning_AppContext;



/*!
 *  \brief  Function pointer to the event handler
 */
typedef _i32 (*fptr_EventHandler)();

/*!
 *  \brief  Entry in the lookup table
 */
typedef struct
{
    fptr_EventHandler   p_evtHndl;  /* Pointer to the event handler */
    PrvnState          nextState;  /* Next state of provisioning */

}s_TblEntry;



/****************************************************************************
                      LOCAL FUNCTION PROTOTYPES
****************************************************************************/

//*****************************************************************************
//
//! \brief This function initializes provisioning process
//!
//! \param[in]  None
//!
//! \return None
//!
//****************************************************************************
void provisioningInit();

//*****************************************************************************
//
//! \brief This function starts provisioning process
//!
//! \param[in]  None
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
_i32 provisioningStart();

//*****************************************************************************
//
//! \brief main provisioning loop
//!
//! \param[in]  None
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
static _i32 provisioningAppTask();

//*****************************************************************************
//! \brief This function puts the device in its default state. It:
//!           - Set the mode to AP
//!           - Configures connection policy to Auto
//!           - Deletes all the stored profiles
//!           - Enables DHCP
//!		  - Disable IPV6
//!           - Disables Scan policy
//!           - Sets Tx power to maximum
//!           - Sets power policy to normal
//!           - Unregister mDNS services
//!           - Remove all filters
//!
//! \param   none
//! \return  On success, zero is returned. On error, negative is returned
//*****************************************************************************
static _i32 ConfigureSimpleLinkToDefaultState();


//*****************************************************************************
//
//! \brief This function starts the SimpleLink in the configured role.
//!		 The device notifies the host asynchronously when the initialization is completed
//!
//! \param[in]  role	Device shall be configured in this role
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************

static _i32 InitSimplelink(_u8 const role);


//*****************************************************************************
//
//! \brief This function handles 'APP_EVENT_STARTED' event
//!
//! \param[in]  None
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
static _i32 HandleStrtdEvt();

//*****************************************************************************
//
//! \brief This function starts the async-event timer
//!
//! \param[in]  None
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
static _i32 StartAsyncEvtTimer(_u32 timeout);


 //*****************************************************************************
//
//! \brief This function stops the async-event timer
//!
//! \param[in]  None
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
static _i32 StopAsyncEvtTimer();

//*****************************************************************************
//
//! \brief internal error detection during provisioning process
//!
//! \param[in]  None
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
static _i32 ReportError();

//*****************************************************************************
//
//! \brief internal report current state during provisioning process
//!
//! \param[in]  None
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
static _i32 ReportSM();

//*****************************************************************************
//
//! \brief steps following a successful provisioning process
//!
//! \param[in]  None
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
static _i32 ReportSuccess();

//*****************************************************************************
//
//! \brief wait for connection following a successful provisioning process
//!
//! \param[in]  None
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
static _i32 WaitForConn();


//*****************************************************************************
//
//! \brief checks for local link connecion to peer device
//!		when set as Station, checks for AP connection
//!		when set as Access Point, no check is required
//!
//! \param[out]  deviceRole		role of the device
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
static _i32 validateLocalLinkConnection(SlWlanMode_e *deviceRole);

//*****************************************************************************
//
//! Notify if device return to factory image
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void NotifyReturnToFactoryImage();



/****************************************************************************
                      GLOBAL VARIABLES
****************************************************************************/
/*!
 *  \brief  Application state's context
 */
Provisioning_AppContext gAppCtx;

timer_t gAsyncEventTimer;
timer_t gLedTimer;


/*!
 *  \brief   Application lookup/transition table
 */
const s_TblEntry gProvisioningTransitionTable[PrvnState_Max][PrvnEvent_Max] =
{
	/* PrvnState_Init */
	{
		/* Event: PrvnEvent_Triggered */
		{provisioningStart      , PrvnState_Idle			            },
        /* Event: PrvnEvent_Started */
        {ReportError         	, PrvnState_Error    					},
		/* Event: PrvnEvent_StartFailed */
        {ReportError         	, PrvnState_Error				    	},
		/* Event: PrvnEvent_ConfirmationSuccess */
        {ReportSM            , PrvnState_Completed                      },		/* special case where confirmation is received after application is restarted and NWP is still in provisioning */
        																		/* in this case, need to move to COMPLETED state */
		/* Event: PrvnEvent_ConfirmationFailed */
        {ReportError            , PrvnState_Error                      },
		/* Event: PrvnEvent_Stopped */
        {ReportSM            , PrvnState_Init		                },		/* in case of auto provisioning */
		/* Event: PrvnEvent_WaitForConn */
		{ReportSM         			, PrvnState_Error                      	},
        /* Event: PrvnEvent_Timeout */
        {ReportError            , PrvnState_Error		               	},
        /* Event: PrvnEvent_Error */
        {ReportError       		, PrvnState_Error                    	},
    },

    /* PrvnState_Idle */
    {
		/* Event: PrvnEvent_Triggered */
		{ReportError      , PrvnState_Error			            },
        /* Event: PrvnEvent_Started */
        {HandleStrtdEvt         , PrvnState_WaitForConfirmation    	},
		/* Event: PrvnEvent_StartFailed */
        {ReportError         	, PrvnState_Error				    	},
		/* Event: PrvnEvent_ConfirmationSuccess */
        {ReportError            , PrvnState_Error                      },
		/* Event: PrvnEvent_ConfirmationFailed */
        {ReportError            , PrvnState_Error                      },
		/* Event: PrvnEvent_Stopped */
        {ReportSM            	, PrvnState_Init                      	},
		/* Event: PrvnEvent_WaitForConn */
		{ReportSM         			, PrvnState_Error                      	},
        /* Event: PrvnEvent_Timeout */
        {ReportError            , PrvnState_Error		               	},
        /* Event: PrvnEvent_Error */
        {ReportError       		, PrvnState_Error                    	},
    },

	/* PrvnState_WaitForConfirmation */
	{
		/* Event: PrvnEvent_Triggered */
		{ReportError      , PrvnState_Error			            },
		/* Event: PrvnEvent_Started */
		{ReportError         	, PrvnState_Error    					},
		/* Event: PrvnEvent_StartFailed */
		{ReportSM         	, PrvnState_Idle				    	},
		/* Event: PrvnEvent_ConfirmationSuccess */
		{ReportSM            	, PrvnState_Completed                  },
		/* Event: PrvnEvent_ConfirmationFailed */
		{ReportSM            	, PrvnState_WaitForConfirmation     },
		/* Event: PrvnEvent_Stopped */
		{ReportSM            	, PrvnState_Init                       },
		/* Event: PrvnEvent_WaitForConn */
		{ReportSM         			, PrvnState_Error                      	},
		/* Event: PrvnEvent_Timeout */
		{ReportError            , PrvnState_Error		               	},
		/* Event: PrvnEvent_Error */
		{ReportError       		, PrvnState_Error                    	},
	},

	/* PrvnState_Completed */
	{
		/* Event: PrvnEvent_Triggered */
		{ReportError      , PrvnState_Error			            },
		/* Event: PrvnEvent_Started */
		{ReportError         	, PrvnState_Error    					},
		/* Event: PrvnEvent_StartFailed */
		{ReportError         	, PrvnState_Error				    	},
		/* Event: PrvnEvent_ConfirmationSuccess */
		{ReportError            , PrvnState_Error                  	},
		/* Event: PrvnEvent_ConfirmationFailed */
		{ReportError            , PrvnState_Error					    },
		/* Event: PrvnEvent_Stopped */
		{ReportSuccess         	, PrvnState_Init                      	},
		/* Event: PrvnEvent_WaitForConn */
		{WaitForConn         	, PrvnState_Init                      	},	/* this state should cover cases where feedback failed but profile exists */
		/* Event: PrvnEvent_Timeout */
		{ReportError            , PrvnState_Error		               	},
		/* Event: PrvnEvent_Error */
		{ReportError       		, PrvnState_Error                    	},
	},

	/* PrvnState_Error */
	{
		/* Event: PrvnEvent_Triggered */
		{ReportError      , PrvnState_Error			            },
		/* Event: PrvnEvent_Started */
		{ReportError         	  , PrvnState_Error    				},
		/* Event: PrvnEvent_StartFailed */
		{ReportError         	  , PrvnState_Error				   	},
		/* Event: PrvnEvent_ConfirmationSuccess */
		{ReportError              , PrvnState_Error                  	},
		/* Event: PrvnEvent_ConfirmationFailed */
		{ReportError              , PrvnState_Error				    },
		/* Event: PrvnEvent_Stopped */
		{ReportError              , PrvnState_Error                   	},
		/* Event: PrvnEvent_WaitForConn */
		{ReportSM         			, PrvnState_Error                      	},
		/* Event: PrvnEvent_Timeout */
		{ReportError              , PrvnState_Error		           	},
		/* Event: PrvnEvent_Error */
		{ReportError       		  , PrvnState_Error                  	},
	}
};


Provisioning_CB	Provisioning_ControlBlock;


/*****************************************************************************
                  Callback Functions
*****************************************************************************/

//*****************************************************************************
//
//! \brief The device init callback
//!
//! \param[in]  status	Mode the device is configured in
//!
//! \return None
//!
//****************************************************************************
void SimpleLinkInitCallback(_u32 status, SlDeviceInitInfo_t *DeviceInitInfo)
{
    Provisioning_AppContext *const pCtx = &gAppCtx;

    UART_PRINT("[Provisioning task] Device started in %s role\n\r", (0 == status) ? "Station" :\
                                               ((2 == status) ? "AP" : "P2P"));

    /** While provisioning is ongoing, the appropriate role is choosen by the
      * device itself, and host can remain agnostic to these details
      */
    if(pCtx->role == ROLE_SELECTION_BY_SL)
    {
        SignalProvisioningEvent(PrvnEvent_Started);
    }
    else
    {
        /* Either trigger an error/started event here */
        if(pCtx->role == status)
        {
            SignalProvisioningEvent(PrvnEvent_Started);
        }
        else
        {
            UART_PRINT("[Provisioning task] But the intended role is %s \n\r",\
                                        (0 == pCtx->role) ? "Station" :\
                                        ((2 == pCtx->role) ? "AP" : "P2P"));
            SignalProvisioningEvent(PrvnEvent_Error);
        }
    }
}


//*****************************************************************************
//
//! \brief The interrupt handler for the async-evt timer
//!
//! \param[in]  None
//!
//! \return None
//!
//****************************************************************************
void AsyncEvtTimerIntHandler(sigval val)
{
	/* Clear the timer interrupt. */
	Platform_TimerInterruptClear();

    /* One Shot */
    /* TBD - Check role for One-shot/periodic */
    StopAsyncEvtTimer();
    SignalProvisioningEvent(PrvnEvent_Timeout);
}

//*****************************************************************************
//
//! \brief The interrupt handler for the LED timer
//!
//! \param[in]  None
//!
//! \return None
//!
//****************************************************************************
void LedTimerIntHandler(sigval val)
{
	/* Clear the timer interrupt. */
	Platform_TimerInterruptClear();

//	static uint8_t ledIndexOn = 0;
//	static uint8_t ledNames[] = { CC3220SF_LAUNCHXL_LED_D5, CC3220SF_LAUNCHXL_LED_D6, CC3220SF_LAUNCHXL_LED_D7 };
//	static uint8_t ledNamesSize = sizeof(ledNames) / sizeof( uint8_t );
//
//    GPIO_write(ledNames[ledIndexOn], Board_LED_OFF);
//
//    ledIndexOn = ++ledIndexOn % ledNamesSize;
//    GPIO_write(ledNames[ledIndexOn], Board_LED_ON);
}

//*****************************************************************************
//                 Local Functions
//*****************************************************************************

//*****************************************************************************
//
//! \brief This function initializes provisioning process
//!
//! \param[in]  None
//!
//! \return None
//!
//****************************************************************************
void provisioningInit()
{
	Provisioning_AppContext *const pCtx = &gAppCtx;

	Platform_TimerInit(AsyncEvtTimerIntHandler, &gAsyncEventTimer);

    /** By default, setting the provisioning mode to AP + Smart Config.
	  * Other values could be PrvsnMode_SC or PrvsnMode_AP
	  */
	pCtx->provisioningMode = PROVISIONING_MODE;
	switch(pCtx->provisioningMode)
	{
		case PrvsnMode_APSC: pCtx->defaultRole = ROLE_AP;
		break;

		case PrvsnMode_AP: pCtx->defaultRole = ROLE_AP;
		break;

		case PrvsnMode_SC: pCtx->defaultRole = ROLE_STA;
		break;
	}

	/* Provisioning has not started yet */
	pCtx->provisioningStatus = PrvsnStatus_Stopped;
	pCtx->currentState = PrvnState_Init;

}


//*****************************************************************************
//
//! \brief This function starts provisioning process
//!
//! \param[in]  None
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
_i32 provisioningStart()
{
	_i32		retVal = 0;
	Provisioning_AppContext *const pCtx = &gAppCtx;
	SlDeviceVersion_t ver = {0};
	_u8  configOpt = 0;
	_u16 configLen = 0;

	/* check if provisioning is running */
	/* if auto provisioning - the command stops it automatically */
	/* in case host triggered provisioning - need to stop it explicitly */
	configOpt = SL_DEVICE_GENERAL_VERSION;
	configLen = sizeof(ver);
	retVal = sl_DeviceGet(SL_DEVICE_GENERAL, &configOpt, &configLen, (_u8 *)(&ver));
	if(SL_RET_CODE_PROVISIONING_IN_PROGRESS == retVal)
	{
		UART_PRINT("[Provisioning task] Provisioning is already running, stopping it...\r\n");
		retVal = sl_WlanProvisioning(SL_WLAN_PROVISIONING_CMD_STOP,ROLE_STA,NULL,NULL,NULL);

		/* return  SL_RET_CODE_PROVISIONING_IN_PROGRESS to indicate the SM to stay in the same state*/
		return SL_RET_CODE_PROVISIONING_IN_PROGRESS;
	}

    retVal = ConfigureSimpleLinkToDefaultState();

    if(retVal < 0)
    {
        UART_PRINT("[Provisioning task] Failed to configure the device in its default state \n\r");
        return retVal;
    }

    UART_PRINT("[Provisioning task] Device is configured in default state \n\r");

	/* Provisioning has not started yet */
	pCtx->provisioningStatus = PrvsnStatus_Stopped;

	retVal = InitSimplelink(pCtx->defaultRole);
	if(retVal < 0)
	{
		UART_PRINT("[Provisioning task] Failed to initialize the device\n\r");
		return retVal;
	}

	return retVal;
}

//*****************************************************************************
//
//! \brief main provisioning loop
//!
//! \param[in]  None
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
static _i32 provisioningAppTask()
{
    Provisioning_AppContext *const pCtx = &gAppCtx;
    s_TblEntry   *pEntry = NULL;
    _u16 eventIdx = 0;
    _i32	retVal = 0;

    for(eventIdx = 0; eventIdx < PrvnEvent_Max; eventIdx++)
    {
        if(0 != (pCtx->pendingEvents & (1 << eventIdx)))
        {
            if (eventIdx != PrvnEvent_Triggered)
            {
                /** Events received - Stop the respective timer if its still
                  * running
                  */
                StopAsyncEvtTimer();

            }

            pEntry = (s_TblEntry *)&gProvisioningTransitionTable[pCtx->currentState][eventIdx];
            if (NULL != pEntry->p_evtHndl)
            {
                retVal = pEntry->p_evtHndl();
                if (retVal == SL_RET_CODE_PROVISIONING_IN_PROGRESS)		/* no state transition is required */
                {
                	pCtx->pendingEvents &= ~(1 << eventIdx);
                	continue;
                }
                else if (retVal < 0)
                {
                    UART_PRINT("[Provisioning task] Event handler failed, error=%d\n\r", retVal);
                    while (1)		/* this is to let other tasks recover by mcu reset, e.g. in case of switching to AP mode */
                    	{
                    		usleep(1000);
                    	}
                }
            }

            if (pEntry->nextState != pCtx->currentState)
            {
                pCtx->currentState = pEntry->nextState;
            }

            pCtx->pendingEvents &= ~(1 << eventIdx);
        }

        /* No more events to handle. Break.! */
        if(0 == pCtx->pendingEvents)
	{
		break;
        }
    }

    usleep(1000);

    return 0;
}



//*****************************************************************************
//! \brief This function puts the device in its default state. It:
//!           - Set the mode to AP
//!           - Configures connection policy to Auto
//!           - Deletes all the stored profiles
//!           - Enables DHCP
//!		  - Disable IPV6
//!           - Disables Scan policy
//!           - Sets Tx power to maximum
//!           - Sets power policy to normal
//!           - Unregister mDNS services
//!           - Remove all filters
//!
//! \param   none
//! \return  On success, zero is returned. On error, negative is returned
//*****************************************************************************
static _i32
ConfigureSimpleLinkToDefaultState()
{
    SlWlanRxFilterOperationCommandBuff_t  RxFilterIdMask = {0};

    _u8 	ucConfigOpt = 0;
    _u16 	ifBitmap  = 0;
    _u8 	ucPower = 0;

    _i32 			ret = -1;
    _i32 			mode = -1;

    /* Start Simplelink - Blocking mode */
    mode = sl_Start(0, 0, 0);
    if (SL_RET_CODE_DEV_ALREADY_STARTED != mode)
	{
		ASSERT_ON_ERROR(mode);
	}

    /* If the device is not in AP mode, try configuring it in AP mode
     in case device is already started (got SL_RET_CODE_DEV_ALREADY_STARTED error code), then mode would remain -1 and in this case we do not know the role. Move to AP role anyway	*/
    if (ROLE_AP != mode)
    {

        /* Switch to AP role and restart */
        ret = sl_WlanSetMode(ROLE_AP);
        ASSERT_ON_ERROR(ret);

        ret = sl_Stop( Board_SL_STOP_TIMEOUT );
        ASSERT_ON_ERROR(ret);

        ret = sl_Start(0, 0, 0);
        ASSERT_ON_ERROR(ret);

        /* Check if the device is in AP again */
        if (ROLE_AP != ret)
        {
            return ret;
        }
    }

    SlWlanSecParams_t g_SecParams;
    g_SecParams.Key = (signed char*) gApplicationControlBlock.desiredWifiKey;
    g_SecParams.KeyLen = strlen( (const char*) gApplicationControlBlock.desiredWifiKey );
    g_SecParams.Type = gApplicationControlBlock.desiredWifiSecurityType;
    ret = sl_WlanProfileAdd(
        ( const signed char* )gApplicationControlBlock.desiredWifiSSID,
        strlen( gApplicationControlBlock.desiredWifiSSID ), 0, &g_SecParams, 0, 1, 0 );
    ASSERT_ON_ERROR(ret);

    /* Set connection policy to Auto (no AutoProvisioning)  */
    ret = sl_WlanPolicySet(SL_WLAN_POLICY_CONNECTION,
                                SL_WLAN_CONNECTION_POLICY(1, 0, 0, 0), NULL, 0);
    ASSERT_ON_ERROR(ret);

    /* Remove all profiles */
    ret = sl_WlanProfileDel(0xFF);
    ASSERT_ON_ERROR(ret);

    /* Enable DHCP client */
    ret = sl_NetCfgSet(SL_NETCFG_IPV4_STA_ADDR_MODE,SL_NETCFG_ADDR_DHCP,0,0);
    ASSERT_ON_ERROR(ret);

    /* Disable IPV6 */
    ifBitmap = 0;
	ret = sl_NetCfgSet(SL_NETCFG_IF, SL_NETCFG_IF_STATE, sizeof(ifBitmap), (_u8 *)&ifBitmap);
	ASSERT_ON_ERROR(ret);


    /* Disable scan */
    ucConfigOpt = SL_WLAN_SCAN_POLICY(0, 0);
    ret = sl_WlanPolicySet(SL_WLAN_POLICY_SCAN , ucConfigOpt, NULL, 0);
    ASSERT_ON_ERROR(ret);

    /* Set Tx power level for station mode
     Number between 0-15, as dB offset from max power - 0 will set max power */
    ucPower = 0;
    ret = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID,
    					SL_WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1, (_u8 *)&ucPower);
    ASSERT_ON_ERROR(ret);

    /* Set PM policy to normal */
    ret = sl_WlanPolicySet(SL_WLAN_POLICY_PM , SL_WLAN_NORMAL_POLICY, NULL, 0);
    ASSERT_ON_ERROR(ret);

    /* Unregister mDNS services */
    ret = sl_NetAppMDNSUnRegisterService(0, 0, 0);
    ASSERT_ON_ERROR(ret);

    /* Remove  all 64 filters (8*8) */
    memset(RxFilterIdMask.FilterBitmap, 0xFF, 8);
    ret = sl_WlanSet(SL_WLAN_RX_FILTERS_ID,
    		   SL_WLAN_RX_FILTER_REMOVE,
        	   sizeof(SlWlanRxFilterOperationCommandBuff_t),
    		   (_u8 *)&RxFilterIdMask);
    ASSERT_ON_ERROR(ret);

    ret = sl_Stop(Board_SL_STOP_TIMEOUT);
    ASSERT_ON_ERROR(ret);

    return ret;
}


//*****************************************************************************
//
//! \brief This function starts the SimpleLink in the configured role.
//!		 The device notifies the host asynchronously when the initialization is completed
//!
//! \param[in]  role	Device shall be configured in this role
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
static _i32 InitSimplelink(_u8 const role)
{
    Provisioning_AppContext *const pCtx = &gAppCtx;
    _i32 retVal = -1;

    pCtx->role = role;
    pCtx->pendingEvents = 0;

    retVal = sl_Start(0, 0, SimpleLinkInitCallback);
    ASSERT_ON_ERROR(retVal);

    /* Start timer */
    pCtx->asyncEvtTimeout = ASYNC_EVT_TIMEOUT;
    retVal = StartAsyncEvtTimer(pCtx->asyncEvtTimeout);
    ASSERT_ON_ERROR(retVal);

    return retVal;
}


//*****************************************************************************
//
//! \brief This function handles 'APP_EVENT_STARTED' event
//!
//! \param[in]  None
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************

static _i32 HandleStrtdEvt()
{
    Provisioning_AppContext *const pCtx = &gAppCtx;
    _i32 retVal     = 0;

    /** If provisioning has already started, don't do anything here
      * The state-machine shall keep waiting for the provisioning status
      */
    if(PrvsnStatus_Stopped == pCtx->provisioningStatus)
    {
        SlDeviceVersion_t   firmwareVersion = {0};

        _u8  ucConfigOpt = 0;
        _u16 ucConfigLen = 0;

		/* Get the device's version-information */
		ucConfigOpt = SL_DEVICE_GENERAL_VERSION;
		ucConfigLen = sizeof(firmwareVersion);
		retVal = sl_DeviceGet(SL_DEVICE_GENERAL, &ucConfigOpt,\
						   &ucConfigLen, (unsigned char *)(&firmwareVersion));
		ASSERT_ON_ERROR(retVal);

        UART_PRINT("[Provisioning task] Host Driver Version: %s\n\r", SL_DRIVER_VERSION);
        UART_PRINT("[Provisioning task] Build Version %d.%d.%d.%d.31.%d.%d.%d.%d.%d.%d.%d.%d\n\r",\
                        firmwareVersion.NwpVersion[0],  \
                        firmwareVersion.NwpVersion[1],  \
                        firmwareVersion.NwpVersion[2],  \
                        firmwareVersion.NwpVersion[3],  \
                        firmwareVersion.FwVersion[0],   \
                        firmwareVersion.FwVersion[1],   \
                        firmwareVersion.FwVersion[2],   \
                        firmwareVersion.FwVersion[3],   \
                        firmwareVersion.PhyVersion[0],  \
                        firmwareVersion.PhyVersion[1],  \
                        firmwareVersion.PhyVersion[2],  \
                        firmwareVersion.PhyVersion[3]);

        /* Start provisioning process */
        UART_PRINT("[Provisioning task] Starting Provisioning - ");
        UART_PRINT("[Provisioning task] in mode %d (0 = AP, 1 = SC, 2 = AP+SC)\r\n",\
                                                    pCtx->provisioningMode);

        retVal = sl_WlanProvisioning(pCtx->provisioningMode, ROLE_STA,\
                                     PROVISIONING_INACTIVITY_TIMEOUT, NULL,NULL);
        ASSERT_ON_ERROR(retVal);

        pCtx->provisioningStatus = PrvsnStatus_InProgress;
        UART_PRINT("[Provisioning task] Provisioning Started. Waiting to be provisioned..!! \r\n");
    }

    return retVal;
}


//*****************************************************************************
//
//! \brief This function starts the async-event timer
//!
//! \param[in]  None
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
static _i32 StartAsyncEvtTimer(_u32 timeout)
{
    Provisioning_AppContext *const pCtx = &gAppCtx;

    pCtx->asyncEvtTimeout = timeout;
    Platform_TimerStart(pCtx->asyncEvtTimeout, gAsyncEventTimer, 0);
    return 0;
}

//*****************************************************************************
//
//! \brief This function stops the async-event timer
//!
//! \param[in]  None
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
static _i32 StopAsyncEvtTimer()
{
    Provisioning_AppContext *const pCtx = &gAppCtx;

    if(0 != pCtx->asyncEvtTimeout)
    {
        Platform_TimerStop(gAsyncEventTimer);
        pCtx->asyncEvtTimeout = 0;
    }

    return 0;
}


//*****************************************************************************
//
//! \brief internal error detection during provisioning process
//!
//! \param[in]  None
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
static _i32 ReportError()
{
    Provisioning_AppContext *const pCtx = &gAppCtx;
    _u16 eventIdx = 0;

    for(eventIdx = 0; eventIdx < PrvnEvent_Max; eventIdx++)
    {
        if(0 != (pCtx->pendingEvents & (1 << eventIdx)))
	{
		break;
        }
    }

    UART_PRINT("[Provisioning task] Unexpected SM: State = %d, Event = %d\n\r",\
                                    pCtx->currentState, eventIdx);
    return -1;
}

//*****************************************************************************
//
//! \brief internal report current state during provisioning process
//!
//! \param[in]  None
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
static _i32 ReportSM()
{
    Provisioning_AppContext *const pCtx = &gAppCtx;
    _u16 eventIdx = 0;

    for(eventIdx = 0; eventIdx < PrvnEvent_Max; eventIdx++)
    {
        if(0 != (pCtx->pendingEvents & (1 << eventIdx)))
	{
		break;
        }
    }

    if (PrvnEvent_Stopped == eventIdx)
    {
    	StopLedEvtTimer();
    	GPIO_write(CC3220SF_LAUNCHXL_LED_D5, Board_LED_OFF);
    }

    return 0;
}

//*****************************************************************************
//
//! \brief steps following a successful provisioning process
//!
//! \param[in]  None
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
static _i32 ReportSuccess()
{
    Provisioning_AppContext *const pCtx = &gAppCtx;
    _u16 ConfigOpt;
	_u16 ipLen;
	SlNetCfgIpV4Args_t ipV4 = {0};
    _i32 retVal;

    UART_PRINT("[Provisioning task] Provisioning completed successfully..!\n\r");
    pCtx->provisioningStatus = PrvsnStatus_Stopped;
    //StopLedEvtTimer();

    /* Get the device's IP address */
	ipLen = sizeof(SlNetCfgIpV4Args_t);
	ConfigOpt = 0;
	retVal = sl_NetCfgGet(SL_NETCFG_IPV4_STA_ADDR_MODE,&ConfigOpt,&ipLen,(_u8 *)&ipV4);
	if (retVal == 0)
	{
		UART_PRINT("[Provisioning task] IP address is %d.%d.%d.%d\n\r",\
					SL_IPV4_BYTE(ipV4.Ip,3),  \
					SL_IPV4_BYTE(ipV4.Ip,2),  \
					SL_IPV4_BYTE(ipV4.Ip,1),  \
					SL_IPV4_BYTE(ipV4.Ip,0));
	}


	/* signal to linklocal task */
	Report("!! Report Success signalling provisioningDoneSignal\n\r");
	sem_post(&Provisioning_ControlBlock.provisioningDoneSignal);

    return 0;
}


//*****************************************************************************
//
//! \brief wait for connection following a successful provisioning process
//!
//! \param[in]  None
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
static _i32 WaitForConn()
{
    Provisioning_AppContext *const pCtx = &gAppCtx;
    struct timespec ts;
    _i32 retVal;

    while (((!IS_IPV6L_ACQUIRED(gApplicationControlBlock.status) ||
			!IS_IPV6G_ACQUIRED(gApplicationControlBlock.status)) &&
			!IS_IP_ACQUIRED(gApplicationControlBlock.status)) ||
			!IS_CONNECTED(gApplicationControlBlock.status))
	{
		clock_gettime( CLOCK_REALTIME, &ts);
		ts.tv_sec += PROFILE_ASYNC_EVT_TIMEOUT;

		retVal = sem_timedwait(&Provisioning_ControlBlock.connectionAsyncEvent, &ts);
		if((retVal == 116) || (retVal == -1))	/* freertos return -1 in case of timeout */
		{
			UART_PRINT("[Provisioning task] Cannot connect to AP or profile does not exist\n\r");
			//GPIO_write(CC3220SF_LAUNCHXL_LED_D5, Board_LED_OFF);
			pCtx->currentState = PrvnState_Init;	/* this state is set so that PrvnEvent_Triggered would invoke provisioning again */
			SignalProvisioningEvent(PrvnEvent_Triggered);

			return 0;
		}
	}

	UART_PRINT("[Provisioning task] Connection to AP succeeded\n\r");

	return ReportSuccess();
}


//*****************************************************************************
//
//! \brief checks for local link connection to peer device
//!		when set as Station, checks for AP connection
//!		when set as Access Point, no check is required
//!
//! \param[out]  deviceRole		role of the device
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
static _i32 validateLocalLinkConnection(SlWlanMode_e *deviceRole)
{
	_u16 ConfigOpt;
	_u16 ipLen;
	SlNetCfgIpV4Args_t ipV4 = {0};
	_i32 retVal;
	_u32 ocpRegVal;

	struct timespec ts;

	retVal = sl_Start(0, 0, 0);
	if (SL_ERROR_CALIB_FAIL == retVal)	/* when calibration fails, reboot is required */
	{
		Report("calibration failed\n\r");
		cc3220Reboot();
	}
	else if ( retVal != SL_RET_CODE_DEV_ALREADY_STARTED )
	{
		Report("Asserting error: %d\n\r", retVal);
		ASSERT_ON_ERROR(retVal);
	}

	/* if in AP role
	 * 1) check OCP register value
	 * 2) if set, it means user set AP mode via switch.
	 *  	 check for IP_ACQUIRED to indicate NWP is running
	 * 4) if not set, procede with STATION role
	 */
	if (retVal == ROLE_AP)
	{
		*deviceRole = ROLE_AP;
		ocpRegVal = MAP_PRCMOCRRegisterRead(OCP_REGISTER_INDEX);
		ocpRegVal &= (1<<OCP_REGISTER_OFFSET);
		if (ocpRegVal)
		{
			if (IS_IP_ACQUIRED(gApplicationControlBlock.status))
			{
				return 0;
			}
			else
			{
				clock_gettime( CLOCK_REALTIME, &ts);
       			ts.tv_sec += PROFILE_ASYNC_EVT_TIMEOUT;

				retVal = sem_timedwait(&Provisioning_ControlBlock.connectionAsyncEvent, &ts);
				if((retVal == 116) || (retVal == -1))	/* freertos return -1 in case of timeout */
				{
					UART_PRINT("[Provisioning task] AP role failed to initialize\n\r");
					//GPIO_write(CC3220SF_LAUNCHXL_LED_D5, Board_LED_OFF);
					retVal = -1;
				}
				return retVal;
			}
		}
	}

	if(retVal != ROLE_STA)
	{
		retVal = sl_WlanSetMode(ROLE_STA);
		ASSERT_ON_ERROR(retVal);

		retVal = sl_Stop(Board_SL_STOP_TIMEOUT);
		ASSERT_ON_ERROR(retVal);

		retVal = sl_Start(0, 0, 0);
		if (retVal < 0 || (retVal != ROLE_STA))
		{
			ASSERT_ON_ERROR(retVal);
		}
		UART_PRINT("[Provisioning task] Device started as STATION \n\r");
	}

	*deviceRole = ROLE_STA;

	SlWlanSecParams_t g_SecParams;
    g_SecParams.Key = (signed char*)gApplicationControlBlock.desiredWifiKey;
    g_SecParams.KeyLen = strlen( (const char*) g_SecParams.Key );
    g_SecParams.Type = gApplicationControlBlock.desiredWifiSecurityType;
    sl_WlanProfileAdd( (const signed char*)gApplicationControlBlock.desiredWifiSSID,
    				   strlen(gApplicationControlBlock.desiredWifiSSID),
					   0, &g_SecParams, 0, 1, 0 );


	while (((!IS_IPV6L_ACQUIRED(gApplicationControlBlock.status) ||
			!IS_IPV6G_ACQUIRED(gApplicationControlBlock.status)) &&
			!IS_IP_ACQUIRED(gApplicationControlBlock.status)) ||
			!IS_CONNECTED(gApplicationControlBlock.status))
	{
		UART_PRINT(".");
		clock_gettime( CLOCK_REALTIME, &ts);
       	ts.tv_sec += PROFILE_ASYNC_EVT_TIMEOUT;

		retVal = sem_timedwait(&Provisioning_ControlBlock.connectionAsyncEvent, &ts);
		if((retVal == 116) || (retVal == -1))	/* freertos return -1 in case of timeout */
		{
			UART_PRINT("retVal: %d\n", retVal);
			UART_PRINT("[Provisioning task] Cannot connect to AP or profile does not exist\n\r");
			//GPIO_write(CC3220SF_LAUNCHXL_LED_D5, Board_LED_OFF);
			retVal = -1;

			return retVal;
		}
	}

	UART_PRINT("[Provisioning task] Connection to AP succeeded\n\r");

	/* Get the device's IP address */
	ipLen = sizeof(SlNetCfgIpV4Args_t);
	ConfigOpt = 0;
	sl_NetCfgGet(SL_NETCFG_IPV4_STA_ADDR_MODE,&ConfigOpt,&ipLen,(_u8 *)&ipV4);
	if (retVal < 0)
	{
		return retVal;
	}

	UART_PRINT("[Provisioning task] IP address is %d.%d.%d.%d\n\r",\
			SL_IPV4_BYTE(ipV4.Ip,3),  \
			SL_IPV4_BYTE(ipV4.Ip,2),  \
			SL_IPV4_BYTE(ipV4.Ip,1),  \
			SL_IPV4_BYTE(ipV4.Ip,0));

    return 0;
}

//*****************************************************************************
//
//! Notify if device return to factory image
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void NotifyReturnToFactoryImage()
{
    if(((HWREG(HIB3P3_BASE + 0x00000418) & (1<<7)) !=0) &&
        ((HWREG(0x4402F0C8) &0x01) != 0))
    {
        UART_PRINT("Return To Factory Image successful, Do a power cycle(POR)"
                    " of the device using switch SW1-Reset\n\r");
        while(1);
    }
}


//****************************************************************************
//                            MAIN FUNCTION
//****************************************************************************

//*****************************************************************************
//
//! \brief This function signals the application events
//!
//! \param[in]  None
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
_i16 SignalProvisioningEvent(PrvnEvent event)
{
    Provisioning_AppContext *const pCtx = &gAppCtx;
    pCtx->pendingEvents |= (1 << event);

    return 0;
}

//*****************************************************************************
//
//! \brief This function gets the current provisioning state
//!
//! \param[in]  None
//!
//! \return provisioning state
//!
//****************************************************************************
PrvnState GetProvisioningState()
{
	Provisioning_AppContext *const pCtx = &gAppCtx;

	return pCtx->currentState;
}


//*****************************************************************************
//
//! \brief This function starts the led toggling timer
//!
//! \param[in]  None
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
_i32 StartLedEvtTimer(_u32 timeout)
{
    Provisioning_AppContext *const pCtx = &gAppCtx;

    pCtx->ledToggleTimeout = timeout;
	Platform_TimerStart(pCtx->ledToggleTimeout, gLedTimer, 1);

    return 0;
}

//*****************************************************************************
//
//! \brief This function stops the led toggling timer
//!
//! \param[in]  None
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
_i32 StopLedEvtTimer()
{
    Provisioning_AppContext *const pCtx = &gAppCtx;

	if(0 != pCtx->ledToggleTimeout)
    {
        Platform_TimerStop(gLedTimer);
        pCtx->ledToggleTimeout = 0;

        GPIO_write(CC3220SF_LAUNCHXL_LED_D5, Board_LED_OFF);
        GPIO_write(CC3220SF_LAUNCHXL_LED_D6, Board_LED_OFF);
        GPIO_write(CC3220SF_LAUNCHXL_LED_D7, Board_LED_OFF);
    }

    return 0;
}


//*****************************************************************************
//
//! \brief This function stops provisioning process
//!
//! \param[in]  None
//!
//! \return SL_RET_CODE_PROVISIONING_IN_PROGRESS if provisioning was running, otherwise 0
//!
//****************************************************************************
_i32 provisioningStop()
{
	Provisioning_AppContext *const pCtx = &gAppCtx;
	_i32						retVal;
	PrvnState 				provisioningState;
	SlDeviceVersion_t ver = {0};
	_u8  configOpt = 0;
	_u16 configLen = 0;

	/* check if provisioning is running */
	configOpt = SL_DEVICE_GENERAL_VERSION;
	configLen = sizeof(ver);
	retVal = sl_DeviceGet(SL_DEVICE_GENERAL, &configOpt, &configLen, (_u8 *)(&ver));
	if(SL_RET_CODE_PROVISIONING_IN_PROGRESS == retVal)
	{
		UART_PRINT("[Provisioning task]  Provisioning is already running, stopping it...\r\n");
		retVal = sl_WlanProvisioning(SL_WLAN_PROVISIONING_CMD_STOP,ROLE_STA,NULL,NULL,NULL);

		/* wait for the stopped event to arrive - wait for PrvnState_Init */
		do
		{
			provisioningState = GetProvisioningState();
			usleep(1000);
		} while (provisioningState != PrvnState_Init);

		pCtx->provisioningStatus = PrvsnStatus_Stopped;

		retVal = SL_RET_CODE_PROVISIONING_IN_PROGRESS;
	}
	else if (retVal < 0)
	{
		return retVal;
	}
	else
	{
		retVal = 0;
	}

	return retVal;
}



//*****************************************************************************
//
//! \brief This is the main provisioning task
//!
//! \param[in]  None
//!
//! \return None
//!
//****************************************************************************
void * provisioningTask(void *pvParameters)
{
	_i32			retVal = -1;
	Provisioning_AppContext 	*const pCtx = &gAppCtx;
	SlWlanMode_e 	deviceRole;

	/* Check the wakeup source. If first time entry or wakeup from HIB */
	if(MAP_PRCMSysResetCauseGet() == 0)
	{
		UART_PRINT("[Provisioning task] Wake up on Power ON\n\r");
	}
	else if(MAP_PRCMSysResetCauseGet() == PRCM_HIB_EXIT)
	{
		UART_PRINT("[Provisioning task] Woken up from Hibernate\n\r");
	}

	/* initialize one-time parameters for provisioning */
	provisioningInit();

	/* Setup LEDs to flash so we know something is happening */
	Platform_TimerInit(LedTimerIntHandler, &gLedTimer);
	pCtx->ledToggleTimeout = LED_TOGGLE_CONFIRMATION_TIMEOUT;

	StartLedEvtTimer(pCtx->ledToggleTimeout);

	/* check whether return-to-default occured */
	NotifyReturnToFactoryImage();

	/* 1) try to connect to a stored profile if in STATION mode
	 * 2) if successful, procede and validate ota bundle and then try to connect to cloud
	 * 3) if unsuccessful when in STATION mode, procede to provisioning
	 * 4) if in AP mode, procede and validate ota bundle
	 */
	retVal = validateLocalLinkConnection(&deviceRole);

	/* On Wifi at this point */
	StopLedEvtTimer();

	/* at this point, provisioning has not started yet, unless auto provisioning is running */
	/* in this case, if provisioning from mobile app is running, it would not be possible to send most of the commands to the device */
	/* need to stop provisioning */
	provisioningStop();

	if (deviceRole == ROLE_STA)
	{
		/* it means a connection to AP has been established, no need to trigger provisioning */
		if (retVal == 0)
		{
			/* signal to linklocal task */
			sem_post(&Provisioning_ControlBlock.provisioningDoneSignal);
		}
		/* it means a connection to AP failed, trigger provisioning */
		else if (retVal < 0)
		{
			SignalProvisioningEvent(PrvnEvent_Triggered);
		}
	}

	if ((retVal == 0) && (deviceRole == ROLE_AP))		/* it means device is initialized as AP, no need to trigger provisioning */
	{
		/* signal to linklocal task */
		sem_post(&Provisioning_ControlBlock.provisioningDoneSignal);
	}

    do
	{
        retVal = provisioningAppTask();

    }while(!retVal); /* Exit on failure */

	return(0);
}

//*****************************************************************************
//
//! \brief This function is registered as netapp request callback
//!
//! \param[in]  pNetAppRequest		netapp request structure
//!
//! \param[out]  pNetAppResponse	netapp response structure
//!
//! \return none
//!
//****************************************************************************
void SimpleLinkNetAppRequestEventHandler(SlNetAppRequest_t *pNetAppRequest, SlNetAppResponse_t *pNetAppResponse)
{

}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
