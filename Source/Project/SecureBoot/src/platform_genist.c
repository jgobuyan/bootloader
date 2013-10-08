/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*
 * Modified for Genist Bootloader.
 *
 * This file uses the STMicro Standard Peripheral Library headers.
 * Rather than using the unwieldy method of initializing GPIO pins individually
 * in the STMicro library and having to manually port the pin assignments
 * already done under ChibiOS, it was desirable to use the autogenerated
 * board.h file as-is and configure all the pins at once as is done under
 * ChibiOS. This file is a hacked version of the autogenerated board.c file,
 * where all references to ChibiOS header files have been removed. The
 * necessary type definitions have been copied from various ChibiOS header
 * files to make this code compile using STMicro headers.
 */

/**
 * @addtogroup SecureBoot
 * @{
 */
/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "stm32f37x.h"
#include "debug.h"
#include "board.h"
#include <string.h>

#define AHB_EN_MASK     (RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN |          \
                         RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIODEN |          \
                         RCC_AHBENR_GPIOEEN | RCC_AHBENR_GPIOFEN |          \
                         RCC_AHBENR_CRCEN)
/* On-board LED Mapping */
#define LED1_BASE (GPIO_TypeDef *)GPIOA_BASE
#define LED1_PIN  (1<<GPIOA_LED1_X)
#define LED2_BASE (GPIO_TypeDef *)GPIOA_BASE
#define LED2_PIN  (1<<GPIOA_LED2_X)
#define LED3_BASE (GPIO_TypeDef *)GPIOB_BASE
#define LED3_PIN  (1<<GPIOB_IN4)
#define HAZ2_BASE (GPIO_TypeDef *)GPIOB_BASE
#define HAZ2_PIN  (1<<GPIOB_HAZ_2)
/* RS485 Kludge */
#define RS485RE_BASE	(GPIO_TypeDef *)GPIOD_BASE
#define RS485RE_PIN (1<<GPIOD_ST_DIS4)
/**
 * @brief   GPIO port setup info.
 */
typedef struct {
  /** Initial value for MODER register.*/
  uint32_t              moder;
  /** Initial value for OTYPER register.*/
  uint32_t              otyper;
  /** Initial value for OSPEEDR register.*/
  uint32_t              ospeedr;
  /** Initial value for PUPDR register.*/
  uint32_t              pupdr;
  /** Initial value for ODR register.*/
  uint32_t              odr;
  /** Initial value for AFRL register.*/
  uint32_t              afrl;
  /** Initial value for AFRH register.*/
  uint32_t              afrh;
} stm32_gpio_setup_t;

/**
 * @brief   STM32 GPIO static initializer.
 * @details An instance of this structure must be passed to @p palInit() at
 *          system startup time in order to initialize the digital I/O
 *          subsystem. This represents only the initial setup, specific pads
 *          or whole ports can be reprogrammed at later time.
 */
typedef struct {
  /** @brief Port A setup data.*/
  stm32_gpio_setup_t    PAData;
  /** @brief Port B setup data.*/
  stm32_gpio_setup_t    PBData;
  /** @brief Port C setup data.*/
  stm32_gpio_setup_t    PCData;
  /** @brief Port D setup data.*/
  stm32_gpio_setup_t    PDData;
  /** @brief Port E setup data.*/
  stm32_gpio_setup_t    PEData;
  /** @brief Port F setup data.*/
  stm32_gpio_setup_t    PFData;
} PALConfig;

/**
 * @brief   PAL setup.
 * @details Digital I/O ports static configuration as defined in @p board.h.
 *          This variable is used by the HAL when initializing the PAL driver.
 */
const PALConfig pal_default_config =
{
  {VAL_GPIOA_MODER, VAL_GPIOA_OTYPER, VAL_GPIOA_OSPEEDR, VAL_GPIOA_PUPDR,
   VAL_GPIOA_ODR,   VAL_GPIOA_AFRL,   VAL_GPIOA_AFRH},
  {VAL_GPIOB_MODER, VAL_GPIOB_OTYPER, VAL_GPIOB_OSPEEDR, VAL_GPIOB_PUPDR,
   VAL_GPIOB_ODR,   VAL_GPIOB_AFRL,   VAL_GPIOB_AFRH},
  {VAL_GPIOC_MODER, VAL_GPIOC_OTYPER, VAL_GPIOC_OSPEEDR, VAL_GPIOC_PUPDR,
   VAL_GPIOC_ODR,   VAL_GPIOC_AFRL,   VAL_GPIOC_AFRH},
  {VAL_GPIOD_MODER, VAL_GPIOD_OTYPER, VAL_GPIOD_OSPEEDR, VAL_GPIOD_PUPDR,
   VAL_GPIOD_ODR,   VAL_GPIOD_AFRL,   VAL_GPIOD_AFRH},
  {VAL_GPIOE_MODER, VAL_GPIOE_OTYPER, VAL_GPIOE_OSPEEDR, VAL_GPIOE_PUPDR,
   VAL_GPIOE_ODR,   VAL_GPIOE_AFRL,   VAL_GPIOE_AFRH},
  {VAL_GPIOF_MODER, VAL_GPIOF_OTYPER, VAL_GPIOF_OSPEEDR, VAL_GPIOF_PUPDR,
   VAL_GPIOF_ODR,   VAL_GPIOF_AFRL,   VAL_GPIOF_AFRH}
};

static void initgpio(GPIO_TypeDef *gpiop, const stm32_gpio_setup_t *config) {

  gpiop->OTYPER  = config->otyper;
  gpiop->OSPEEDR = config->ospeedr;
  gpiop->PUPDR   = config->pupdr;
  gpiop->ODR     = config->odr;
  gpiop->AFR[0]  = config->afrl;
  gpiop->AFR[1]  = config->afrh;
  gpiop->MODER   = config->moder;
}

static TIM_TimeBaseInitTypeDef TIM2Config;
/**
 * Initialize board GPIOs
 */
void boardInit(void)
{
	RCC_AHBPeriphClockCmd(AHB_EN_MASK, ENABLE);
	initgpio((GPIO_TypeDef *)GPIOA_BASE, &pal_default_config.PAData);
	initgpio((GPIO_TypeDef *)GPIOB_BASE, &pal_default_config.PBData);
	initgpio((GPIO_TypeDef *)GPIOC_BASE, &pal_default_config.PCData);
	initgpio((GPIO_TypeDef *)GPIOD_BASE, &pal_default_config.PDData);
	initgpio((GPIO_TypeDef *)GPIOE_BASE, &pal_default_config.PEData);
	initgpio((GPIO_TypeDef *)GPIOF_BASE, &pal_default_config.PFData);
}

/**
 * Initialize platform.
 */
void platform_init(void)
{
    boardInit();
    /* Set up Timer 2 for LED blinking */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseStructInit(&TIM2Config);
    TIM2Config.TIM_Prescaler = 7199;	/* 100 KHz timebase */
    TIM2Config.TIM_Period = 1000;
    TIM2Config.TIM_CounterMode = TIM_CounterMode_Down;
    TIM_DeInit(TIM2);
    TIM_TimeBaseInit(TIM2, &TIM2Config);
    TIM_ARRPreloadConfig(TIM2, ENABLE);
    TIM_ITConfig(TIM2,TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
    NVIC_EnableIRQ(TIM2_IRQn);
}

/**
 * Deinitialize platform.
 */
void platform_deinit(void)
{
    NVIC_DisableIRQ(TIM2_IRQn);
    TIM_DeInit(TIM2);
    /* Revert to HSI as system clock source */
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
    RCC->CFGR |= (uint32_t)RCC_CFGR_SW_HSI;

    /* Wait till HSI is used as system clock source */
    while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)RCC_CFGR_SWS_HSI)
    {
    }
}

/**
 * Get the state of the signal that is used to invoke the bootloader function.
 * In the Genist board, the hazard 2 input is checked to see if it is held
 * for high for a long time (> 5s). This is not a typical state in normal
 * operation as it is usually either low or flashing high for a 1-2 seconds.
 *
 * @return 0 to invoke bootloader, 1 to run application
 */
uint32_t platform_getSwitchState(void)
{
	uint8_t pinState;
	uint32_t timeout = 20000000;
	/* Sample Hazard 2 input. If it is ever detected low, then exit. */
	GPIO_WriteBit(LED1_BASE, LED1_PIN, Bit_RESET);
	GPIO_WriteBit(LED2_BASE, LED2_PIN, Bit_RESET);
	GPIO_WriteBit(LED3_BASE, LED3_PIN, Bit_SET);


	while (timeout > 0)
	{
		pinState = GPIO_ReadInputDataBit(HAZ2_BASE, HAZ2_PIN);
		if (pinState == 0) // temperary
		{
			GPIO_WriteBit(LED1_BASE, LED1_PIN, Bit_RESET);
			GPIO_WriteBit(LED2_BASE, LED2_PIN, Bit_SET);
			GPIO_WriteBit(LED3_BASE, LED3_PIN, Bit_RESET);
 //           while (1)
 //           {
 //           }

			return 0;
		}
		timeout--;
	}
	return 1;
}

static uint8_t flashLed = 0;
static uint8_t flashLedStatus = 0;

/**
 * flash red LED on
 */
void platform_redLedFlashOn(void)
{
    flashLed |= 1;
}

/**
 * flash red LED off
 */
void platform_redLedFlashOff(void)
{
    flashLed &= ~1;
}

/**
 * Turn red LED on
 */
void platform_redLedOn(void)
{

    GPIO_WriteBit(LED1_BASE, LED1_PIN, Bit_SET);
    GPIO_WriteBit(LED2_BASE, LED2_PIN, Bit_RESET);
	GPIO_WriteBit(LED3_BASE, LED3_PIN, Bit_RESET);
}

/**
 * Turn red LED off
 */
void platform_redLedOff(void)
{
    GPIO_WriteBit(LED1_BASE, LED1_PIN, Bit_RESET);
	GPIO_WriteBit(LED2_BASE, LED2_PIN, Bit_RESET);
	GPIO_WriteBit(LED3_BASE, LED3_PIN, Bit_RESET);

}

/**
 * Turn blue LED on
 */
void platform_blueLedOn(void)
{
    GPIO_WriteBit(LED2_BASE, LED2_PIN, Bit_RESET);

}

/**
 * Turn blue LED off
 */
void platform_blueLedOff(void)
{
    GPIO_WriteBit(LED2_BASE, LED2_PIN, Bit_SET);
}

/**
 * Turn green LED on
 */
void platform_greenLedOn(void)
{
    GPIO_WriteBit(LED3_BASE, LED3_PIN, Bit_RESET);

}

/**
 * Turn green LED off
 */
void platform_greenLedOff(void)
{
    GPIO_WriteBit(LED3_BASE, LED3_PIN, Bit_SET);
}


/**
 * Initialize serial port
 */
void platform_initSerialPort(void *init)
{

	  /* Enable USART clock */
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	  /* USART configuration */
	  USART_Init(USART3, (USART_InitTypeDef *)init);

	  /* TODO: Configure RS485 functionality */
	  /* Enable USART */
	  USART_Cmd(USART3, ENABLE);
      USART_DEPolarityConfig(USART3, USART_DEPolarity_High);
	  USART_DECmd(USART3, ENABLE);

}

/**
 * @brief  Print a string on an output device
 * @param  s: The string to be printed
 * @retval None
 */
void DebugPutString(char *s)
{
	(void) s;
}

void DebugPutString1(char *s, uint32_t n)
{
	(void) s;
	(void) n;
}

/**
 * Kludge the RS485 RE signal for Genist V3 board to mirror the DE signal.
 * @param state
 */
void platform_rs485Kludge(uint8_t state)
{

//#ifndef BOARD_GENIST_STM32F373_V5
//	if (state)
//	{
//		GPIO_WriteBit(RS485RE_BASE, RS485RE_PIN, Bit_RESET);
//	}
//	else
//	{
//		GPIO_WriteBit(RS485RE_BASE, RS485RE_PIN, Bit_SET);
//	}
//#endif
}

/**
 * Timer 2 interrupt handler.
 */
void TIM2_IRQHandler(void)
{
    static int count = 0;
    if (flashLed & 1)
    {
        if (count == 0)
        {
            count = 3;
            if (flashLedStatus & 1)
            {
                platform_redLedOff();
                flashLedStatus &= ~1;
            }
            else
            {
                platform_redLedOn();
                flashLedStatus |= 1;
            }
        }
        else
        {
            count--;
        }
    }
    else
    {
        platform_redLedOff();
    }
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}
