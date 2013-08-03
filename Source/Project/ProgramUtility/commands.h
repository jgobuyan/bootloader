/**
 * commands.h
 *
 *  Created on: 2013-04-09
 *      Author: jeromeg
 *
 * @addtogroup BootloaderCommand
 * @{
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_
#include "mb.h"
#include "mbport.h"
#include "bootloader.h"

UCHAR cmd_getheader(UCHAR ucMBAddr, UCHAR ucBank);
UCHAR cmd_prepareflash(UCHAR ucMBAddr, UCHAR ucBank);
UCHAR cmd_uploadblock(UCHAR ucMBaddr, UCHAR ucBlock, UCHAR *pucData, USHORT usLen);
UCHAR cmd_validatesig(UCHAR ucMBAddr);
UCHAR cmd_setkeys(UCHAR ucMBaddr, UCHAR ucBlock, UCHAR *pucData, USHORT usLen);
UCHAR cmd_lockkeys(UCHAR ucMBAddr);

void cmd_start(void);
void cmd_done(UCHAR status);
UCHAR cmd_status(void);
UCHAR cmd_status_wait(void);

int util_addheader(char *infile, char *outfile, char *version, char *dsa_keystring,
        char *bf_keystring);
int util_checkheader(char *infile);
int util_upload(UCHAR ucMBaddr, char *infile, UCHAR ucBank);
int util_set_rsakey(char *rsa_keyfile);
void util_str2key(char *keystring, UCHAR *keyarray, ULONG *keylength);
void util_encrypt(UCHAR *pOutfile, ULONG size, char *bf_keystring);
void util_sign(UCHAR *data, ULONG size, char *rsa_keyfile);
int util_createkeyfile(UCHAR ucMBaddr, char *outfile, char *rsa_keyfile, char *bf_keystring);

/* ModBus master commands */
eMBErrorCode eMBSendFrame(UCHAR *ucMBFrame, USHORT usLength);
eMBErrorCode eMBRegisterTimeoutCB( BOOL (*pxHandler)(void));
eMBErrorCode eMBRegisterIllegalFuncCB(BOOL (*pxHandler)(void));

eMBException cmd_getheader_callback( UCHAR * pucFrame, USHORT * pusLength );
eMBException cmd_prepareflash_callback( UCHAR * pucFrame, USHORT * pusLength );
eMBException cmd_uploadblock_callback( UCHAR * pucFrame, USHORT * pusLength );
eMBException cmd_validatesig_callback( UCHAR * pucFrame, USHORT * pusLength );
eMBException cmd_setkeys_callback( UCHAR * pucFrame, USHORT * pusLength );
eMBException cmd_lockkeys_callback( UCHAR * pucFrame, USHORT * pusLength );
BOOL cmd_timeout_callback(void);
BOOL cmd_illegalfunc_callback(void);

/**
 * @}
 */
#endif /* COMMANDS_H_ */
