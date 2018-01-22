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


#ifndef __BMA222_H__
#define __BMA222_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

/* TI-DRIVERS Header files */
#include <ti/drivers/I2C.h>

/* driverlib Header files */
#include <ti/devices/cc32xx/inc/hw_types.h>

//*****************************************************************************
// BMA222 Device I2C address
//*****************************************************************************
#define BMA222_DEV_ADDR          0x18

#define BMA222_CHID_ID_NUM       0x00

//*****************************************************************************
// BMA222 Acc Data Register related macros
//*****************************************************************************
#define BMA222_ACC_DATA_X_NEW   (0x2)
#define BMA222_ACC_DATA_X       (0x3)
#define BMA222_ACC_DATA_Y_NEW   (0x4)
#define BMA222_ACC_DATA_Y       (0x5)
#define BMA222_ACC_DATA_Z_NEW   (0x6)
#define BMA222_ACC_DATA_Z       (0x7)



//*****************************************************************************
// BMA222 Data Interpretation macros
//*****************************************************************************
#define RESOLUTION_8BIT         ((float)(1.999 / 127))  //+-2g
#define G_VAL                   ((float)9.7798)

//*****************************************************************************
//
// API Function prototypes
//
//*****************************************************************************

//****************************************************************************
//
//! \brief Initialize the BMA222 accelerometer device with defaults
//!      Reads the CHIP ID.
//!
//! \param[in]  i2cHandle       the handle to the openned i2c device
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int BMA222Open(I2C_Handle i2cHandle);

//****************************************************************************
//
//! \brief Reads a block of continuous data
//!      Returns the data values in the specified store
//!
//! \param[in]  i2cHandle   the handle to the openned i2c device
//! \param[in]  ucRegAddr   the start offset register address
//! \param[out]     pucBlkData  the pointer to the data value store
//! \param[in]  ucBlkDataSz     the size of data to be read
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int BMA222Read(I2C_Handle i2cHandle, signed char *pcAccX, signed char *pcAccY, signed char *pcAccZ);

//****************************************************************************
//
//! \brief Get the raw accelerometer data register readings
//!         1. Reads the data registers over I2C.
//!         2. Returns the accelerometer readings
//!
//! \param[in]  i2cHandle   is the handle to the openned i2c device
//! \param[out]     psAccX      pointer to the raw AccX store
//! \param[out]     psAccY      pointer to the raw AccY store
//! \param[out]     psAccZ      pointer to the raw AccZ store
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int BMA222ReadNew(I2C_Handle i2cHandle, signed char *pcAccX, signed char *pcAccY, signed char *pcAccZ);

//****************************************************************************
//
//! \brief Place the BMA222 accelerometer device to standby
//!      Sets the device to standby mode.
//!
//! \param None
//!
//! \return 0: Success
//
//****************************************************************************
int BMA222Close();

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif //  __BMA222_H__
