/**
 * encryption.h
 *
 *  Created on: 2013-04-10
 *      Author: jeromeg
 */

#ifndef ENCRYPTION_H_
#define ENCRYPTION_H_

void block_decrypt(void *pData, uint32_t size);
BOOL validate_signature (fwHeader *pHdr );

#endif /* ENCRYPTION_H_ */