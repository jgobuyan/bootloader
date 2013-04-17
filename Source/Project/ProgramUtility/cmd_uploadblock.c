/**
 * cmd_uploadblock.c
 *
 *  Created on: 2013-04-06
 *      Author: jeromeg
 */

#include <stdio.h>
#include <string.h>
#include "bootloader.h"
#include "fwheader.h"
#include "commands.h"
#include "platform.h"

void     display_bytes(void *buf, int len)
{
#if 0
    int i;
    unsigned char *pBuf = buf;
    for (i = 0; i < len; i++)
    {
        if (!(i & 0x07))
        {
            printf("\n%02d: ", i);
        }
        printf("%02x ", pBuf[i]);
    }
    printf("\n");
#endif
}
/**
 * Send upload block request
 *
 * Data is copied into the buffer first. The block is padded to the block size
 * with zeroes before sending.
 * @param ucMBaddr
 * @param ucBlock
 * @param pucData
 * @param usLen
 * @return
 */
UCHAR cmd_uploadblock(UCHAR ucMBaddr, UCHAR ucBlock, UCHAR *pucData, USHORT usLen)
{
    UCHAR    buf[UPLOAD_BLOCK_SIZE + 6];
    cmdFrameHeader *pFrame = (cmdFrameHeader *)buf;
    DEBUG_PUTSTRING1("UPLOAD_BLOCK ", ucBlock);
    /* Clear memory first */
    memset(buf, 0, UPLOAD_BLOCK_SIZE);
    pFrame->mbAddr = ucMBaddr;
    pFrame->cmdId = MB_FUNC_BOOT_UPLOADBLOCK;
    pFrame->subcmdId = ucBlock;
    pFrame->status = 0;
    memcpy(pFrame + 1, pucData, usLen);
    display_bytes(buf, 16);
    cmd_start();
    eMBSendFrame(buf, MB_FUNC_BOOT_UPLOADBLOCK_REQ_SIZE + 1);
    return cmd_status_wait();
}

/**
 * Process Upload Block response
 * @param pucFrame
 * @param pusLength
 * @return
 */
eMBException
cmd_uploadblock_callback( UCHAR * pucFrame, USHORT * pusLength )
{
    cmdFrameHeader *pFrame = (cmdFrameHeader *)(pucFrame - 1);
    fwInfo *info = (fwInfo *)&pucFrame[MB_PDU_FUNC_BOOT_FWHEADER_OFF];
    switch (pFrame->status)
    {
    case BOOT_OK:
        DEBUG_PUTSTRING ("OK");
        break;
    default:
        DEBUG_PUTSTRING1("Upload error: ", pFrame->status);
        break;
    }
    cmd_done(pFrame->status);
    return MB_EX_NONE;
}
