/**
 * @file    SecureBoot/src/crc32.c
 * @author  Jerome Gobuyan
 * @version V1.0.0
 * @date    2013-07-27
 * @brief CRC32 functions implemented in STM32F37x hardware
 */

/**
 * @addtogroup SecureBoot
 * @{
 */

#include "crc32.h"
#include "port.h"
#include "stm32f37x_crc.h"
#include "platform.h" /* Debug */

#define CRC32_POLYNOMIAL    0x04c11db7  /**< Ethernet CRC-32 Polynomial */
#define CRC_INIT_VALUE      0xffffffff  /**< CRC-32 Initial value       */

/**
 * Calculate or check CRC across the buffer. The CRC32 field is assumed to be
 * at the end of the buffer. To generate, do not include CRC32 field in the
 * length and put the calculated value into it. To check, include the CRC32 field
 * in the length and check if the resulting CRC is zero.
 * @param buf - pointer to buffer
 * @param size - size of data in buffer
 * @return CRC
 */
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

/**
 * @}
 */
