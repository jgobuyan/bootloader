/*
 * bootloader.h
 *
 *  Created on: 2013-04-05
 *      Author: jeromeg
 */

#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_
#include "port.h"
#include "mbframe.h"
#define MB_FUNC_BOOT_GETHEADER          ( 100 )
#define MB_FUNC_BOOT_PREPAREFLASH       ( 101 )
#define MB_FUNC_BOOT_UPLOADBLOCK        ( 102 )
#define MB_FUNC_BOOT_GETSIGNATURE       ( 103 )

#define VERSION_STRING_LENGTH   32
#define SIGNATURE_LENGTH        446
#define FW_MAGIC                0x494e4547  /* "GENI" in little endian */

/* ----------------------- Type definitions ---------------------------------*/
typedef struct {
    ULONG   magic;
    CHAR    version[VERSION_STRING_LENGTH];
    ULONG   length;
    ULONG   dcrc;
    UCHAR   reserved[12];
    ULONG   hcrc;
} fwInfo;

typedef struct {
    ULONG   seqNum;
    fwInfo  info;
    USHORT  siglength;
    UCHAR   sig[SIGNATURE_LENGTH];
} fwHeader;

typedef enum
{
    BOOT_OK = 0x00,
    BOOT_BANKEMPTY = 0x01,
    BOOT_BADHCRC = 0x02,
    BOOT_BADDCRC = 0x03,
    BOOT_BADSEQNUM = 0x04,
    BOOT_BADLENGTH = 0x05
} eBootErrorCode;

/* Bank ID Definitions */
#define BANK_BOOT       0
#define BANK_A          1
#define BANK_B          2
#define BANK_F          3
#define SEQNUM_MASK     0x7fffffff

/* Boot Command PDU Definitions */
#define MB_PDU_FUNC_BOOT_SUBID_OFF          ( MB_PDU_DATA_OFF )
#define MB_PDU_FUNC_BOOT_CTRLSTATUS_OFF     ( MB_PDU_DATA_OFF + 1 )

#define MB_PDU_FUNC_BOOT_BANK_OFF           ( MB_PDU_FUNC_BOOT_SUBID_OFF )
#define MB_PDU_FUNC_BOOT_FWHEADER_OFF       ( MB_PDU_DATA_OFF + 3 )

#define MB_PDU_FUNC_BOOT_SEQNUM_OFF         ( MB_PDU_FUNC_BOOT_SUBID_OFF )
#define MB_PDU_FUNC_BOOT_BLOCKDATA_OFF      ( MB_PDU_DATA_OFF + 3 )

#define MB_FUNC_BOOT_GETHEADER_REQ_SIZE     3
#define MB_FUNC_BOOT_GETHEADER_RESP_SIZE    67
#define MB_FUNC_BOOT_PREPAREFLASH_SIZE      3
#define MB_FUNC_BOOT_UPLOADBLOCK_SIZE       1027
#define MB_FUNC_BOOT_GETSIGNATURE_MIN_SIZE  3
#define MB_FUNC_BOOT_GETSIGNATURE_MAX_SIZE  449

/* Flash Memory Map */
#define VTABLE_SIZE         512
#define FLASH_BOOT_BASE     0x08000000
#define FLASH_BOOT_HEADER   (0x08000000 + VTABLE_SIZE)
#define FLASH_BANKA_BASE    0x08002000
#define FLASH_BANKB_BASE    0x08016000
#define FLASH_BANKF_BASE    0x0802a000
#define FLASH_KEY           0x0803e000
void mbBootInit( void );

#endif /* BOOTLOADER_H_ */
