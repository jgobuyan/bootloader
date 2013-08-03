/**
 * @file cmd_keys.c
 *
 * Encryption/signature key management
 *
 *  Created on: 2013-07-18
 *      Author: jeromeg
 */

/**
 *
 * @addtogroup BootloaderCommand
 * @{
 */
#include <stdio.h>
#include <string.h>
#include "bootloader.h"
#include "fwheader.h"
#include "commands.h"
#include "platform.h"
/**
 * Send Set Keys request.
 * Binary key file is sent in two 1024 byte blocks.
 *
 * @param ucMBaddr
 * @param ucBlock
 * @param pucData
 * @param usLen
 * @return
 */
UCHAR cmd_setkeys(UCHAR ucMBaddr, UCHAR ucBlock, UCHAR *pucData, USHORT usLen)
{
    UCHAR    buf[UPLOAD_BLOCK_SIZE + 6];
    cmdFrameHeader *pFrame = (cmdFrameHeader *)buf;
    DEBUG_PUTSTRING1("SET_KEYS ", ucBlock);
    /* Clear memory first */
    memset(buf, 0, UPLOAD_BLOCK_SIZE);
    pFrame->mbAddr = ucMBaddr;
    pFrame->cmdId = MB_FUNC_BOOT_SETKEYS;
    pFrame->subcmdId = ucBlock;
    pFrame->status = 0;
    memcpy(pFrame + 1, pucData, usLen);
    cmd_start();
    eMBSendFrame(buf, MB_FUNC_BOOT_SETKEYS_REQ_SIZE + 1);
    return cmd_status_wait();
}

/**
 * Send Lock Keys request
 *
 * @param ucMBaddr
 * @return
 */
UCHAR cmd_lockkeys(UCHAR ucMBaddr)
{
    UCHAR    buf[8];
    cmdFrameHeader *pFrame = (cmdFrameHeader *)buf;
    DEBUG_PUTSTRING("LOCK_KEYS");
    pFrame->mbAddr = ucMBaddr;
    pFrame->cmdId = MB_FUNC_BOOT_LOCKKEYS;
    pFrame->subcmdId = 0;
    pFrame->status = 0;
    cmd_start();
    eMBSendFrame(buf, MB_FUNC_BOOT_VALIDATEIMAGE_SIZE + 1);
    return cmd_status_wait();
}

/**
 * Set Keys response callback.
 *
 * @param pucFrame - pointer to response frame
 * @param pusLength - response frame length
 * @return
 */
eMBException
cmd_setkeys_callback( UCHAR * pucFrame, USHORT * pusLength )
{
    cmdFrameHeader *pFrame = (cmdFrameHeader *)(pucFrame - 1);
    switch (pFrame->status)
    {
    case BOOT_OK:
        DEBUG_PUTSTRING ("OK");
        break;
    default:
        DEBUG_PUTSTRING1("Key Upload error: ", pFrame->status);
        break;
    }
    cmd_done(pFrame->status);
    return MB_EX_NONE;
}

/**
 * Lock Keys response callback.
 * @param pucFrame - pointer to response frame
 * @param pusLength - response frame length
 * @return
 */
eMBException
cmd_lockkeys_callback( UCHAR * pucFrame, USHORT * pusLength )
{
    cmdFrameHeader *pFrame = (cmdFrameHeader *)(pucFrame - 1);
    switch (pFrame->status)
    {
    case BOOT_OK:
        printf ("Lock Keys OK\n");
        break;
    default:
        printf ("Lock Keys failed: %d\n", pFrame->status);
        break;
    }
    cmd_done(pFrame->status);
    return MB_EX_NONE;
}

/**
 * @}
 */
