/**
 * commands.h
 *
 *  Created on: 2013-04-09
 *      Author: jeromeg
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_
#include "mb.h"
#include "mbport.h"
#include "bootloader.h"

UCHAR cmd_getheader(UCHAR ucMBAddr, UCHAR ucBank);
UCHAR cmd_prepareflash(UCHAR ucMBAddr);
UCHAR cmd_uploadblock(UCHAR ucMBaddr, UCHAR ucBlock, UCHAR *pucData, USHORT usLen);

void cmd_start(void);
void cmd_done(UCHAR status);
UCHAR cmd_status(void);
UCHAR cmd_status_wait(void);
BOOL cmd_in_progress(void);

int util_addheader(char *infile, char *outfile, char *version, char *dsa_key,
        char *bf_key);
int util_checkheader(char *infile);
int util_upload(UCHAR ucMBaddr, char *infile);
void util_encrypt(UCHAR *pOutfile, ULONG size, UCHAR *bf_keystring);

/* ModBus master commands */
eMBErrorCode eMBSendFrame(UCHAR *ucMBFrame, USHORT usLength);
eMBErrorCode eMBRegisterTimeoutCB( BOOL(*pxHandler)(void));
eMBErrorCode eMBRegisterIllegalFuncCB(BOOL (*pxHandler) (void) );

eMBException cmd_timeout_callback( UCHAR * pucFrame, USHORT * pusLength );
eMBException cmd_getheader_callback( UCHAR * pucFrame, USHORT * pusLength );
eMBException cmd_prepareflash_callback( UCHAR * pucFrame, USHORT * pusLength );
eMBException cmd_uploadblock_callback( UCHAR * pucFrame, USHORT * pusLength );
eMBException cmd_validatesig_callback( UCHAR * pucFrame, USHORT * pusLength );
eMBException cmd_illegalfunc_callback( UCHAR * pucFrame, USHORT * pusLength );

#endif /* COMMANDS_H_ */
