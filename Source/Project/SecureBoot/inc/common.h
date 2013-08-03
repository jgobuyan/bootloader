/**
  ******************************************************************************
  * @file    SecureBoot/inc/common.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    02-October-2012
  * @brief   This file provides all the headers of the common functions.
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
#ifndef __COMMON_H
#define __COMMON_H

/* Includes ------------------------------------------------------------------*/
#ifdef STM32F37X
  #include "stm32f37x.h"
#ifdef USE_EVAL_BOARD
  #include "stm32373c_eval.h"
#endif
#endif  /* STM32F37X */

#ifdef STM32F30X
  #include "stm32f30x.h"
#ifdef USE_EVAL_BOARD
  #include "stm32303c_eval.h"
#endif
#endif  /* STM32F30X */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Constants used by Serial Command Line Mode */
#define CMD_STRING_SIZE       128   /**< Command String Size */

/* Exported macro ------------------------------------------------------------*/
/* Common routines */
#define IS_AF(c)  ((c >= 'A') && (c <= 'F'))
#define IS_af(c)  ((c >= 'a') && (c <= 'f'))
#define IS_09(c)  ((c >= '0') && (c <= '9'))
#define ISVALIDHEX(c)  IS_AF(c) || IS_af(c) || IS_09(c)
#define ISVALIDDEC(c)  IS_09(c)
#define CONVERTDEC(c)  (c - '0')
#define CONVERTHEX_alpha(c)  (IS_AF(c) ? (c - 'A'+10) : (c - 'a'+10))
#define CONVERTHEX(c)   (IS_09(c) ? (c - '0') : CONVERTHEX_alpha(c))

#define SerialPutString(x) Serial_PutString((uint8_t*)(x))

/* Exported functions ------------------------------------------------------- */
void Int2Str(uint8_t* str,int32_t intnum);
uint32_t Str2Int(uint8_t *inputstr,int32_t *intnum);
uint32_t GetIntegerInput(int32_t * num);
uint32_t SerialKeyPressed(uint8_t *key);
uint8_t GetKey(void);
void SerialPutChar(uint8_t c);
void Serial_PutString(uint8_t *s);
void GetInputString(uint8_t * buffP);

#endif  /* __COMMON_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
