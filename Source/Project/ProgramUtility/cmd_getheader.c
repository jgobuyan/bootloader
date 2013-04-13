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
UCHAR cmd_getheader(UCHAR ucMBaddr, UCHAR ucBank)
{
    UCHAR    buf[8];
    cmdFrameHeader *pFrame = (cmdFrameHeader *)buf;
    DEBUG_PUTSTRING1("GET_HEADER addr=", ucMBaddr);
    DEBUG_PUTSTRING1("GET_HEADER bank=", ucBank);
    pFrame->mbAddr = ucMBaddr;
    pFrame->cmdId = MB_FUNC_BOOT_GETHEADER;
    pFrame->subcmdId = ucBank;
    pFrame->status = 0;
    cmd_start();
    eMBSendFrame(buf, MB_FUNC_BOOT_GETHEADER_REQ_SIZE + 1);
    return cmd_status_wait();
}

/**
 * Process Get Header response
 * @param pucFrame
 * @param pusLength
 * @return
 */
eMBException
cmd_getheader_callback( UCHAR * pucFrame, USHORT * pusLength )
{
    cmdFrameHeader *pFrame = (cmdFrameHeader *)(pucFrame - 1);
    fwInfo *info = (fwInfo *)&pucFrame[MB_PDU_FUNC_BOOT_FWHEADER_OFF];
    switch (pFrame->status)
    {
    case BOOT_OK:
        printf ("Version: %s\n", info->version);
        break;
    case BOOT_BANKEMPTY:
        printf ("Bank Empty\n");
        break;
    case BOOT_BADHCRC:
        printf ("Bad Header CRC\n");
        break;
    case BOOT_BADDCRC:
        printf ("Bad Data CRC\n");
        break;
    }
    cmd_done(pFrame->status);
    return MB_EX_NONE;
}
