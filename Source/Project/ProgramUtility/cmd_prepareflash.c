/**
 * cmd_prepareflash.c
 *
 *  Created on: 2013-04-06
 *      Author: jeromeg
 */

#include <stdio.h>
#include "bootloader.h"
#include "fwheader.h"
#include "commands.h"
#include "platform.h"

/**
 * Send Prepare Flash request
 *
 * @param ucMBaddr
 * @param ucBank
 * @return
 */
UCHAR cmd_prepareflash(UCHAR ucMBaddr, UCHAR ucBank)
{
    UCHAR    buf[8];
    cmdFrameHeader *pFrame = (cmdFrameHeader *)buf;
    DEBUG_PUTSTRING1("PREPARE_FLASH addr=", ucMBaddr);
    pFrame->mbAddr = ucMBaddr;
    pFrame->cmdId = MB_FUNC_BOOT_PREPAREFLASH;
    pFrame->subcmdId = ucBank;
    pFrame->status = 0;
    cmd_start();
    eMBSendFrame(buf, MB_FUNC_BOOT_PREPAREFLASH_SIZE + 1);
    return cmd_status_wait();
}

/**
 * Process Get Header response
 * @param pucFrame
 * @param pusLength
 * @return
 */
eMBException
cmd_prepareflash_callback( UCHAR * pucFrame, USHORT * pusLength )
{
    cmdFrameHeader *pFrame = (cmdFrameHeader *)(pucFrame - 1);
    DEBUG_PUTSTRING("PREPARE_FLASH CALLBACK");
    switch (pFrame->status)
    {
    case BOOT_OK:
        printf ("Selected Bank: %d\n", pFrame->subcmdId);
        break;
    default:
        printf ("PrepareFlash: Unknown response\n");
        break;
    }
    cmd_done(pFrame->status);
    return MB_EX_NONE;
}
