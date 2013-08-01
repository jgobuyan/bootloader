/**
 * util_sign.c
 *
 * Signature and Key File Utilities
 *  Created on: 2013-04-12
 *      Author: jeromeg
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include "port.h"
#include "crypto_misc.h"
#include "keyfile.h"
#include "commands.h"
#include "fwheader.h"
#include "encryption.h"
#include "platform.h"

#define NUM_RETRIES 5
/**
 * RSA Key context pointer
 */
static RSA_CTX *rsa_context;

/**
 * Key Ring buffer
 */
static UCHAR keyarray[KEYARRAY_SIZE];    /* Flash block size */

/**
 * Signature file buffer
 */
static sigFile sig;

/**
 * Get RSA key info from the private DER file. The public elements are saved
 * into the Key Ring buffer.
 * @param buf
 * @param len
 * @return
 */
int get_key(const uint8_t *buf)
{
    KeyRing *pKeyfile = (KeyRing *)keyarray;
    uint8_t *pData = (uint8_t *)(pKeyfile + 1);
    int i;
    int index = 0;
    int offset = 7;
    uint8_t *modulus = NULL, *priv_exp = NULL, *pub_exp = NULL;
    int mod_len, priv_len, pub_len;

    /* not in der format */
    if (buf[0] != ASN1_SEQUENCE) /* basic sanity check */
    {
        printf("Error: This is not a valid ASN.1 file\n");
        return X509_INVALID_PRIV_KEY;
    }
    mod_len = asn1_get_int(buf, &offset, &modulus);
    pub_len = asn1_get_int(buf, &offset, &pub_exp);
    priv_len = asn1_get_int(buf, &offset, &priv_exp);

    if (mod_len <= 0 || pub_len <= 0 || priv_len <= 0)
        return X509_INVALID_PRIV_KEY;

    RSA_priv_key_new(&rsa_context,
            modulus, mod_len, pub_exp, pub_len, priv_exp, priv_len);
    pKeyfile->rsa_modulus_size = mod_len;
    pKeyfile->rsa_exponent_size = pub_len;
    printf("mod_len=%ld pub_len=%ld priv_len=%d\n", pKeyfile->rsa_modulus_size,
            pKeyfile->rsa_exponent_size, priv_len);

    /* Copy modulus to keyfile */
    pKeyfile->rsa_modulus_offset = index;
    memcpy(&pData[index], modulus, pKeyfile->rsa_modulus_size);
    index += pKeyfile->rsa_modulus_size;
    pKeyfile->rsa_exponent_offset = index;
    /* Copy exponent to keyfile */
    memcpy(&pData[index], pub_exp, pKeyfile->rsa_exponent_size);

    printf("modulus: ");
    for (i = 0; i < pKeyfile->rsa_modulus_size; i++)
    {
        printf("%02x:", pData[pKeyfile->rsa_modulus_offset + i]);
    }
    printf("\n");
    printf("exponent: ");
    for (i = 0; i < pKeyfile->rsa_exponent_size; i++)
    {
        printf("%02x:", pData[pKeyfile->rsa_exponent_offset + i]);
    }
    printf("\n");

    free(modulus);
    free(priv_exp);
    free(pub_exp);
    return X509_OK;
}

/**
 * Free the key context
 */
void free_key(void)
{
    RSA_free(rsa_context);
}

/**
 * Create keyfile and output to a file or upload to the board.
 *
 * If outfile is not null, the keyfile is written to a file. Otherwise it
 * is uploaded to the board.
 *
 * @param outfile
 * @param rsa_keyfile
 * @param bf_keystring
 * @return
 */
int util_createkeyfile(UCHAR ucMBaddr, char *outfile, char *rsa_keyfile, char *bf_keystring)
{
    int fdout;
    UCHAR ucStatus;
    BOOL ret = FALSE;
    /* Clear memory first */
    memset(keyarray, 0, KEYARRAY_SIZE);

    KeyRing *pKeyfile = (KeyRing *) keyarray;
    pKeyfile->bf_key_lock = 0xffffffff;
    if (util_set_rsakey(rsa_keyfile))
    {
        return TRUE;
    }
    if (outfile)
    {
        /* outfile is specified, so open keyfile */
        fdout = open(outfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (fdout == -1)
        {
            perror("Could not open output");
            return TRUE;
        }
    }

    util_str2key(bf_keystring, &pKeyfile->bf_key[0], &pKeyfile->bf_key_size);
    if (outfile)
    {
        /* outfile is specified, so write keyfile */
        write(fdout, keyarray, KEYARRAY_SIZE);
        close(fdout);
    }
    else
    {
        int index;
        int retry;
        /* Upload keyfile to board */
        printf("Uploading keys:\n");
        while (index < KEYARRAY_SIZE)
        {
            DEBUG_PUTSTRING1("Key Block ", index);
            retry = NUM_RETRIES;
            ucStatus = cmd_setkeys(ucMBaddr, index / UPLOAD_BLOCK_SIZE,
                    &keyarray[index], UPLOAD_BLOCK_SIZE);
            while (ucStatus != BOOT_OK)
            {
                if (ucStatus == BOOT_LOCKED)
                {
                    fprintf(stderr, "Error: Keys are now locked.\n");
                    ret = TRUE;
                    break;
                }
                if ((--retry) == 0)
                {
                    fprintf(stderr, "Too many retries at block %d\n",
                            index / UPLOAD_BLOCK_SIZE);
                    ret = TRUE;
                    break;
                }
                printf("R");
                fflush(stdout);
                ucStatus = cmd_setkeys(ucMBaddr, index / UPLOAD_BLOCK_SIZE,
                        &keyarray[index], UPLOAD_BLOCK_SIZE);
            }
            if (ret)
            {
                break;
            }
            printf("*");
            fflush(stdout);
            index += UPLOAD_BLOCK_SIZE;
        }
    }
    return ret;
}

/**
 * Read the DER formatted RSA key and populate the key ring buffer.
 *
 * @param rsa_keyfile
 * @return
 */
int util_set_rsakey(char *rsa_keyfile)
{
    int ret = FALSE;
    int fdin;
    uint8_t *pInfile;
    struct stat sb;
    fdin = open(rsa_keyfile, O_RDONLY);
    if (fdin == -1)
    {
        perror("Could not open input");
        return TRUE;
    }
    fstat(fdin, &sb);
    pInfile = mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fdin, 0);
    ret = get_key(pInfile);
    munmap(pInfile, sb.st_size);
    close(fdin);
    return ret;
}

/**
 * Sign the signature file.
 *
 * An MD5 digest is calculated over the firmware image and added to the
 * signature file. The signature file is signed by encrypting it using
 * the RSA private key.
 * @param data - pointer to firmware image
 * @param size - size of firmware image
 * @param rsa_keystring
 */
void util_sign(UCHAR *data, ULONG size, char *rsa_keyfile)
{
    int i;
    MD5_CTX md5_context;
    fwHeader *pHeader = (fwHeader *)data;
    /* Calculate message digest over the code image */
    MD5_Init(&md5_context);
    MD5_Update(&md5_context, &data[sizeof(fwHeader)], pHeader->info.length);
    MD5_Final(&sig.md5_digest[0], &md5_context);

    printf("Digest: ");
    for (i = 0; i < MD5_SIZE; i++)
    {
        printf("%02x", sig.md5_digest[i]);

    }
    printf("\n");

    /* Copy firmware info to signature file */
    memcpy(&sig.info, &pHeader->info, sizeof(fwInfo));
    util_set_rsakey(rsa_keyfile);
    pHeader->siglength = RSA_encrypt(rsa_context, (uint8_t *)&sig, sizeof(sig),
            pHeader->sig, TRUE);
    printf("Signature length=%d\n", pHeader->siglength);
}
