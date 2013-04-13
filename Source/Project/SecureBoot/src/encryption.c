/**
 * encryption.c
 *
 *  Created on: 2013-04-10
 *      Author: jeromeg
 */

#include "port.h"
#include "fwheader.h"
#include "blowfish.h"
#include "flashmap.h"
#include "platform.h"
static BLOWFISH_context context;
static const flashKey *keyring = (const flashKey *)FLASH_KEY_BASE;
void block_decrypt(void *pData, uint32_t size)
{
    UCHAR *pucData = pData;
    if (keyring->bf_key_size != 0xffffffff)
    {
        DEBUG_PUTSTRING1("BLOWFISH: ", keyring->bf_key_size);
        blowfish_setkey(&context, keyring->bf_key, keyring->bf_key_size);
        /* Decrypt in-place */
        while (size > 0)
        {
            blowfish_decrypt_block(&context, pucData, pucData);
            pucData += BLOWFISH_BLOCKSIZE;
            size -= BLOWFISH_BLOCKSIZE;
        }
    }
}

BOOL validate_signature (fwHeader *pHdr )
{
    //TODO: Put RSA decrypt interface
    return TRUE;
}
