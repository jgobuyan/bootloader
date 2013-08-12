/**
 * @file    SecureBoot/src/mbfuncboot.c
 * @author  Jerome Gobuyan
 * @version V1.0.0
 * @date    2013-07-27
 * @brief   ModBus Functions
  */
/** @addtogroup ModBus
  * @{
  */
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
#include "keyfile.h"
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbframe.h"
#include "mbproto.h"
#include "mbconfig.h"

/* ----------------------- CCAN includes ------------------------------------*/
#include "crc32.h"
#include "encryption.h"

/* ----------------------- Defines ------------------------------------------*/
#define FLASH_EMPTY     0xffffffff  /**< Flash word in erased state */

/* ----------------------- Static functions ---------------------------------*/

/* ----------------------- Start implementation -----------------------------*/
static UCHAR    ucCurrentBank;
static ULONG    ulCurrentSeqNum;

const flashPartition Bank[4] = {
        { (UCHAR *)FLASH_BOOT_HEADER, FLASH_BOOT_SIZE   },
        { (UCHAR *)FLASH_BANKA_BASE,  FLASH_BANKA_SIZE  },
        { (UCHAR *)FLASH_BANKB_BASE,  FLASH_BANKB_SIZE  },
        { (UCHAR *)FLASH_BANKF_BASE,  FLASH_BANKF_SIZE  }
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

            if (ucBank != BANK_BOOT)
            {
                pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = ucCheckImage(pFwHeader);
            }
            else
            {
                /* Bootloader does not have a full header */
                pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_OK;
            }
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
 * Prepare Flash partition.
 * The bank can be specified explicitly or the bootloader can choose the
 * least recently used bank.
 *
 * If the factory bank is selected, the lock status is checked.
 *
 * @param pucFrame - ModBus frame
 * @param usLen - ModBus frame length
 * @return status
 */
eMBException eMBFuncBootPrepareFlash(UCHAR * pucFrame, USHORT * usLen)
{
    UCHAR ucBank;
    eMBException eStatus = MB_EX_NONE;
    fwHeader *pHdrA = (fwHeader *)Bank[BANK_A].addr;
    fwHeader *pHdrB = (fwHeader *)Bank[BANK_B].addr;
    ucBank = pucFrame[MB_PDU_FUNC_BOOT_BANK_OFF];
    if ((*usLen == MB_FUNC_BOOT_PREPAREFLASH_SIZE) || (ucBank > BANK_B))
    {
        if (ucBank == BANK_F)
        {
            /* Program Factory Bank. Initial state is unlocked. */
            ucCurrentBank = ucBank;
            ulCurrentSeqNum = FACTORY_UNLOCKED;

        }
        else if ((ucBank == BANK_A) || (ucBank == BANK_B))
        {
            /* Single Bank Mode. Always use specified Bank */
            ucCurrentBank = ucBank;
            if (ucCurrentBank == BANK_A)
            {
                /* Check if the image in Bank B is valid. If it is not,
                 * start sequence at 0. Otherwise ensure that Bank A
                 * sequence number is higher than Bank B.
                 */
                if (ucCheckImage(pHdrB))
                {
                    DEBUG_PUTSTRING("Bank B Invalid");

                    /* Start from 0 */
                    ulCurrentSeqNum = 0;
                }
                else
                {
                    DEBUG_PUTSTRING("Bank B Valid");
                    /* Bank B is valid. Increment sequence number */
                    ulCurrentSeqNum = (pHdrB->seqNum + 1) & SEQNUM_MASK;
                }
            }
            else
            {
                /* Check if the image in Bank A is valid */
                if (ucCheckImage(pHdrA))
                {
                    DEBUG_PUTSTRING("Bank A Invalid");

                    /* Start from 0 */
                    ulCurrentSeqNum = 0;
                }
                else
                {
                    DEBUG_PUTSTRING("Bank A Valid");
                    /* Bank A is valid. Increment sequence number */
                    ulCurrentSeqNum = (pHdrA->seqNum + 1) & SEQNUM_MASK;
                }
            }
        }
        else
        {
            /* Check if the bank number is valid */
            if (ucCheckImage(pHdrA))
            {
                DEBUG_PUTSTRING("Bank A Invalid");
                ucCurrentBank = BANK_A;
                if (ucCheckImage(pHdrB))
                {
                    DEBUG_PUTSTRING("Bank B Invalid");
                    /* Both banks are empty. Use A and start from 0 */
                    ulCurrentSeqNum = 0;
                }
                else
                {
                    DEBUG_PUTSTRING("Bank B Valid");
                    /* Bank B is valid, Bank A is empty */
                    ulCurrentSeqNum = (pHdrB->seqNum + 1) & SEQNUM_MASK;
                }
            }
            else
            {
                DEBUG_PUTSTRING("Bank A Valid");
                if (ucCheckImage(pHdrB))
                {
                    /* Bank A is valid, Bank B is empty */
                    DEBUG_PUTSTRING("Bank B Invalid");
                    ucCurrentBank = BANK_B;
                    ulCurrentSeqNum = (pHdrA->seqNum + 1) & SEQNUM_MASK;
                }
                else
                {
                    DEBUG_PUTSTRING("Bank B Valid");
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
        }
        /*
         * Fill in common parameters
         */
        pHdrA = (fwHeader *)Bank[ucCurrentBank].addr;
        *usLen = MB_FUNC_BOOT_PREPAREFLASH_SIZE;
        pucFrame[MB_PDU_FUNC_BOOT_BANK_OFF] = ucCurrentBank;

        /*
         * Check if factory load and check if locked
         */
        if ((ucCurrentBank == BANK_F) && (pHdrA->seqNum == FACTORY_LOCKED))
        {
            /* Don't erase if Bank F selected and is locked */
            pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_LOCKED;
            ucCurrentBank = BANK_INVALID;
        }
        else
        {
            /* Erase if Bank A or B selected or if Bank F is select and not locked */
            FLASH_Unlock();
            FLASH_If_Erase((uint32_t)Bank[ucCurrentBank].addr, Bank[ucCurrentBank].size);
            FLASH_Lock();
            pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_OK;
        }
    }
    else
    {
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}

/**
 * Receive an encrypted block. Decrypt and write it into Flash. The current bank
 * is selected previously in eMBFuncBootPrepareFlash().
 * @param pucFrame - ModBus frame
 * @param usLen - ModBus frame length
 * @return status
 */
eMBException eMBFuncBootUploadBlock(UCHAR * pucFrame, USHORT * usLen)
{
    eMBException eStatus = MB_EX_NONE;
    UCHAR ucBlockNum;
    ULONG ulFlashAddress;
    ULONG status;
    if (*usLen == MB_FUNC_BOOT_UPLOADBLOCK_REQ_SIZE)
    {
        FLASH_Unlock();
        ucBlockNum = pucFrame[MB_PDU_FUNC_BOOT_BLOCKNUM_OFF];
        if ((ucCurrentBank != BANK_A) && (ucCurrentBank != BANK_B) && (ucCurrentBank != BANK_F))
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
            if (!(status = FLASH_If_Write(&ulFlashAddress,
                    (uint32_t *)&pucFrame[MB_PDU_FUNC_BOOT_BLOCKDATA_OFF],
                    UPLOAD_BLOCK_SIZE / 4)))
            {
                pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_OK;
            }
            else
            {
                if (status == 1)
                {
                    pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_ERROR;
                }
                else
                {
                    pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_LOCKED;
                }
            }
        }
        else
        {
            /* block number is out of range */
            pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_BADBLKNUM;
        }
        FLASH_Lock();
    }
    else
    {
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    *usLen = MB_FUNC_BOOT_DEFAULT_RESP_SIZE;
    return eStatus;
}

/**
 * Validate image in Flash. Write the sequence number into the first location.
 * @param pucFrame
 * @param usLen
 * @return
 */
eMBException eMBFuncBootValidateImage(UCHAR * pucFrame, USHORT * usLen)
{
    eMBException eStatus = MB_EX_NONE;
    fwHeader *pHdr = (fwHeader *)Bank[ucCurrentBank].addr;
    ULONG ulAddr = (ULONG)Bank[ucCurrentBank].addr;
    UCHAR ucImageStatus;
    if (*usLen == MB_FUNC_BOOT_VALIDATEIMAGE_SIZE)

    {
        if ((ucCurrentBank != BANK_A) && (ucCurrentBank != BANK_B) && (ucCurrentBank != BANK_F))
        {
            pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_INVALID;
        }
        else
        {
            ucImageStatus = ucCheckImage(pHdr);
            pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = ucImageStatus;
            if (ucImageStatus == BOOT_UNVALIDATED)
            {

                if (validate_signature(pHdr))
                {
                    DEBUG_PUTSTRING("Bad Signature");
                    pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_BADSIG;
                }
                else
                {
                    DEBUG_PUTSTRING("Image OK");
                    pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_OK;
                    /* Write sequence number to validate */
                    FLASH_Unlock();
                    FLASH_If_Write(&ulAddr, &ulCurrentSeqNum, 1);
                    FLASH_Lock();
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
 * Receive a key ring block and write it into Flash.
 * @param pucFrame
 * @param usLen
 * @return
 */
eMBException eMBFuncBootSetKeys(UCHAR * pucFrame, USHORT * usLen)
{
    eMBException eStatus = MB_EX_NONE;
    UCHAR ucBlockNum;
    ULONG ulFlashAddress;
    KeyRing *pKeyRing = (KeyRing *)FLASH_KEY_BASE;
    if (*usLen == MB_FUNC_BOOT_SETKEYS_REQ_SIZE)
    {
        ucBlockNum = pucFrame[MB_PDU_FUNC_BOOT_BLOCKNUM_OFF];
        if (pKeyRing->bf_key_lock != FLASH_EMPTY)
        {
            /* Keys are locked. Do not allow reprogramming. */
            pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_LOCKED;
        }
        else if ((ucBlockNum * UPLOAD_BLOCK_SIZE) < KEYARRAY_SIZE)
        {
            ulFlashAddress = (ULONG) FLASH_KEY_BASE
                    + ucBlockNum * UPLOAD_BLOCK_SIZE;
            if (ucBlockNum == 0)
            {
                /* Ensure that the key lock field is in erased state */
                memset(&pucFrame[MB_PDU_FUNC_BOOT_BLOCKDATA_OFF],0xff, 4);
                /* Erase the keys */
                FLASH_If_Erase(ulFlashAddress, KEYARRAY_SIZE);
            }

            /* Write keys to Flash */
            FLASH_Unlock();
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
            FLASH_Lock();
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

/**
 * Lock keys or factory bank in Flash.
 *
 * For keys, the magic number is written into the first location.
 *
 * For the factory bank, the FACTORY_LOCKED value is written into the
 * sequence number.
 * @param pucFrame
 * @param usLen
 * @return
 */
eMBException eMBFuncBootLock(UCHAR * pucFrame, USHORT * usLen)
{
    UCHAR ucBank;
    eMBException eStatus = MB_EX_NONE;
    ULONG ulAddr;
    ULONG *pAddr;
    ULONG i;
    ULONG empty = TRUE;
    ULONG ulValue;
    ucBank = pucFrame[MB_PDU_FUNC_BOOT_BANK_OFF];

    if (*usLen == MB_FUNC_BOOT_LOCKKEYS_SIZE)

    {
        if (ucBank == BANK_BOOT)
        {
            ulAddr = (ULONG)FLASH_KEY_BASE;
            pAddr = (ULONG *)FLASH_KEY_BASE;
            ulValue = FW_MAGIC;
            /* Requesting to lock keys
             * Check if Flash key ring is empty.
             */
            i = 0;
            while ((pAddr[i] == FLASH_EMPTY)
                    && (i < KEYARRAY_SIZE / sizeof(ULONG)))
            {
                i++;
                if (pAddr[i] != FLASH_EMPTY)
                {
                    empty = FALSE;
                }
            }
            if (empty)
            {
                /* Don't lock if key ring is empty */
                pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_BANKEMPTY;
            }
            else
            {
                pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_OK;
                /* Write magic number to lock */
                FLASH_If_Write(&ulAddr, &ulValue, 1);
            }
        }
        else if (ucBank == BANK_F)
        {
            /*
             * Requesting to lock factory bank
             */
            fwHeader *pHdr = (fwHeader *)Bank[BANK_F].addr;
            ulAddr = (ULONG)&pHdr->seqNum;
            pAddr = &pHdr->seqNum;
            ulValue = FACTORY_LOCKED;

            pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = ucCheckImage(pHdr);
            if (pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] == BOOT_OK)
            {
                /* Factory bank is valid. Check if it is already locked. */
                if (pHdr->seqNum != FACTORY_LOCKED)
                {
                    /* Write sequence number to lock */
                    FLASH_Unlock();
                    FLASH_If_Write(&ulAddr, &ulValue, 1);
                    FLASH_Lock();
                }
                else
                {
                    /* Already locked */
                    pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_LOCKED;
                }
            }
            /*
             * If bank is invalid, the return code will indicate the error.
             */
        }
        else
        {
            pucFrame[MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF] = BOOT_INVALID;
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
    if ((ucBank == BANK_BOOT) || (ucCheckImage((fwHeader *)Bank[ucBank].addr) == BOOT_OK))
    {
        ret = (fwHeader *) Bank[ucBank].addr;
    }

    return ret;
}

/**
 * Initialize ModBus for SecureBoot.
 *
 * Register bootloader functions and set variables to default values.
 */
void mbBootInit(void)
{
    eMBRegisterCB( MB_FUNC_BOOT_GETHEADER, eMBFuncBootGetHeader);
    eMBRegisterCB( MB_FUNC_BOOT_PREPAREFLASH, eMBFuncBootPrepareFlash);
    eMBRegisterCB( MB_FUNC_BOOT_UPLOADBLOCK, eMBFuncBootUploadBlock);
    eMBRegisterCB( MB_FUNC_BOOT_VALIDATEIMAGE, eMBFuncBootValidateImage);
    eMBRegisterCB( MB_FUNC_BOOT_SETKEYS, eMBFuncBootSetKeys);
    eMBRegisterCB( MB_FUNC_BOOT_LOCK, eMBFuncBootLock);
    ucCurrentBank = BANK_BOOT;
}

/**
  * @}
  */

