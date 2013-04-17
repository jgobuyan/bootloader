/*
 * fwheader.h
 *
 *  Created on: 2013-04-07
 *      Author: jeromeg
 */

#ifndef FWHEADER_H_
#define FWHEADER_H_
#include "port.h"

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

/* Bank ID Definitions */
#define BANK_BOOT       0
#define BANK_A          1
#define BANK_B          2
#define BANK_F          3

fwHeader *getImageHeader(UCHAR ucBank);

#endif /* FWHEADER_H_ */
