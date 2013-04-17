/**
 * bootloader.h
 *
 *  Created on: 2013-04-05
 *      Author: jeromeg
 */

#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_
#include "port.h"
#include "mb.h"
#include "mbframe.h"
#define MB_FUNC_BOOT_GETHEADER          ( 100 )
#define MB_FUNC_BOOT_PREPAREFLASH       ( 101 )
#define MB_FUNC_BOOT_UPLOADBLOCK        ( 102 )
#define MB_FUNC_BOOT_VALIDATEIMAGE      ( 103 )

#define VERSION_STRING_LENGTH   32
#define SIGNATURE_LENGTH        446
#define FW_MAGIC                0x494e4547  /* "GENI" in little endian */

/* ----------------------- Type definitions ---------------------------------*/
typedef enum
{
    BOOT_OK = 0x00,
    BOOT_BANKEMPTY = 0x01,
    BOOT_BADHCRC = 0x02,
    BOOT_BADDCRC = 0x03,
    BOOT_BADBLKNUM = 0x04,
    BOOT_BADSIG = 0x05,
    BOOT_ERROR = 0x06,
    BOOT_UNVALIDATED = 0x07,
    BOOT_EXIT = 0xfd,
    BOOT_TIMEOUT = 0xfe,
    BOOT_INVALID = 0xff,
} eBootErrorCode;

typedef struct
{
    UCHAR   mbAddr;
    UCHAR   cmdId;
    UCHAR   subcmdId;
    UCHAR   status;
} cmdFrameHeader;

/* Bank ID Definitions */
#define BANK_BOOT       0
#define BANK_A          1
#define BANK_B          2
#define BANK_F          3
#define SEQNUM_MASK     0x7fffffff
#define UPLOAD_BLOCK_SIZE   1024
/* Boot Command PDU Definitions */
#define MB_PDU_FUNC_BOOT_SUBID_OFF          ( MB_PDU_DATA_OFF )
#define MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF     ( MB_PDU_DATA_OFF + 1 )

#define MB_PDU_FUNC_BOOT_BANK_OFF           ( MB_PDU_FUNC_BOOT_SUBID_OFF )
#define MB_PDU_FUNC_BOOT_FWHEADER_OFF       ( MB_PDU_DATA_OFF + 2 )

#define MB_PDU_FUNC_BOOT_BLOCKNUM_OFF       ( MB_PDU_FUNC_BOOT_SUBID_OFF )
#define MB_PDU_FUNC_BOOT_BLOCKDATA_OFF      ( MB_PDU_DATA_OFF + 2 )

#define MB_FUNC_BOOT_DEFAULT_RESP_SIZE      3
#define MB_FUNC_BOOT_GETHEADER_REQ_SIZE     3
#define MB_FUNC_BOOT_GETHEADER_RESP_SIZE    67
#define MB_FUNC_BOOT_PREPAREFLASH_SIZE      3
#define MB_FUNC_BOOT_UPLOADBLOCK_REQ_SIZE   (UPLOAD_BLOCK_SIZE + 3)
#define MB_FUNC_BOOT_VALIDATEIMAGE_SIZE     3

void mbBootInit( void );

#endif /* BOOTLOADER_H_ */
