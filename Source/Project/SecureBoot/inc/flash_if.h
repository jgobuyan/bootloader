/**
  ******************************************************************************
  * @file    SecureBoot/inc/flash_If.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    02-October-2012
  * @brief   This file provides all the headers of the flash_if functions.
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
#ifndef __FLASH_IF_H
#define __FLASH_IF_H

/* Includes ------------------------------------------------------------------*/
#include "port.h"
#include "flashmap.h"
#ifdef STM32F37X
  #include "stm32f37x.h"
#endif  /* STM32F37X */

#ifdef STM32F30X
  #include "stm32f30x.h"
#endif  /* STM32F30X */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
  #define USER_FLASH_LAST_PAGE_ADDRESS  0x0803F800
  #define USER_FLASH_END_ADDRESS        0x0803FFFF /* 256 KBytes */
  #define FLASH_PAGE_SIZE               0x800      /* 2 Kbytes */

/* define the address from where user application will be loaded,
   the application address should be a start sector address */
//#define APPLICATION_ADDRESS     (uint32_t)0x08003000

/* Get the number of pages from where the user program will be loaded */
#define  FLASH_PAGE_NUMBER      (uint32_t)((FLASH_BANKA_BASE - 0x08000000) >> 12)

/* Compute the mask to test if the Flash memory, where the user program will be
   loaded, is write protected */
#define  FLASH_PROTECTED_PAGES   ((uint32_t)~((1 << FLASH_PAGE_NUMBER) - 1))

/* define the user application size */
#define USER_FLASH_SIZE   (USER_FLASH_END_ADDRESS - FLASH_BANKA_BASE + 1)

/* Exported functions ------------------------------------------------------- */
void FLASH_If_Init(void);
uint32_t FLASH_If_Erase(uint32_t StartSector, uint32_t Size);
uint32_t FLASH_If_Write(__IO uint32_t* FlashAddress, uint32_t* Data, uint16_t DataLength);
uint32_t FLASH_If_DisableWriteProtection(void);
uint32_t FLASH_If_GetWriteProtectionStatus(void);

#endif  /* __FLASH_IF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/*******************(C)COPYRIGHT 2011 STMicroelectronics *****END OF FILE******/
