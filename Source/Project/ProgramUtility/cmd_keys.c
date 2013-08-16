/**
 * @file cmd_keys.c
 *
 * Encryption/signature key management
 *
 *  Created on: 2013-07-18
 *      Author: jeromeg
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
 * Binary key file is sent in two 1024 byte blocks. The following ModBus
 * packet is sent to the board.
 *
 * <TABLE>
 * <TR><TH>Byte</TH><TH>Description </TH><TH>Value      </TH></TR>
 * <TR><TD>0</TD><TD>ModBusAddress  </TD><TD>0x01       </TD></TR>
 * <TR><TD>1</TD><TD>FuncID         </TD><TD>0x68       </TD></TR>
 * <TR><TD>2</TD><TD>Block Number   </TD><TD>0 or 1     </TD></TR>
 * <TR><TD>3</TD><TD>Unused         </TD><TD>0x00       </TD></TR>
 * <TR><TD>4</TD><TD>CRCL           </TD><TD>           </TD></TR>
 * <TR><TD>5</TD><TD>CRCH           </TD><TD>           </TD></TR>
 * </TABLE>
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
 * Send Lock File request. This function is used to lock either the
 * decryption/signature keys or the factory bank.
 *
 * The locking of keys will fail if the key partition in Flash is erased (empty).
 * The locking of the factory bank will fail if the bank  contains a corrupt load
 * or is empty.
 *
 * The following ModBus packet is sent to the board.
 *
 * <TABLE>
 * <TR><TH>Byte</TH><TH>Description </TH><TH>Value      </TH></TR>
 * <TR><TD>0</TD><TD>ModBusAddress  </TD><TD>0x01       </TD></TR>
 * <TR><TD>1</TD><TD>FuncID         </TD><TD>0x69       </TD></TR>
 * <TR><TD>2</TD><TD>Block Number   </TD><TD>0 = Lock Keys<BR>
 *                                           3 = Lock Bank F</TD></TR>
 * <TR><TD>3</TD><TD>Unused         </TD><TD>0x00       </TD></TR>
 * <TR><TD>4</TD><TD>CRCL           </TD><TD>           </TD></TR>
 * <TR><TD>5</TD><TD>CRCH           </TD><TD>           </TD></TR>
 * </TABLE>
 *
 * @param ucMBaddr
 * @param ucBank - BANK_BOOT to lock keys, BANK_F to lock factory bank
 * @return
 */
UCHAR cmd_lockfile(UCHAR ucMBaddr, UCHAR ucBank)
{
    UCHAR    buf[8];
    cmdFrameHeader *pFrame = (cmdFrameHeader *)buf;
    if (ucBank == BANK_BOOT)
    {
        DEBUG_PUTSTRING("LOCK KEYS");
    }
    else if (ucBank == BANK_F)
    {
        DEBUG_PUTSTRING("LOCK FACTORY BANK");
    }
    else
    {
        DEBUG_PUTSTRING1("LOCK UNKNOWN BANK ", ucBank);
    }
    pFrame->mbAddr = ucMBaddr;
    pFrame->cmdId = MB_FUNC_BOOT_LOCK;
    pFrame->subcmdId = ucBank;
    pFrame->status = 0;
    cmd_start();
    eMBSendFrame(buf, MB_FUNC_BOOT_LOCKKEYS_SIZE + 1);
    return cmd_status_wait();
}

/**
 * Set Keys response callback.
 * The following ModBus packet is sent by the board.
 *
 * <TABLE>
 * <TR><TH>Byte</TH><TH>Description </TH><TH>Value      </TH></TR>
 * <TR><TD>0</TD><TD>ModBusAddress  </TD><TD>0x01       </TD></TR>
 * <TR><TD>1</TD><TD>FuncID         </TD><TD>0x68       </TD></TR>
 * <TR><TD>2</TD><TD>Block Number   </TD><TD>0 or 1     </TD></TR>
 * <TR><TD>3</TD><TD>Status         </TD><TD>   OK<BR>
 *                                              BANKEMPTY<BR>
 * <TR><TD>4</TD><TD>CRCL           </TD><TD>           </TD></TR>
 * <TR><TD>5</TD><TD>CRCH           </TD><TD>           </TD></TR>
 * </TABLE>
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
 * Lock File response callback.
 *
 * The following ModBus packet is sent by the board.
 *
 * <TABLE>
 * <TR><TH>Byte</TH><TH>Description </TH><TH>Value      </TH></TR>
 * <TR><TD>0</TD><TD>ModBusAddress  </TD><TD>0x01       </TD></TR>
 * <TR><TD>1</TD><TD>FuncID         </TD><TD>0x69       </TD></TR>
 * <TR><TD>2</TD><TD>Block Number   </TD><TD>0 = Lock Keys<BR>
 *                                           3 = Lock Bank F</TD></TR>
 * <TR><TD>3</TD><TD>Status         </TD><TD>   OK<BR>
 *                                              BANKEMPTY<BR>
 *                                              LOCKED<BR>
 *                                              INVALID<BR>
 *                                              BADHCRC<BR>
 *                                              BADCCRC<BR>
 * <TR><TD>4</TD><TD>CRCL           </TD><TD>           </TD></TR>
 * <TR><TD>5</TD><TD>CRCH           </TD><TD>           </TD></TR>
 * </TABLE>
 * @param pucFrame - pointer to response frame
 * @param pusLength - response frame length
 * @return
 */
eMBException
cmd_lockfile_callback( UCHAR * pucFrame, USHORT * pusLength )
{
    cmdFrameHeader *pFrame = (cmdFrameHeader *)(pucFrame - 1);
    if (pFrame->status != BOOT_OK)
    {
        printf ("Lock File failed: %s\n", cmd_errorString(pFrame->status));
    }
    cmd_done(pFrame->status);
    return MB_EX_NONE;
}

/**
 * @}
 */
