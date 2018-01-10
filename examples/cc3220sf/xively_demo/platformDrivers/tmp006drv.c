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
#include "tmp006drv.h"
#include "xively_example.h"


#define FAILURE                 -1
#define SUCCESS                 0


/****************************************************************************
                      LOCAL FUNCTION PROTOTYPES
****************************************************************************/

//****************************************************************************
//
//! \brief Returns the value in the specified register
//!
//! \param[in] 	i2cHandle 	the handle to the openned i2c device
//! \param[in] 	ucRegAddr 	the offset register address
//! \param[out] 	pucRegValue 	the pointer to the register value store
//! 
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
static int GetRegisterValue(I2C_Handle i2cHandle, unsigned char ucRegAddr, unsigned short *pusRegValue);

//****************************************************************************
//
//! \brief Compute the temperature value from the sensor voltage and die temp.
//!
//! \param[in] dVobject 	the sensor voltage value
//! \param[in] dTAmbient 	the local die temperature
//! 
//! \return temperature value
//
//****************************************************************************
static double ComputeTemperature(double dVobject, double dTAmbient);


//*****************************************************************************
//                 Local Functions
//*****************************************************************************

//****************************************************************************
//
//! \brief Returns the value in the specified register
//!
//! \param[in] 	i2cHandle 	the handle to the openned i2c device
//! \param[in] 	ucRegAddr 	the offset register address
//! \param[out] 	pucRegValue 	the pointer to the register value store
//! 
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int
GetRegisterValue(I2C_Handle i2cHandle, unsigned char ucRegAddr, unsigned short *pusRegValue)
{
	I2C_Transaction i2cTransaction;
	unsigned char ucRegData[2];
	signed char status;

	/* Invoke the readfrom I2C API to get the required bytes */
    i2cTransaction.slaveAddress = TMP006_DEV_ADDR;
	i2cTransaction.writeBuf = &ucRegAddr;
	i2cTransaction.writeCount = 1;
	i2cTransaction.readBuf = ucRegData;
	i2cTransaction.readCount = 2;

	status = I2C_transfer(i2cHandle, &i2cTransaction);

	if(status != true)
	{
		return FAILURE;
	}

	*pusRegValue = (unsigned short)(ucRegData[0] << 8) | ucRegData[1];

	return SUCCESS;
}


//****************************************************************************
//
//! \brief Compute the temperature value from the sensor voltage and die temp.
//!
//! \param[in] dVobject 	the sensor voltage value
//! \param[in] dTAmbient 	the local die temperature
//! 
//! \return temperature value
//
//****************************************************************************
double ComputeTemperature(double dVobject, double dTAmbient)
{
    /*
    * This algo is obtained from 
    * http://processors.wiki.ti.com/index.php/SensorTag_User_Guide
    * #IR_Temperature_Sensor
    */
    double Tdie2 = dTAmbient + 273.15;
    const double S0 = 6.4E-14;            /* Calibration factor */
    const double a1 = 1.75E-3;
    const double a2 = -1.678E-5;
    const double b0 = -2.94E-5;
    const double b1 = -5.7E-7;
    const double b2 = 4.63E-9;
    const double c2 = 13.4;
    const double Tref = 298.15;
    double S = S0*(1+a1*(Tdie2 - Tref)+a2*pow((Tdie2 - Tref),2));
    double Vos = b0 + b1*(Tdie2 - Tref) + b2*pow((Tdie2 - Tref),2);
    double fObj = (dVobject - Vos) + c2*pow((dVobject - Vos),2);
    double tObj = pow(pow(Tdie2,4) + (fObj/S),.25);
    tObj = (tObj - 273.15);
    return tObj;
}

//****************************************************************************
//                            MAIN FUNCTION
//****************************************************************************


//****************************************************************************
//
//! \brief Initialize the temperature sensor
//!    		1. Get the device manufacturer and version
//!    		2. Add any initialization here
//!
//! \param i2cHandle[in] the handle to the openned i2c device
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int 
TMP006DrvOpen(I2C_Handle i2cHandle)
{
    unsigned short usManufacID, usDevID, usConfigReg;
	int status;

    /* Get the manufacturer ID */
    status = GetRegisterValue(i2cHandle, TMP006_MANUFAC_ID_REG_ADDR, &usManufacID);
	if(status != 0)
	{
		return FAILURE;
	}
	
    INFO_PRINT("Manufacturer ID: 0x%x\n\r", usManufacID);
    if(usManufacID != TMP006_MANUFAC_ID)
    {
        UART_PRINT("Error in Manufacturer ID\n\r");
        return FAILURE;
    }

    /* Get the device ID */
    status = GetRegisterValue(i2cHandle, TMP006_DEVICE_ID_REG_ADDR, &usDevID);
	if(status != 0)
	{
		return FAILURE;
	}
	
    INFO_PRINT("Device ID: 0x%x\n\r", usDevID);
    if(usDevID != TMP006_DEVICE_ID)
    {
        UART_PRINT("Error in Device ID\n");
        return FAILURE;
    }

    /* Get the configuration register value */
    status = GetRegisterValue(i2cHandle, TMP006_CONFIG_REG_ADDR, &usConfigReg);
	if(status != 0)
	{
		return FAILURE;
	}
	
    INFO_PRINT("Configuration register value: 0x%x\n\r", usConfigReg);

    return SUCCESS;
}


//****************************************************************************
//
//! \brief Get the temperature value
//!    		1. Get the sensor voltage reg and ambient temp reg values
//!    		2. Compute the temperature from the read values
//!
//! \param[in] 	i2cHandle 	the handle to the openned i2c device
//! \param[out] 	pfCurrTemp 	the pointer to the temperature value store
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int 
TMP006DrvGetTemp(I2C_Handle i2cHandle, float *pfCurrTemp)
{
    unsigned short usVObjectRaw, usTAmbientRaw;
    double dVObject, dTAmbient;
	int status;
	
    /* Get the sensor voltage register value */
    status = GetRegisterValue(i2cHandle, TMP006_VOBJECT_REG_ADDR, &usVObjectRaw);
	if(status != 0)
	{
		return FAILURE;
	}
	
    /* Get the ambient temperature register value */
    status = GetRegisterValue(i2cHandle, TMP006_TAMBIENT_REG_ADDR, &usTAmbientRaw);
	if(status != 0)
	{
		return FAILURE;
	}
	
    /* Apply the format conversion */
    dVObject = ((short)usVObjectRaw) * 156.25e-9;
    dTAmbient = ((short)usTAmbientRaw) / 128;

    *pfCurrTemp = ComputeTemperature(dVObject, dTAmbient);
    
    /* Convert to Farenheit */
    *pfCurrTemp = ((*pfCurrTemp * 9) / 5) + 32;

    return SUCCESS;
}
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
