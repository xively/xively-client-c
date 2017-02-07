/**
  ******************************************************************************
  * @file    Projects/Multi/Examples/IKS01A1/DataLog/Src/serial_protocol.c
  * @author  CL
  * @version V3.0.0
  * @date    12-August-2016
  * @brief   This file implements some utilities for the serial protocol
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */


/* Standard include ----------------------------------------------------------*/
#include "serial_protocol.h"

/** @addtogroup X_NUCLEO_IKS01A1_Examples
  * @{
  */

/** @addtogroup DATALOG
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/**
 * @brief  Byte stuffing process for one byte
 * @param  Dest destination
 * @param  Source source
 * @retval Total number of bytes processed
*/
int ByteStuffCopyByte(uint8_t *Dest, uint8_t Source)
{
  switch(Source)
  {
    case TMsg_EOF:
      Dest[0] = TMsg_BS;
      Dest[1] = TMsg_BS_EOF;
      return 2;
    case TMsg_BS:
      Dest[0] = TMsg_BS;
      Dest[1] = TMsg_BS;
      return 2;
    default:
      Dest[0] = Source;
      return 1;
  }
}

/**
 * @brief  Byte stuffing process for a Msg
 * @param  Dest destination
 * @param  Source source
 * @retval Total number of bytes processed
 */
int ByteStuffCopy(uint8_t *Dest, TMsg *Source)
{
  int i, Count;

  Count = 0;
  for (i = 0; i < Source->Len; i++)
  {
    Count += ByteStuffCopyByte(&Dest[Count], Source->Data[i]);
  }
  Dest[Count] = TMsg_EOF;
  Count++;
  return Count;
}

/**
 * @brief  Reverse Byte stuffing process for one byte
 * @param  Source source
 * @param  Dest destination
 * @retval Number of input bytes processed (1 or 2) or 0 for invalid sequence
 */
int ReverseByteStuffCopyByte(uint8_t *Source, uint8_t *Dest)
{
  if (Source[0] == TMsg_BS)
  {
    if (Source[1] == TMsg_BS)
    {
      *Dest = TMsg_BS;
      return 2;
    }
    if (Source[1] == TMsg_BS_EOF)
    {
      *Dest = TMsg_EOF;
      return 2;
    }
    return 0; // invalide sequence
  }
  else
  {
    *Dest = Source[0];
    return 1;
  }
}

/**
 * @brief  Reverse Byte stuffing process for two input data
 * @param  Source0 input data
 * @param  Source1 input data
 * @param  Dest the destination data
 * @retval Number of input bytes processed (1 or 2) or 0 for invalid sequence
 */
int ReverseByteStuffCopyByte2(uint8_t Source0, uint8_t Source1, uint8_t *Dest)
{
  if (Source0 == TMsg_BS)
  {
    if (Source1 == TMsg_BS)
    {
      *Dest = TMsg_BS;
      return 2;
    }
    if (Source1 == TMsg_BS_EOF)
    {
      *Dest = TMsg_EOF;
      return 2;
    }
    return 0; // invalid sequence
  }
  else
  {
    *Dest = Source0;
    return 1;
  }
}

/**
 * @brief  Reverse Byte stuffing process for a Msg
 * @param  Dest destination
 * @param  Source source
 * @retval 1 if the operation succeeds, 0 if an error occurs
 */
int ReverseByteStuffCopy(TMsg *Dest, uint8_t *Source)
{
  int Count = 0, State = 0;

  while ((*Source) != TMsg_EOF)
  {
    if (State == 0)
    {
      if ((*Source) == TMsg_BS)
      {
        State = 1;
      }
      else
      {
        Dest->Data[Count] = *Source;
        Count++;
      }
    }
    else
    {
      if ((*Source) == TMsg_BS)
      {
        Dest->Data[Count] = TMsg_BS;
        Count++;
      }
      else
      {
        if ((*Source) == TMsg_BS_EOF)
        {
          Dest->Data[Count] = TMsg_EOF;
          Count++;
        }
        else
        {
          return 0; // invalid sequence
        }
      }
      State = 0;
    }
    Source++;
  }
  if (State != 0) return 0;
  Dest->Len = Count;
  return 1;
}

/**
 * @brief  Compute and add checksum
 * @param  Msg pointer to the message
 * @retval None
 */
void CHK_ComputeAndAdd(TMsg *Msg)
{
  uint8_t CHK = 0;
  int i;

  for(i = 0; i < Msg->Len; i++)
  {
    CHK -= Msg->Data[i];
  }
  Msg->Data[i] = CHK;
  Msg->Len++;
}

/**
 * @brief  Compute and remove checksum
 * @param  Msg pointer to the message
 * @retval A number different from 0 if the operation succeeds, 0 if an error occurs
 */
int CHK_CheckAndRemove(TMsg *Msg)
{
  uint8_t CHK = 0;
  int i;

  for(i = 0; i < Msg->Len; i++)
  {
    CHK += Msg->Data[i];
  }
  Msg->Len--;
  return (CHK == 0);
}

/**
 * @brief  Build an array from the uint32_t (LSB first)
 * @param  Dest destination
 * @param  Source source
 * @param  Len number of bytes
 * @retval None
 */
void Serialize(uint8_t *Dest, uint32_t Source, uint32_t Len)
{
  int i;
  for (i = 0; i < Len; i++)
  {
    Dest[i] = Source & 0xFF;
    Source >>= 8;
  }
}

/**
 * @brief  Unbuild a Number from an array (LSB first)
 * @param  Source source
 * @param  Len number of bytes
 * @retval Rebuild unsigned int variable
 */
uint32_t Deserialize(uint8_t *Source, uint32_t Len)
{
  uint32_t app;
  app = Source[--Len];
  while(Len > 0)
  {
    app <<= 8;
    app += Source[--Len];
  }
  return app;
}

/**
 * @brief  Build an array from the uint32_t (LSB first)
 * @param  Dest destination
 * @param  Source source
 * @param  Len number of bytes
 * @retval None
 */
void Serialize_s32(uint8_t *Dest, int32_t Source, uint32_t Len)
{
  int i;
  for (i = 0; i < Len; i++)
  {
    Dest[i] = Source & 0xFF;
    Source >>= 8;
  }
}

/**
 * @brief  Unbuild a Number from an array (LSB first)
 * @param  Source source
 * @param  Len number of bytes
 * @retval Rebuild signed int variable
 */
int32_t Deserialize_s32(uint8_t *Source, uint32_t Len)
{
  int32_t app;
  app = Source[--Len];
  while(Len > 0)
  {
    app <<= 8;
    app += Source[--Len];
  }
  return app;
}

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
