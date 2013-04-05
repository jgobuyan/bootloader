/**
  ******************************************************************************
  * @file    STM32F3xx_IAP/inc/ymodem.h 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    02-October-2012
  * @brief   This file provides all the headers of the ymodem functions.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __YMODEM_H_
#define __YMODEM_H_

/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include "common.h"
#include "flash_if.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define PACKET_SEQNO_INDEX      (1)
#define PACKET_SEQNO_COMP_INDEX (2)

#define PACKET_HEADER           (3)
#define PACKET_TRAILER          (2)
#define PACKET_OVERHEAD         (PACKET_HEADER + PACKET_TRAILER)
#define PACKET_SIZE             (128)
#define PACKET_1K_SIZE          (1024)

#define FILE_NAME_LENGTH        (256)
#define FILE_SIZE_LENGTH        (16)

#define SOH                     (0x01)  /* start of 128-byte data packet */
#define STX                     (0x02)  /* start of 1024-byte data packet */
#define EOT                     (0x04)  /* end of transmission */
#define ACK                     (0x06)  /* acknowledge */
#define NAK                     (0x15)  /* negative acknowledge */
#define CA                      (0x18)  /* two of these in succession aborts transfer */
#define CRC16                   (0x43)  /* 'C' == 0x43, request 16-bit CRC */

#define ABORT1                  (0x41)  /* 'A' == 0x41, abort by user */
#define ABORT2                  (0x61)  /* 'a' == 0x61, abort by user */

#define NAK_TIMEOUT             (0x100000)
#define MAX_ERRORS              (5)

/* The STM32F3xx devices embed a CRC peripheral that can be used to replace the 
   software routine for CRC checksum. In this application note, the CRC peripheral 
   is used by default. The software routine is provided and can be used by 
   commenting the define below. */
   
//#define USE_CRC_PERIPH

/* Exported functions ------------------------------------------------------- */
int32_t Ymodem_Receive (uint8_t *);
uint8_t Ymodem_Transmit (uint8_t *,const  uint8_t* , uint32_t );
uint16_t UpdateCRC16(uint16_t crcIn, uint8_t byte);
uint16_t Cal_CRC16(const uint8_t* data, uint32_t size);
uint8_t CalChecksum(const uint8_t* data, uint32_t size);
int32_t Ymodem_CheckResponse(uint8_t c);
void Ymodem_PrepareIntialPacket(uint8_t *data, const uint8_t* fileName, uint32_t *length);
void Ymodem_PreparePacket(uint8_t *SourceBuf, uint8_t *data, uint8_t pktNo, uint32_t sizeBlk);
void Ymodem_SendPacket(uint8_t *data, uint16_t length);

#endif  /* __YMODEM_H_ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
