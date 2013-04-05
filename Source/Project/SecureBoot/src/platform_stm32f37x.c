/**
  * Platform functions
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f37x.h"
#include "stm32373c_eval_lcd.h"
#include "debug.h"
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
}


/**
 * @brief  Print a string on the LCD
 * @param  s: The string to be printed
 * @retval None
 */
void DebugPutString(uint8_t *s)
{
    LCD_ClearLine(LINE(devLCD.row));
    LCD_DisplayStringLine(LINE(devLCD.row), s);
    devLCD.row = (devLCD.row + 1) % NUM_ROWS;
}

void DebugPutString1(uint8_t *s, uint32_t n)
{
    uint8_t *p;
    uint8_t buf[20];
    p = strncpy(buf, s, 20);
    AppendInt2HexString(buf, n, 20);
    DebugPutString(buf);
}

