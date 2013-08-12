/**
 * @file cmd_prepareflash.c
 *
 *  Created on: 2013-04-06
 *      Author: jeromeg
 */

/**
 * @addtogroup BootloaderCommand
 * @{
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
 * Get Header response callback
 * @param pucFrame - pointer to response frame
 * @param pusLength - response frame length
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
        printf ("Selected Bank: ");
        switch(pFrame->subcmdId)
        {
        case 1:
            printf("A\n");
            break;
        case 2:
            printf("B\n");
            break;
        case 3:
            printf("F\n");
            break;
        default:
            printf("Unknown\n");
            break;
        }
        break;
    default:
        printf ("PrepareFlash: %s\n", cmd_errorString(pFrame->status));
        break;
    }
    cmd_done(pFrame->status);
    return MB_EX_NONE;
}

/**
 * @}
 */
