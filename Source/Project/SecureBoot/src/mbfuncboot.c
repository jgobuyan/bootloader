/* 
 * FreeModbus Libary: A portable Modbus implementation for Modbus ASCII/RTU.
 * Copyright (c) 2006 Christian Walter <wolti@sil.at>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * File: $Id: mbfunccoils.c,v 1.8 2007/02/18 23:47:16 wolti Exp $
 */

/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"
#include "bootloader.h"
#include "fwheader.h"
#include "flash_if.h"
#include "flashmap.h"
#include "platform.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbframe.h"
#include "mbproto.h"
#include "mbconfig.h"

/* ----------------------- CCAN includes ------------------------------------*/
#include "crc32.h"
#include "encryption.h"

/* ----------------------- Defines ------------------------------------------*/
#define FLASH_EMPTY     0xffffffff

/* ----------------------- Static functions ---------------------------------*/

/* ----------------------- Start implementation -----------------------------*/
static UCHAR    ucCurrentBank;
static ULONG    ulCurrentSeqNum;

const flashPartition Bank[4] = {
        { (UCHAR *)FLASH_BOOT_HEADER, FLASH_BOOT_SIZE   },
        { (UCHAR *)FLASH_BANKA_BASE,  FLASH_BANK_SIZE   },
        { (UCHAR *)FLASH_BANKB_BASE,  FLASH_BANK_SIZE   },
        { (UCHAR *)FLASH_BANKF_BASE,  FLASH_BANK_SIZE   }
};

/**
 * Check for valid image
 * @param pHdr
 * @return
 */
UCHAR ucCheckImage(fwHeader *pHdr)
{
    UCHAR ret = BOOT_OK;
    if (pHdr->info.magic != FW_MAGIC)
    {
        ret = BOOT_BANKEMPTY;
    }
    else
    {
        if (crc32(&pHdr->info, sizeof(fwInfo) - 4) != pHdr->info.hcrc)

        {
            /* Bad header CRC */
            ret = BOOT_BADHCRC;
        }
        else if ((crc32(&pHdr[1], pHdr->info.length) != pHdr->info.dcrc))
        {
            ret = BOOT_BADDCRC;
        }
        else if (pHdr->seqNum > SEQNUM_MASK)
        {
            ret = BOOT_UNVALIDATED;
        }
    }
    return ret;
}
/**
 * Get Header Function Handler
 * @param pucFrame
 * @param usLen
 * @return
 */
eMBException eMBFuncBootGetHeader(UCHAR * pucFrame, USHORT * usLen)
{
    UCHAR ucBank;
    eMBException eStatus = MB_EX_NONE;
    fwHeader *pFwHeader;
    if (*usLen == MB_FUNC_BOOT_GETHEADER_REQ_SIZE)
    {
        ucBank = pucFrame[MB_PDU_FUNC_BOOT_BANK_OFF];

        /* Check if the bank number is valid */
        if (ucBank <= BANK_F)
        {
            pFwHeader = (fwHeader *)Bank[ucBank].addr;

            /* Copy the header from Flash */
            memcpy(&pucFrame[MB_PDU_FUNC_BOOT_FWHEADER_OFF], &pFwHeader->info,
                    sizeof(fwInfo));
            *usLen = MB_FUNC_BOOT_GETHEADER_RESP_SIZE;
#if 1
            if (ucBank != BANK_BOOT)
            {
                pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = ucCheckImage(pFwHeader);
            }
            else
            {
                /* Bootloader does not have a full header */
                pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_OK;
            }
#else
            pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_OK;
            if (pFwHeader->info.magic != FW_MAGIC)
            {
                pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_BANKEMPTY;
            }
            else
            {
                /* Don't calculate CRCs for bootloader */
                if (ucBank != BANK_BOOT)
                {
                    if (crc32(&pFwHeader->info, sizeof(fwInfo) - 4)
                            != pFwHeader->info.hcrc)

                    {
                        pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] =
                                BOOT_BADHCRC;
                    }
                    else if ((crc32(&pFwHeader[1], pFwHeader->info.length)
                            != pFwHeader->info.dcrc))
                    {
                        pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] =
                                BOOT_BADDCRC;
                    }
                }
            }
#endif
        }
        else
        {
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    else
    {
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}

/**
 * Prepare Flash partition
 * @param pucFrame
 * @param usLen
 * @return
 */
eMBException eMBFuncBootPrepareFlash(UCHAR * pucFrame, USHORT * usLen)
{
    eMBException eStatus = MB_EX_NONE;
    fwHeader *pHdrA = (fwHeader *)Bank[BANK_A].addr;
    fwHeader *pHdrB = (fwHeader *)Bank[BANK_B].addr;

    if (*usLen == MB_FUNC_BOOT_PREPAREFLASH_SIZE)
    {
        /* Check if the bank number is valid */
        if (ucCheckImage(pHdrA))
        {
            DEBUG_PUTSTRING("Bank A Invalid\n");
            ucCurrentBank = BANK_A;
            if (ucCheckImage(pHdrB))
            {
                DEBUG_PUTSTRING("Bank B Invalid\n");
                /* Both banks are empty. Use A and start from 0 */
                ulCurrentSeqNum = 0;
            }
            else
            {
                DEBUG_PUTSTRING("Bank B Valid\n");
                /* Bank B is valid, Bank A is empty */
                ulCurrentSeqNum = (pHdrB->seqNum + 1) & SEQNUM_MASK;
            }
        }
        else
        {
            DEBUG_PUTSTRING("Bank A Valid\n");
            if (ucCheckImage(pHdrB))
            {
                /* Bank A is valid, Bank B is empty */
                DEBUG_PUTSTRING("Bank B Invalid\n");
                ucCurrentBank = BANK_B;
                ulCurrentSeqNum = (pHdrA->seqNum + 1) & SEQNUM_MASK;
            }
            else
            {
                DEBUG_PUTSTRING("Bank B Valid\n");
                /* Both banks are valid. Compare seqNums and use the older one */
                if (pHdrA->seqNum == ((pHdrB->seqNum + 1) & SEQNUM_MASK))
                {
                    ucCurrentBank = BANK_B;
                    ulCurrentSeqNum = (pHdrA->seqNum + 1) & SEQNUM_MASK;
                }
                else
                {
                    ucCurrentBank = BANK_A;
                    ulCurrentSeqNum = (pHdrB->seqNum + 1) & SEQNUM_MASK;
                }
            }
        }

        pHdrA = (fwHeader *)Bank[ucCurrentBank].addr;

        /*
         *
         */
        FLASH_If_Erase((uint32_t)Bank[ucCurrentBank].addr, Bank[ucCurrentBank].size);
        *usLen = MB_FUNC_BOOT_PREPAREFLASH_SIZE;
        pucFrame[MB_PDU_FUNC_BOOT_BANK_OFF] = ucCurrentBank;
        pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_OK;

    }
    else
    {
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}
eMBException eMBFuncBootUploadBlock(UCHAR * pucFrame, USHORT * usLen)
{
    eMBException eStatus = MB_EX_NONE;
    UCHAR ucBlockNum;
    ULONG ulFlashAddress;
    if (*usLen == MB_FUNC_BOOT_UPLOADBLOCK_REQ_SIZE)
    {
        ucBlockNum = pucFrame[MB_PDU_FUNC_BOOT_BLOCKNUM_OFF];
        if ((ucCurrentBank != BANK_A) && (ucCurrentBank != BANK_B))
        {
            pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_INVALID;
        }
        else if ((ucBlockNum * UPLOAD_BLOCK_SIZE) < Bank[ucCurrentBank].size)
        {
            /* Decrypt block */
            block_decrypt(&pucFrame[MB_PDU_FUNC_BOOT_BLOCKDATA_OFF],
                    UPLOAD_BLOCK_SIZE);

            if (ucBlockNum == 0)
            {
                /* Ensure that the sequence number field is in erased state */
                memset(&pucFrame[MB_PDU_FUNC_BOOT_BLOCKDATA_OFF],0xff, 4);
            }
            ulFlashAddress = (ULONG) Bank[ucCurrentBank].addr
                    + ucBlockNum * UPLOAD_BLOCK_SIZE;
            if (!FLASH_If_Write(&ulFlashAddress,
                    (uint32_t *)&pucFrame[MB_PDU_FUNC_BOOT_BLOCKDATA_OFF],
                    UPLOAD_BLOCK_SIZE / 4))
            {
                pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_OK;
            }
            else
            {
                pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_ERROR;
            }
        }
        else
        {
            /* block number is out of range */
            pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_BADBLKNUM;
        }
    }
    else
    {
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    *usLen = MB_FUNC_BOOT_DEFAULT_RESP_SIZE;
    return eStatus;
}
eMBException eMBFuncBootValidateImage(UCHAR * pucFrame, USHORT * usLen)
{
    eMBException eStatus = MB_EX_NONE;
    fwHeader *pHdr = (fwHeader *)Bank[ucCurrentBank].addr;
    ULONG ulAddr = (ULONG)Bank[ucCurrentBank].addr;
    UCHAR ucImageStatus;
    if (*usLen == MB_FUNC_BOOT_VALIDATEIMAGE_SIZE)

    {
        if ((ucCurrentBank != BANK_A) && (ucCurrentBank != BANK_B))
        {
            pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_INVALID;
        }
        else
        {
            ucImageStatus = ucCheckImage(pHdr);
            pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = ucImageStatus;
            if (ucImageStatus == BOOT_UNVALIDATED)
            {
#if 0
                if (validate_signature(pHdr))
                {
                    DEBUG_PUTSTRING("Bad Signature");
                    pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_BADSIG;
                }
                else
#endif
                {
                    DEBUG_PUTSTRING("Image OK");
                    pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_OK;
                    /* Write sequence number to validate */
                    FLASH_If_Write(&ulAddr, &ulCurrentSeqNum, 1);
                }
            }
        }
    }
    else
    {
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    *usLen = MB_FUNC_BOOT_DEFAULT_RESP_SIZE;
    return eStatus;
}

/**
 * Return pointer to image header if bank is valid.
 * @param ucBank
 * @return
 */
fwHeader *getImageHeader(UCHAR ucBank)
{
    fwHeader *ret = NULL;
    if ((ucBank != BANK_BOOT) || (ucCheckImage((fwHeader *)Bank[ucBank].addr) == BOOT_OK))
    {
        ret = (fwHeader *) Bank[ucBank].addr;
    }

    return ret;
}

void mbBootInit(void)
{
    eMBRegisterCB( MB_FUNC_BOOT_GETHEADER, eMBFuncBootGetHeader);
    eMBRegisterCB( MB_FUNC_BOOT_PREPAREFLASH, eMBFuncBootPrepareFlash);
    eMBRegisterCB( MB_FUNC_BOOT_UPLOADBLOCK, eMBFuncBootUploadBlock);
    eMBRegisterCB( MB_FUNC_BOOT_VALIDATEIMAGE, eMBFuncBootValidateImage);
    ucCurrentBank = BANK_BOOT;
}

