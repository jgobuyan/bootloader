/**
 * @file crc32.c
 *
 *  Created on: 2013-04-11
 *      Author: jeromeg
 */
#include "port.h"

#define CRC_INIT_VALUE  0xffffffff  /**< Initial value for CRC calculations */

/* Nibble lookup table for 0x04C11DB7 polynomial */
static const ULONG CrcTable[16] =
{
        0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9, 0x130476DC, 0x17C56B6B,
                0x1A864DB2, 0x1E475005, 0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6,
                0x2B4BCB61, 0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD };

/**
 * Do one CRC calculation step over 32 bits.
 * @param ulCrc - Current CRC32 value
 * @param ulData - 32 bit integer to process
 * @return New CRC32 value
 */
ULONG crc32_step(ULONG ulCrc, ULONG ulData)
{

    ulCrc = ulCrc ^ ulData; // Apply all 32-bits

    // Process 32-bits, 4 at a time, or 8 rounds

    ulCrc = (ulCrc << 4) ^ CrcTable[ulCrc >> 28]; // Assumes 32-bit reg, masking index to 4-bits
    ulCrc = (ulCrc << 4) ^ CrcTable[ulCrc >> 28]; //  0x04C11DB7 Polynomial used in STM32
    ulCrc = (ulCrc << 4) ^ CrcTable[ulCrc >> 28];
    ulCrc = (ulCrc << 4) ^ CrcTable[ulCrc >> 28];
    ulCrc = (ulCrc << 4) ^ CrcTable[ulCrc >> 28];
    ulCrc = (ulCrc << 4) ^ CrcTable[ulCrc >> 28];
    ulCrc = (ulCrc << 4) ^ CrcTable[ulCrc >> 28];
    ulCrc = (ulCrc << 4) ^ CrcTable[ulCrc >> 28];

    return (ulCrc);
}

/**
 * Calculate CRC32 across a buffer.
 * @param buf - Pointer to buffer
 * @param size - Size of the valid data in the buffer.
 * @return CRC32 value
 */
ULONG crc32(const void *buf, ULONG size)
{
    ULONG i;
    ULONG ulCrc = CRC_INIT_VALUE;
    ULONG *pBuf = buf;
    for (i = 0; i < size; i += 4)
    {
        ulCrc = crc32_step(ulCrc, *(pBuf++));
    }
    return ulCrc;
}

