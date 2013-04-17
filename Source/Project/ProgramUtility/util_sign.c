/**
 * util_sign.c
 *
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

#define KEYARRAY_SIZE   2048
static RSA_CTX *rsa_context;
static UCHAR keyarray[2048];    /* Flash block size */

static int get_publickey(const uint8_t *cert, int *offset)
{
    int i;
    int index = 0;
    int ret = X509_NOT_OK;
    KeyRing *pKeyfile = (KeyRing *)keyarray;
    uint8_t *pData = (uint8_t *)(pKeyfile + 1);
    uint8_t *modulus = NULL, *pub_exp = NULL;
    printf("PUBLIC KEY\n");
    if (asn1_next_obj(cert, offset, ASN1_SEQUENCE) < 0 ||
            asn1_skip_obj(cert, offset, ASN1_SEQUENCE) ||
            asn1_next_obj(cert, offset, ASN1_BIT_STRING) < 0)
    {
        printf("Err1\n");
        goto end_pub_key;
    }

    (*offset)++;        /* ignore the padding bit field */
    if (asn1_next_obj(cert, offset, ASN1_SEQUENCE) < 0)
    {
        printf("Err2\n");
        goto end_pub_key;
    }
    pKeyfile->rsa_modulus_size = asn1_get_int(cert, offset, &modulus);
    pKeyfile->rsa_exponent_size = asn1_get_int(cert, offset, &pub_exp);
    printf("mod_len=%ld pub_len=%ld\n", pKeyfile->rsa_modulus_size,
            pKeyfile->rsa_exponent_size);
    RSA_pub_key_new(&rsa_context, modulus, pKeyfile->rsa_modulus_size, pub_exp,
            pKeyfile->rsa_exponent_size);

    /* Copy modulus to keyfile */
    pKeyfile->rsa_modulus_offset = index;
    memcpy(&pData[index], modulus, pKeyfile->rsa_modulus_size);
    index += pKeyfile->rsa_modulus_size;
    pKeyfile->rsa_exponent_offset = index;
    /* Copy exponent to keyfile */
    memcpy(&pData[index], pub_exp, pKeyfile->rsa_exponent_size);
    free(modulus);
    free(pub_exp);
#if 1
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
#endif
    ret = 0;

end_pub_key:
    printf ("get_publickey ret=%d\n", ret);
    return ret;
}

void free_publickey(void)
{
    RSA_free(rsa_context);
}

/**
 * Sign the file
 * @param pOutfile
 * @param size
 * @param dsa_keystring
 */
void util_sign(UCHAR *data, ULONG size, char *rsa_keyfile, char *bf_keystring)
{
    (void)data;
    (void)size;
    (void)rsa_keyfile;
}

int util_createkeyfile(char *outfile, char *rsa_keyfile, char *bf_keystring)
{
    int fdout;
    /* Clear memory first */
    memset(keyarray, 0, KEYARRAY_SIZE);

    KeyRing *pKeyfile = (KeyRing *)keyarray;
    pKeyfile->bf_key_lock = 0xffffffff;
    if (util_set_rsakey(rsa_keyfile))
    {
        return TRUE;
    }

    fdout = open(outfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fdout == -1)
    {
        perror("Could not open output");
        return TRUE;
    }
    util_str2key(bf_keystring, &pKeyfile->bf_key[0], &pKeyfile->bf_key_size);
    write(fdout, keyarray, KEYARRAY_SIZE);
    close(fdout);
    return FALSE;
}

int util_set_rsakey(char *rsa_keyfile)
{
    int ret = FALSE;
    int fdin;
    int len;
    int offset = 0;
    char *pInfile;
    struct stat sb;
    fdin = open(rsa_keyfile, O_RDONLY);
    if (fdin == -1)
    {
        perror("Could not open input");
        return TRUE;
    }
    fstat(fdin, &sb);
    pInfile = mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fdin, 0);
    ret = get_publickey(pInfile, &offset);
    munmap(pInfile, sb.st_size);
    close(fdin);
    return ret;
}
