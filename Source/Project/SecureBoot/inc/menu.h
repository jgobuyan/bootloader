/**
  ******************************************************************************
  * @file    STM32F3xx_IAP/inc/menu.h 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    02-October-2012
  * @brief   This file provides all the headers of the menu functions.
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
#ifndef __MENU_H
#define __MENU_H

/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "flash_if.h"
#include "ymodem.h"

/* Private variables ---------------------------------------------------------*/
typedef  void (*pFunction)(void);

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Main_Menu(void);

#endif  /* __MENU_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

