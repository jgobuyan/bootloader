/**
 * encryption.c
 *
 * Blowfish encryption/RSA signing routines
 *  Created on: 2013-04-10
 *      Author: jeromeg
 * @addtogroup Encryption
 * @{
 */

#include <string.h>
#include "port.h"
#include "fwheader.h"
#include "blowfish.h"
#include "crypto.h"
#include "flashmap.h"
#include "platform.h"
#include "keyfile.h"
#include "encryption.h"

static BLOWFISH_context context;
static const KeyRing *keyring = (const KeyRing *)FLASH_KEY_BASE;
extern void *ax_malloc(int size);
extern void ax_free(void *addr);

/**
 * Decrypt uploaded block using Blowfish.
 * If key is not programmed, do not decrypt.
 *
 * @param pData - pointer to buffered data
 * @param size - size of block
 */
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
 * Validate signature. The embedded signature data is an RSA signed form of
 * the header information and MD5 digest inserted after compilation.
 * @param pHdr - pointer to the Flash partition
 * @return FALSE if OK, TRUE if error
 */
BOOL validate_signature (fwHeader *pHdr )
{
    LONG len;
    RSA_CTX *rsa_context = 0;
    MD5_CTX md5_context;
    sigFile *pSig;
    UCHAR md5_digest[MD5_SIZE];
    BOOL ret = FALSE;
    UCHAR *data = (UCHAR *)(keyring + 1);
    UCHAR *out_data;
    DEBUG_PUTSTRING("VALIDATE");

    /* Create public key from parameters in Flash */
    RSA_pub_key_new(&rsa_context, &data[keyring->rsa_modulus_offset],
            keyring->rsa_modulus_size, &data[keyring->rsa_exponent_offset],
            keyring->rsa_exponent_size);
    out_data = ax_malloc(rsa_context->num_octets);
    pSig = (sigFile *)out_data;
    DEBUG_PUTSTRING("DECRYPT");
    len = RSA_decrypt(rsa_context, (const UCHAR *)(&pHdr->sig), out_data, FALSE);
    DEBUG_PUTSTRING1("MAGIC: ", pSig->info.magic);

    /* Calculate message digest */
    MD5_Init(&md5_context);
    MD5_Update(&md5_context, (const uint8_t *)(pHdr + 1), pHdr->info.length);
    MD5_Final(&md5_digest[0], &md5_context);

    /* Compare header info */
    if (len < 0)
    {
        DEBUG_PUTSTRING("Decryption Error");
        ret = TRUE;
    }
    else if (memcmp(&pSig->info, &pHdr->info, sizeof(fwInfo)))

    {
        DEBUG_PUTSTRING("Header Mismatch");
        ret = TRUE;
    }
    /* Compare MD5 digest */
    else if (memcmp(&pSig->md5_digest, &md5_digest, MD5_SIZE))
    {
        DEBUG_PUTSTRING("MD5 Mismatch");
        ret = TRUE;
    }
    else
    {
        DEBUG_PUTSTRING("Header/MD5 OK");
    }
    RSA_free(rsa_context);
    ax_free(out_data);
    DEBUG_PUTSTRING("VALIDATE COMPLETE");
    return ret;
}

/**
 * @}
 */
