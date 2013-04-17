/**
 * encryption.c
 *
 *  Created on: 2013-04-10
 *      Author: jeromeg
 */

#include "port.h"
#include "fwheader.h"
#include "blowfish.h"
//#include "cipher.h"
//#include "dsa.h"
#include "crypto.h"
#include "flashmap.h"
#include "platform.h"
#include "keyfile.h"

static BLOWFISH_context context;
static const KeyRing *keyring = (const KeyRing *)FLASH_KEY_BASE;
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

/**
 * Validate signature
 * @param pHdr
 * @return
 */
BOOL validate_signature (fwHeader *pHdr )
{
#if 1
    //TODO: Put RSA decrypt interface
    ULONG len;
    RSA_CTX *rsa_context;
    UCHAR out_data[64];
    UCHAR modulus[8] = {1,2,3,4,5,6,7,8};
    UCHAR pub_exp[8] = {8,7,6,5,4,3,2,1};
    RSA_pub_key_new(&rsa_context, modulus, 8, pub_exp, 8);
    len = RSA_decrypt(rsa_context, (const UCHAR *)pHdr, out_data, FALSE);
    return TRUE;
#else
    return TRUE;
#endif
}
