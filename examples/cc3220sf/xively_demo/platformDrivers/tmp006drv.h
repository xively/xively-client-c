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

#ifndef __TMP006DRV_H__
#define __TMP006DRV_H__

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
// TMP006 Device I2C address
//*****************************************************************************
#define TMP006_DEV_ADDR         0x41

//*****************************************************************************
// TMP006 Register offset address
//*****************************************************************************
#define TMP006_VOBJECT_REG_ADDR         0x0
#define TMP006_TAMBIENT_REG_ADDR        0x1
#define TMP006_CONFIG_REG_ADDR          0x2
#define TMP006_MANUFAC_ID_REG_ADDR      0xFE
#define TMP006_DEVICE_ID_REG_ADDR       0xFF

//*****************************************************************************
// TMP006 Device details
//*****************************************************************************
#define TMP006_MANUFAC_ID       0x5449
#define TMP006_DEVICE_ID        0x0067

//*****************************************************************************
//
// API Function prototypes
//
//*****************************************************************************


//****************************************************************************
//
//! \brief Initialize the temperature sensor
//!         1. Get the device manufacturer and version
//!         2. Add any initialization here
//!
//! \param i2cHandle[in] the handle to the openned i2c device
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int TMP006DrvOpen(I2C_Handle i2cHandle);

//****************************************************************************
//
//! \brief Get the temperature value
//!         1. Get the sensor voltage reg and ambient temp reg values
//!         2. Compute the temperature from the read values
//!
//! \param[in]  i2cHandle   the handle to the openned i2c device
//! \param[out]     pfCurrTemp  the pointer to the temperature value store
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int TMP006DrvGetTemp(I2C_Handle i2cHandle, float *pfCurrTemp);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif //  __TMP006DRV_H__
