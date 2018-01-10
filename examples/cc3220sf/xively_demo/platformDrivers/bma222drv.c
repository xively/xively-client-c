/*
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//*****************************************************************************
//
//! \addtogroup out_of_box
//! @{
//
//*****************************************************************************

/* standard includes */
#include <math.h>

/* TI-DRIVERS Header files */
#include <uart_term.h>

/* Example/Board Header files */
#include "bma222drv.h"
#include "xively_example.h"


#define FAILURE                 -1
#define SUCCESS                 0

/****************************************************************************
                      LOCAL FUNCTION PROTOTYPES
****************************************************************************/

//****************************************************************************
//
//! \brief Returns the value in the specified register
//!		 Returns the value in the specified register
//!
//! \param[in] 	i2cHandle 	the handle to the openned i2c device
//! \param[in] 	ucRegAddr 	the offset register address
//! \param[out] 	pucRegValue 	the pointer to the register value store
//! 
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int
GetRegisterValue(I2C_Handle i2cHandle, unsigned char ucRegAddr, unsigned char *pucRegValue);

//****************************************************************************
//
//! \brief Reads a block of continuous data
//!		 Returns the data values in the specified store
//!
//! \param[in] 	i2cHandle 	the handle to the openned i2c device
//! \param[in] 	ucRegAddr 	the start offset register address
//! \param[out] 	pucBlkData 	the pointer to the data value store
//! \param[in] 	ucBlkDataSz 	the size of data to be read
//! 
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int
BlockRead(I2C_Handle i2cHandle, unsigned char ucRegAddr, unsigned char *pucBlkData, unsigned char ucBlkDataSz);


//*****************************************************************************
//                 Local Functions
//*****************************************************************************

//****************************************************************************
//
//! \brief Returns the value in the specified register
//!		 Returns the value in the specified register
//!
//! \param[in] 	i2cHandle 	the handle to the openned i2c device
//! \param[in] 	ucRegAddr 	the offset register address
//! \param[out] 	pucRegValue 	the pointer to the register value store
//! 
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int
GetRegisterValue(I2C_Handle i2cHandle, unsigned char ucRegAddr, unsigned char *pucRegValue)
{
	I2C_Transaction i2cTransaction;
	signed char status;

	/* Invoke the readfrom I2C API to get the required bytes */
    i2cTransaction.slaveAddress = BMA222_DEV_ADDR;
	i2cTransaction.writeBuf = &ucRegAddr;
	i2cTransaction.writeCount = 1;
	i2cTransaction.readBuf = pucRegValue;
	i2cTransaction.readCount = 1;

	status = I2C_transfer(i2cHandle, &i2cTransaction);

	if(status != true)
	{
		return FAILURE;
	}

	return SUCCESS;
}


//****************************************************************************
//
//! \brief Reads a block of continuous data
//!		 Returns the data values in the specified store
//!
//! \param[in] 	i2cHandle 	the handle to the openned i2c device
//! \param[in] 	ucRegAddr 	the start offset register address
//! \param[out] 	pucBlkData 	the pointer to the data value store
//! \param[in] 	ucBlkDataSz 	the size of data to be read
//! 
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int
BlockRead(I2C_Handle i2cHandle, unsigned char ucRegAddr, unsigned char *pucBlkData, unsigned char ucBlkDataSz)
{
	I2C_Transaction i2cTransaction;
	signed char status;

	/* Invoke the readfrom I2C API to get the required bytes */
	i2cTransaction.slaveAddress = BMA222_DEV_ADDR;
	i2cTransaction.writeBuf = &ucRegAddr;
	i2cTransaction.writeCount = 1;
	i2cTransaction.readBuf = pucBlkData;
	i2cTransaction.readCount = ucBlkDataSz;

	status = I2C_transfer(i2cHandle, &i2cTransaction);
	
	if(status != true)
	{
		return FAILURE;
	}

	return SUCCESS;
}

//****************************************************************************
//                            MAIN FUNCTION
//****************************************************************************


//****************************************************************************
//
//! \brief Sets the value in the specified register
//!		 Returns the value in the specified register
//!
//! \param[in] i2cHandle	the handle to the openned i2c device
//! \param[in] ucRegAddr 	the offset register address
//! \param[in] ucRegValue 	the register value to be set
//! 
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int
SetRegisterValue(I2C_Handle i2cHandle, unsigned char ucRegAddr, unsigned char ucRegValue)
{
	unsigned char ucData[2];
	I2C_Transaction i2cTransaction;
	signed char status;

	/* Select the register to be written followed by the value */
	ucData[0] = ucRegAddr;
	ucData[1] = ucRegValue;
	
	/* Invoke the readfrom I2C API to get the required bytes */
	i2cTransaction.slaveAddress = BMA222_DEV_ADDR;
	i2cTransaction.writeBuf = ucData;
	i2cTransaction.writeCount = 2;
	i2cTransaction.readBuf = NULL;
	i2cTransaction.readCount = 0;

	status = I2C_transfer(i2cHandle, &i2cTransaction);

	if(status != true)
	{
		return FAILURE;
	}

	return SUCCESS;
}


//****************************************************************************
//
//! \brief Initialize the BMA222 accelerometer device with defaults
//!		 Reads the CHIP ID.
//!
//! \param[in]	i2cHandle		the handle to the openned i2c device
//! 
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int
BMA222Open(I2C_Handle i2cHandle)
{
	unsigned char ucRegVal;
	signed char status;

	/* Read the CHIP ID NUM */
	status = GetRegisterValue(i2cHandle, BMA222_CHID_ID_NUM, &ucRegVal);
	if(status == 0)
	{
		INFO_PRINT("CHIP ID: 0x%x\n\r", ucRegVal);
		return SUCCESS;
	}

	return FAILURE;
}

//****************************************************************************
//
//! \brief Place the BMA222 accelerometer device to standby
//!    	 Sets the device to standby mode.
//!
//! \param None
//! 
//! \return 0: Success
//
//****************************************************************************
int
BMA222Close()
{
    return SUCCESS;
}

//****************************************************************************
//
//! \brief Get the accelerometer data readings
//!    		1. Reads the data registers over I2C.
//!    		2. Applies the range conversion to the raw values
//!
//! \param[in] 	i2cHandle 	is the handle to the openned i2c device
//! \param[out] 	psAccX 		pointer to the raw AccX store
//! \param[out] 	psAccY 		pointer to the raw AccY store
//! \param[out] 	psAccZ 		pointer to the raw AccZ store
//! 
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int
BMA222Read(I2C_Handle i2cHandle, signed char *pcAccX, signed char *pcAccY, signed char *pcAccZ)
{
	signed char status;
	char cAccX = 0;
	char cAccY = 0;
	char cAccZ = 0;

	/* Read the acclerometer output registers LSB and MSB */
	status = BlockRead(i2cHandle, BMA222_ACC_DATA_X, (unsigned char *)&cAccX, sizeof(cAccX));
	if(status != 0)
	{
		return FAILURE;
	}

	status = BlockRead(i2cHandle, BMA222_ACC_DATA_Y, (unsigned char *)&cAccY,  sizeof(cAccY));
	if(status != 0)
	{
		return FAILURE;
	}

	status = BlockRead(i2cHandle, BMA222_ACC_DATA_Z, (unsigned char *)&cAccZ,  sizeof(cAccZ));
	if(status != 0)
	{
		return FAILURE;
	}

	*pcAccX = cAccX;
	*pcAccY = cAccY;
	*pcAccZ = cAccZ;

	return SUCCESS;
}

//****************************************************************************
//
//! \brief Get the raw accelerometer data register readings
//!    		1. Reads the data registers over I2C.
//!    		2. Returns the accelerometer readings
//!
//! \param[in] 	i2cHandle 	is the handle to the openned i2c device
//! \param[out] 	psAccX 		pointer to the raw AccX store
//! \param[out] 	psAccY 		pointer to the raw AccY store
//! \param[out] 	psAccZ 		pointer to the raw AccZ store
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int
BMA222ReadNew(I2C_Handle i2cHandle, signed char *pcAccX, signed char *pcAccY, signed char *pcAccZ)
{
	signed char status;
	char cAccX[6];

	/* Read the acclerometer output registers LSB and MSB */
	status = BlockRead(i2cHandle, BMA222_ACC_DATA_X_NEW, (unsigned char *)cAccX,6);
	if(status != 0)
	{
		return FAILURE;
	}

	/* Check whether new Sensor Data is available */
	if((cAccX[0] & 0x1) && (cAccX[2] & 0x1) && (cAccX[4] & 0x1))
	{
	 *pcAccX = cAccX[1];
	 *pcAccY = cAccX[3];
	 *pcAccZ = cAccX[5];

	 return SUCCESS;
	}

	/*New Sensor Data Not Available */
	
	return FAILURE;

}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
