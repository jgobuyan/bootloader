/**
 * Debug utilities
 */

/* Includes ------------------------------------------------------------------*/
#include "port.h"
#include "debug.h"

/**
 * Convert an unsigned number into a hex string and append.
 * @param str
 * @param intnum
 */

void AppendInt2HexString(uint8_t* str, uint32_t n, uint32_t size)
{
    uint8_t i;
    uint32_t len = 0;
    while ((*str != 0) && (len < size))
    {
        str++;
        len++;
    }
    for (i = 0; (i < 8) && (len < size); i++)
    {
        str[i] = ((n & 0xf0000000) >> 28);
        if (str[i] > 9)
        {
            str[i] += 'A' - 10;
        }
        else
        {
            str[i] += '0';
        }
        n = n << 4;
        len++;
    }
    if (len < size)
    {
        str[i] = 0;
    }
}
