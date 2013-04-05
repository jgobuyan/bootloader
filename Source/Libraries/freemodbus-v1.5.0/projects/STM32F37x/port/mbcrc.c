/*
 * FreeModbus Library: ST Micro STM32F37x port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: mbcrc.c,v 1.2 2006/05/14 21:55:01 wolti Exp $
 */

/* ----------------------- System includes ----------------------------------*/


/* ----------------------- Platform includes --------------------------------*/
#include "port.h"
#include "stm32f37x_crc.h"
#include "platform.h" /* Debug */

#define CRC_POLYNOMIAL  0x8005
#define CRC_INIT_VALUE  0xffffffff

void CRC_Init( void )
{
    CRC_PolynomialSizeSelect(CRC_PolSize_16);
    CRC_SetPolynomial(CRC_POLYNOMIAL);
    CRC_ReverseInputDataSelect(CRC_ReverseInputData_8bits);
    CRC_ReverseOutputDataCmd(ENABLE);
    CRC_SetInitRegister(CRC_INIT_VALUE);
}

USHORT
usMBCRC16( UCHAR * pucFrame, USHORT usLen )
{
    USHORT i;
    uint16_t crc;
    CRC_Init();
	/* TODO: Use built-in CRC engine */
    CRC_ResetDR();
    for (i = 0; i < usLen; i++)
    {
        CRC_CalcCRC8bits(*(pucFrame++));
    }
    crc = CRC_GetCRC() & (uint16_t)0x0000ffff;
    //DEBUG_PUTSTRING1("CRC: ", crc);
    return crc;
}
