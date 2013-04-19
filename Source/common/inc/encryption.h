/**
 * encryption.h
 *
 *  Created on: 2013-04-10
 *      Author: jeromeg
 */

#ifndef ENCRYPTION_H_
#define ENCRYPTION_H_
#include "crypto_misc.h"

typedef struct
{
    fwInfo info;
    UCHAR md5_digest[MD5_SIZE];
} sigFile;

void block_decrypt(void *pData, uint32_t size);
BOOL validate_signature (fwHeader *pHdr );

#endif /* ENCRYPTION_H_ */
