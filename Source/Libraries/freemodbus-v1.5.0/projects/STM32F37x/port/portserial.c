/*
 * FreeModbus Libary: STM32F37x Port
 * Copyright (C) 2013 Jerome Gobuyan <jerome.gobuyan@gmail.com>
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
 * File: $Id: portserial.c,v 1.1 2006/08/22 21:35:13 wolti Exp $
 */

#include "port.h"
#include "platform.h"
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- ST Peripheral includes ---------------------------*/
#include "stm32f37x.h"

#ifndef USE_EVAL_BOARD
#define USART                           USART3
#define USART_IRQ                       USART3_IRQn
#else
#define USART                           USART2
#define USART_IRQ                       USART2_IRQn
#endif
/* ----------------------- static functions ---------------------------------*/
extern void platform_rs485Kludge(uint8_t state);

USART_InitTypeDef USART_InitStruct;
/* ----------------------- Start implementation -----------------------------*/
void
vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
    /* If xRXEnable enable serial receive interrupts. If xTxEnable enable
     * transmitter empty interrupts.
     */
    if (xRxEnable)
    {
        /* RS485 Kludge */
        platform_rs485Kludge(1);
        USART_ITConfig(USART, USART_IT_RXNE, ENABLE);

    }
    else
    {
        USART_ITConfig(USART, USART_IT_RXNE, DISABLE);
    }
    if (xTxEnable)
    {
        /* RS485 Kludge */
        platform_rs485Kludge(0);
        USART_ITConfig(USART, USART_IT_TXE, ENABLE);
    }
    else
    {
        USART_ITConfig(USART, USART_IT_TXE, DISABLE);
    }
}

BOOL
xMBPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits,
        eMBParity eParity)
{
    ULONG timeout;
    USART_Cmd(USART, ENABLE);
    USART_StructInit(&USART_InitStruct);
    USART_InitStruct.USART_BaudRate = ulBaudRate;
    ucPORT = ucPORT; /* keeps compiler happy */
    switch (ucDataBits)
    {
    case 7:
        USART_InitStruct.USART_WordLength = USART_WordLength_8b;
        break;
    case 8:
        if (eParity)
        {
            USART_InitStruct.USART_WordLength = USART_WordLength_9b;
        }
        else
        {
            USART_InitStruct.USART_WordLength = USART_WordLength_8b;
        }
        break;
    case 9:
        USART_InitStruct.USART_WordLength = USART_WordLength_9b;
        break;
    default:
        return FALSE;
        break;
    }
    switch (eParity)
    {
    case MB_PAR_NONE:
        USART_InitStruct.USART_StopBits = USART_StopBits_2;
        USART_InitStruct.USART_Parity = USART_Parity_No;
        break;
    case MB_PAR_EVEN:
        USART_InitStruct.USART_StopBits = USART_StopBits_1;
        USART_InitStruct.USART_Parity = USART_Parity_Even;
        break;
    case MB_PAR_ODD:
        USART_InitStruct.USART_StopBits = USART_StopBits_1;
        USART_InitStruct.USART_Parity = USART_Parity_Odd;
        break;
    }
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART, &USART_InitStruct);
    /*
     * Set idle timeout value in bit clocks. For baud rates 19200 and above,
     * set the timeout to 1.75 ms. Otherwise set it to 3.5 x character time
     * (39 bit clocks).
     */
    if (ulBaudRate >= 19200)
    {
        timeout = (175 * ulBaudRate) / 100000UL + 1;
    }
    else
    {
        timeout = 39;
    }
    USART_SetReceiverTimeOut(USART, timeout);
    USART_ReceiverTimeOutCmd(USART, ENABLE);

#ifndef USE_EVAL_BOARD
    USART_DEPolarityConfig(USART, USART_DEPolarity_High);
    USART_DECmd(USART, ENABLE);
    USART_SetDEAssertionTime(USART, 10);
    USART_SetDEDeassertionTime(USART, 10);
    USART_LINCmd(USART, ENABLE);
#endif
    USART_Cmd(USART, ENABLE);

    NVIC_EnableIRQ(USART_IRQ);
    return TRUE;
}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{
    /* Put a byte in the UARTs transmit buffer. This function is called
     * by the protocol stack if pxMBFrameCBTransmitterEmpty( ) has been
     * called. */
    USART_SendData(USART, (USHORT) ucByte);
    return TRUE;
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
    /* Return the byte in the UARTs receive buffer. This function is called
     * by the protocol stack after pxMBFrameCBByteReceived( ) has been called.
     */
    *pucByte = (CHAR) (USART_ReceiveData(USART) & (uint16_t)0x00ff);
    //DEBUG_PUTSTRING1("RxB: ", *pucByte);
    return TRUE;
}



/* Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call
 * xMBPortSerialPutByte( ) to send the character.
 */

void prvvUARTTxReadyISR( void )
{
    pxMBFrameCBTransmitterEmpty(  );
}

/* Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
void prvvUARTRxISR( void )
{
    BOOL bErr = FALSE;

    if (USART_GetFlagStatus(USART, USART_FLAG_FE))
    {
        bErr = TRUE;
        USART_ClearFlag(USART, USART_FLAG_FE);
    }

    if (USART_GetFlagStatus(USART, USART_FLAG_PE))
    {
        bErr = TRUE;
        USART_ClearFlag(USART, USART_FLAG_PE);
    }
    if (USART_GetFlagStatus(USART, USART_FLAG_ORE))
    {
        bErr = TRUE;
        USART_ClearFlag(USART, USART_FLAG_ORE);
    }
    if (!bErr)
    {
        pxMBFrameCBByteReceived(  );
    }
    else
    {
        /* Error occurred. Read and discard byte */
        (void)USART_ReceiveData(USART);
    }
}

void USART_RxTimeoutInterruptCmd( FunctionalState newstate )
{
    USART_ClearFlag(USART, USART_FLAG_RTO);
    if (newstate == ENABLE)
    {
        USART_ClearITPendingBit(USART, USART_IT_RTO);
        USART_ITConfig(USART, USART_IT_RTO, ENABLE);
    }
    else
    {
        USART_ITConfig(USART, USART_IT_RTO, DISABLE);
    }
}
