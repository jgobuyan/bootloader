/**
  * Platform functions
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f37x.h"
#include "stm32373c_eval.h"
#include "stm32373c_eval_lcd.h"
#include "debug.h"
#include <string.h>

#define NUM_ROWS    10
typedef struct {
    uint8_t row;
    uint8_t col;
    uint16_t bgcolor;
    uint16_t fgcolor;
} LCD_Device;

LCD_Device devLCD;

void platform_init(void)
{
    /*------------------- Resources Initialization -----------------------------*/
    /* Enable GPIOA, GPIOB and GPIOC clocks */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC | \
                          RCC_AHBPeriph_GPIOE | RCC_AHBPeriph_GPIOF, ENABLE);

    /* Set I2C2 Clock Source to SYSCLK */
    /* Note : I2C_TIMING depends on the I2C2 Clock source */
    RCC_I2CCLKConfig(RCC_I2C2CLK_SYSCLK);

    devLCD.row = 0;
    devLCD.col = 0;
    devLCD.bgcolor = LCD_COLOR_RED;
    devLCD.fgcolor = LCD_COLOR_WHITE;
    STM32373C_LCD_Init();
    LCD_Clear(devLCD.bgcolor);
    LCD_SetBackColor(devLCD.bgcolor);
    LCD_SetTextColor(devLCD.fgcolor);
    LCD_DisplayOn();
    STM_EVAL_PBInit(BUTTON_KEY, BUTTON_MODE_GPIO);
}

/**
 * Get the state of the switch that is used to invoke the bootloader function
 * @return
 */
uint32_t platform_getSwitchState(void)
{
	return STM_EVAL_PBGetState(BUTTON_KEY);
}

/**
 * Initialize serial port
 */
void platform_initSerialPort(void *init)
{
    STM_EVAL_COMInit(COM1, (USART_InitTypeDef *)init);
}

/**
 * @brief  Print a string on the LCD
 * @param  s: The string to be printed
 * @retval None
 */
void DebugPutString(char *s)
{
    //LCD_ClearLine(LINE(devLCD.row));
    LCD_DisplayStringLine(LINE(devLCD.row), (uint8_t *)s);
    devLCD.row = (devLCD.row + 1) % NUM_ROWS;
    LCD_ClearLine(LINE(devLCD.row));
}

void DebugPutString1(char *s, uint32_t n)
{
    char buf[20];
    strncpy(buf, s, 20);
    AppendInt2HexString(buf, n, 20);
    DebugPutString(buf);
}

