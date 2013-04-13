/**
 * cmd_getheader.c
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
 * Send Get Header request
 */
void cmd_validatesig(UCHAR ucMBaddr)
{
    UCHAR    buf[8];
    cmdFrameHeader *pFrame = (cmdFrameHeader *)buf;
    DEBUG_PUTSTRING("VALIDATE_SIG");
    pFrame->mbAddr = ucMBaddr;
    pFrame->cmdId = MB_FUNC_BOOT_GETHEADER;
    pFrame->subcmdId = 0;
    pFrame->status = 0;
    cmd_start();
    eMBSendFrame(buf, MB_FUNC_BOOT_GETHEADER_REQ_SIZE + 1);
}

/**
 * Process Get Header response
 * @param pucFrame
 * @param pusLength
 * @return
 */
eMBException
cmd_validatesig_callback( UCHAR * pucFrame, USHORT * pusLength )
{
    cmdFrameHeader *pFrame = (cmdFrameHeader *)(pucFrame - 1);
    fwInfo *info = (fwInfo *)&pucFrame[MB_PDU_FUNC_BOOT_FWHEADER_OFF];
    switch (pFrame->status)
    {
    case BOOT_OK:
        printf ("Validate OK\n");
        break;
    default:
        printf ("Validate failed\n");
        break;
    }
    cmd_done(pFrame->status);
    return MB_EX_NONE;
}
