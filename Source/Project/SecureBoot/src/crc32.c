/*
 * crc32 functions implemented in STM32F37x hardware
 */

#include "crc32.h"
#include "port.h"
#include "stm32f37x_crc.h"
#include "platform.h" /* Debug */

#define CRC32_POLYNOMIAL    0x04c11db7
//#define CRC32_POLYNOMIAL    0xedb88320
#define CRC_INIT_VALUE      0xffffffff
#define CRC_INIT_VALUE_IEEE 0x00000000

uint32_t crc32(const void *buf, ULONG size)
{
    USHORT i;
    ULONG crc;
    const UCHAR *pBuf = buf;
    /* Initialize CRC engine */
    CRC_DeInit();
    CRC_PolynomialSizeSelect(CRC_PolSize_32);
    CRC_SetPolynomial(CRC32_POLYNOMIAL);
#if 1
    CRC_ReverseInputDataSelect(CRC_ReverseInputData_No);
    CRC_ReverseOutputDataCmd(DISABLE);
#else
    CRC_ReverseInputDataSelect(CRC_ReverseInputData_32bits);
    CRC_ReverseOutputDataCmd(ENABLE);
#endif
    CRC_SetInitRegister(CRC_INIT_VALUE);
    CRC_ResetDR();
#if 1
    /* Assume everything is word aligned */
    /* Do block calculation if possible */
    if (size & ~0x03)
    {
        crc = CRC_CalcBlockCRC((uint32_t *)pBuf, size / 4);
        pBuf += size & ~0x03;
    }

    /* Do remainder as single bytes */
    if (size & 0x03)
    {
        for (i = 0; i < (size & 0x03); i++)
        {
            CRC_CalcCRC8bits(*(pBuf++));
        }
    }
#else
    while (size)
    {
        crc = CRC_CalcCRC8bits(*(pBuf++));
        size--;
    }
#endif
    crc = CRC_GetCRC();
    DEBUG_PUTSTRING1("CRC32: ", crc);
    return crc;
}
