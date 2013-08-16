/**
 * @file cmd_getheader.c
 *
 * Get firmware header from board.
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
 * Send Get Header request and wait for response. The following ModBus
 * packet is sent to the board.
 *
 * <TABLE>
 * <TR><TH>Byte</TH><TH>Description </TH><TH>Value      </TH></TR>
 * <TR><TD>0</TD><TD>ModBusAddress  </TD><TD>0x01       </TD></TR>
 * <TR><TD>1</TD><TD>FuncID         </TD><TD>0x64       </TD></TR>
 * <TR><TD>2</TD><TD>Bank           </TD><TD>   0 = Bootloader<BR>
 *                                              1 = Bank A<BR>
 *                                              2 = Bank B<BR>
 *                                              3 = Bank F<BR>
 *                                      </TD></TR>
 * <TR><TD>3</TD><TD>Unused         </TD><TD>0x00       </TD></TR>
 * <TR><TD>4</TD><TD>CRCL           </TD><TD>           </TD></TR>
 * <TR><TD>5</TD><TD>CRCH           </TD><TD>           </TD></TR>
 * </TABLE>
 * @param ucMBaddr - ModBus Address
 * @param ucBank - Bank (Boot = 0, A = 1, B = 2 or F = 3)
 * @return response status
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
 * Process Get Header response callback.The following ModBus
 * packet is sent by the board.
 *
 * <TABLE>
 * <TR><TH>Byte</TH><TH>Description </TH><TH>Value      </TH></TR>
 * <TR><TD>0</TD><TD>ModBusAddress  </TD><TD>0x01       </TD></TR>
 * <TR><TD>1</TD><TD>FuncID         </TD><TD>0x64       </TD></TR>
 * <TR><TD>2</TD><TD>Bank           </TD><TD>   0 = Bootloader<BR>
 *                                              1 = Bank A<BR>
 *                                              2 = Bank B<BR>
 *                                              3 = Bank F<BR>
 *                                      </TD></TR>
 * <TR><TD>3</TD><TD>Status         </TD><TD>   OK<BR>
 *                                              BANKEMPTY<BR>
 *                                              BADHCRC<BR>
 *                                              BADCCRC<BR>
 *                                                      </TD></TR>
 * <TR><TD>4-7</TD><TD>Seq Number   </TD><TD>           </TD></TR>
 * <TR><TD>8-11</TD><TD>"GENI"      </TD><TD>           </TD></TR>
 * <TR><TD>12-43</TD><TD>Version    </TD><TD>           </TD></TR>
 * <TR><TD>44-47</TD><TD>Length     </TD><TD>           </TD></TR>
 * <TR><TD>48-51</TD><TD>Data CRC   </TD><TD>           </TD></TR>
 * <TR><TD>52-63</TD><TD>Reserved   </TD><TD>           </TD></TR>
 * <TR><TD>64-67</TD><TD>Header CRC </TD><TD>           </TD></TR>
 * <TR><TD>68</TD><TD>CRCL          </TD><TD>           </TD></TR>
 * <TR><TD>69</TD><TD>CRCH          </TD><TD>           </TD></TR>
 * </TABLE>
 * @param pucFrame - pointer to ModBus response frame
 * @param pusLength - length of response frame
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

/**
 * @}
 */
