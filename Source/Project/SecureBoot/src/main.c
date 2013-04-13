/**
 ******************************************************************************
 * @file    STM32F3xx_IAP/src/main.c
 * @author  MCD Application Team
 * @version V1.0.0
 * @date    02-October-2012
 * @brief   Main program body
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

/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "flash_if.h"
#include "mb.h"
#include "platform.h"
#include "bootloader.h"
#include "flashmap.h"
/** @addtogroup STM32F3xx_IAP
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
typedef  void (*pFunction)(void);

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
pFunction Jump_To_Application;
uint32_t JumpAddress;

/* Private function prototypes -----------------------------------------------*/
static void SecureBoot_Init(void);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Main program.
 * @param  None
 * @retval None
 */
int main(void)
{
    /*!< At this stage the microcontroller clock setting is already configured,
     this is done through SystemInit() function which is called from startup
     file (startup_stm32f37x.s/startup_stm32f30x.s) before to branch to
     application main. To reconfigure the default setting of SystemInit()
     function, refer to system_stm32f37x.c/system_stm32f30x.c files.
     */
    /* Initialize the platform */
    platform_init();
    DEBUG_PUTSTRING("HELLO!");
    /* Initialize Key Button mounted on STM320518-EVAL board */
    STM_EVAL_PBInit(BUTTON_KEY, BUTTON_MODE_GPIO);

    /* Test if Key push-button on STM320518-EVAL Board is pressed */
    if (STM_EVAL_PBGetState(BUTTON_KEY) == 0x00)
    {
        /* If Key is pressed, execute the IAP driver in order to re-program the Flash */
        SecureBoot_Init();
        DEBUG_PUTSTRING("Bootloader Mode");
        DEBUG_PUTSTRING("Start");

        /* Launch Modbus */
        ( void ) eMBInit(MB_RTU, 0x01, 0, 115200, MB_PAR_EVEN);
        mbBootInit();

        ( void ) eMBEnable(  );

    }

    /* Keep the user application running */
    else
    {
        /* Test if user code is programmed starting from address "APPLICATION_ADDRESS" */
        if (((*(__IO uint32_t*) FLASH_BANKA_BASE ) & 0x2FFE0000)
                == 0x20000000)
        {
            /* Jump to user application */
            JumpAddress = *(__IO uint32_t*) (FLASH_BANKA_BASE + 4);
            Jump_To_Application = (pFunction) JumpAddress;

            /* Initialize user application's Stack Pointer */
            __set_MSP(*(__IO uint32_t*) FLASH_BANKA_BASE);

            /* Jump to application */
            Jump_To_Application();
        }
    }

    __enable_irq();
    /* Infinite loop */
    while (1)
    {
        ( void )eMBPoll(  );
    }
}

/**
 * @brief  Initialize the IAP: Configure RCC, USART, CRC, Flash and GPIOs.
 * @param  None
 * @retval None
 */
void SecureBoot_Init(void)
{
    USART_InitTypeDef USART_InitStructure;

    /* Unlock the Flash Program Erase controller */
    FLASH_If_Init();

    /* USART resources configuration (Clock, GPIO pins and USART registers) ----*/
    /* USART configured as follow:
     - BaudRate = 115200 baud
     - Word Length = 8 Bits
     - One Stop Bit
     - No parity
     - Hardware flow control disabled (RTS and CTS signals)
     - Receive and transmit enabled
     */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl =
            USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    STM_EVAL_COMInit(COM1, &USART_InitStructure);

    /* Enable CRC clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);

#ifdef USE_CRC_PERIPH
    /* DeInit CRC peripheral */
    CRC_DeInit();
    /* Init the INIT register */
    CRC_SetInitRegister(0);
    /* Select 16-bit pol size */
    CRC_PolynomialSizeSelect(CRC_PolSize_16);
    /* Set the polynomial coefficients: x16 + x12 + x5 + 1 used in ymodem file transfer protocol */
    CRC_SetPolynomial(0x1021);
#endif /* USE_CRC_PERIPH */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}
#endif

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
