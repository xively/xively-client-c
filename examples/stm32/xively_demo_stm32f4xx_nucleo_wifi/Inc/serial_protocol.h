/**
  *******************************************************************************
  * @file    Projects/Multi/Examples/IKS01A1/DataLog/Inc/serial_protocol.h
  * @author  CL
  * @version V3.0.0
  * @date    12-August-2016
  * @brief   header for serial_protocol.c.
  *******************************************************************************
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
  ********************************************************************************
  */

/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __SERIAL_PROTOCOL__
#define __SERIAL_PROTOCOL__

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported defines --------------------------------------------------------*/

#define TMsg_EOF                            0xF0
#define TMsg_BS                             0xF1
#define TMsg_BS_EOF                         0xF2


#ifdef USE_USB_OTG_HS
#define TMsg_MaxLen             512
#else
#define TMsg_MaxLen             256
#endif

/* Exported types ------------------------------------------------------------*/

/**
 * @brief  Serial message structure definition
 */
typedef struct
{
  uint32_t Len;
  uint8_t Data[TMsg_MaxLen];
} TMsg;

/* Exported macro ------------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */

int ByteStuffCopyByte(uint8_t *Dest, uint8_t Source);
int ReverseByteStuffCopyByte2(uint8_t Source0, uint8_t Source1, uint8_t *Dest);
int ByteStuffCopy(uint8_t *Dest, TMsg *Source);
int ReverseByteStuffCopyByte(uint8_t *Source, uint8_t *Dest);
int ReverseByteStuffCopy(TMsg *Dest, uint8_t *Source);
void CHK_ComputeAndAdd(TMsg *Msg);
int CHK_CheckAndRemove(TMsg *Msg);
uint32_t Deserialize(uint8_t *Source, uint32_t Len);
int32_t Deserialize_s32(uint8_t *Source, uint32_t Len);
void Serialize(uint8_t *Dest, uint32_t Source, uint32_t Len);
void Serialize_s32(uint8_t *Dest, int32_t Source, uint32_t Len);

#endif /* __SERIAL_PROTOCOL__ */

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
