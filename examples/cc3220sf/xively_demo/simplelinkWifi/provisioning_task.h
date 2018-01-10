//*****************************************************************************
// Copyright (C) 2016 Texas Instruments Incorporated
// 
// All rights reserved. Property of Texas Instruments Incorporated.
// Restricted rights to use, duplicate or disclose this code are
// granted through contract.
// The program may not be used without the written permission of
// Texas Instruments Incorporated or against the terms and conditions
// stipulated in the agreement under which this program has been supplied,
// and under no circumstances can it be used with non-TI connectivity device.
//
//*****************************************************************************

#ifndef __PROVISIONING_H__
#define __PROVISIONING_H__

/* TI-DRIVERS Header files */
#include <ti/drivers/net/wifi/simplelink.h>

/* POSIX Header files */
#include <semaphore.h>
#include <pthread.h>
#include <time.h>

#define OCP_REGISTER_INDEX      (0)
#define OCP_REGISTER_OFFSET     (10)    /* OCP register used to store device role when coming out of hibernate */
                                        /* if ocpRegOffset is set -> AP role, otherwise -> STATION role */


/*!
 *  \brief  Provisioning events
 */
typedef enum
{
    PrvnEvent_Triggered,
    PrvnEvent_Started,
    PrvnEvent_StartFailed,
    PrvnEvent_ConfirmationSuccess,
    PrvnEvent_ConfirmationFailed,
    PrvnEvent_Stopped,
    PrvnEvent_WaitForConn,
    PrvnEvent_Timeout,
    PrvnEvent_Error,
    PrvnEvent_Max,
}PrvnEvent;

/*!
 *  \brief  Provisioning states
 */
typedef enum
{
    PrvnState_Init,
    PrvnState_Idle,
    PrvnState_WaitForConfirmation,
    PrvnState_Completed,
    PrvnState_Error,
    PrvnState_Max

}PrvnState;

typedef struct Provisioning_ControlBlock_t
{
    sem_t   connectionAsyncEvent;
    sem_t   provisioningDoneSignal;
}Provisioning_CB;

/****************************************************************************
                      GLOBAL VARIABLES
****************************************************************************/
extern Provisioning_CB  Provisioning_ControlBlock;

//****************************************************************************
//                      FUNCTION PROTOTYPES
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
_i16 SignalProvisioningEvent(PrvnEvent event);

//*****************************************************************************
//
//! \brief This function gets the current provisioning state
//!
//! \param[in]  None
//!
//! \return provisioning state
//!
//****************************************************************************
PrvnState GetProvisioningState();

//*****************************************************************************
//
//! \brief The interrupt handler for the LED timer
//!
//! \param[in]  None
//!
//! \return None
//!
//****************************************************************************
void LedTimerIntHandler(sigval val);

//*****************************************************************************
//
//! \brief This function starts the led toggling timer
//!
//! \param[in]  None
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
_i32 StartLedEvtTimer(_u32 timeout);

//*****************************************************************************
//
//! \brief This function stops the led toggling timer
//!
//! \param[in]  None
//!
//! \return 0 on success, negative value otherwise
//!
//****************************************************************************
_i32 StopLedEvtTimer();

//*****************************************************************************
//
//! \brief This function stops provisioning process
//!
//! \param[in]  None
//!
//! \return SL_RET_CODE_PROVISIONING_IN_PROGRESS if provisioning was running, otherwise 0
//!
//****************************************************************************
_i32 provisioningStop();

//*****************************************************************************
//
//! \brief This is the main provisioning task
//!
//! \param[in]  None
//!
//! \return None
//!
//****************************************************************************
void * provisioningTask(void *pvParameters);

#endif
