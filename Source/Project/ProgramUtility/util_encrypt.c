/**
 * util_encrypt.c
 *
 *  Created on: 2013-04-12
 *      Author: jeromeg
 */

#include <stdio.h>
#include "port.h"
#include "blowfish.h"

static BLOWFISH_context bf_context;
static UCHAR bf_key[256];
static ULONG bf_keylength;
/**
 * Convert string of hex characters and put results into
 * the specified key array.
 * Valid characters are '0'-'9', 'a'-'f' and 'A'-'F'.
 * Periods can be used as separators and are ignored.
 * The key terminates on the first invalid character or the
 * end of the string. A warning is issued when an invalid
 * character is encountered.
 *
 * @param keystring - NULL terminated hex string
 * @param keyarray - UCHAR array holding key
 * @param len - key length
 */
void util_str2key(CHAR *keystring, UCHAR *keyarray, ULONG *keylength)
{
    UCHAR i = 0;
    UCHAR ucNibble;
    UCHAR done = FALSE;
    while ((*keystring != 0) && !done)
    {
        if (*keystring >= '0' && *keystring <= '9')
        {
            ucNibble = *keystring - '0';
        }
        else if (*keystring >= 'a' && *keystring <= 'f')
        {
            ucNibble = *keystring - 'a' + 10;
        }
        else if (*keystring >= 'A' && *keystring <= 'F')
        {
            ucNibble = *keystring - 'A' + 10;
        }
        else if (*keystring == '.')
        {
            ucNibble = '.';
        }
        else
        {
            ucNibble = 0xff;
        }
        switch (ucNibble)
        {
        case '.':
            break;
        case 0xff:
            printf("Warning: Invalid character truncates key.\n");
            done = TRUE;
            break;
        default:
            if (!(i & 0x01))
            {
                /* This is the first nibble. */
                keyarray[i >> 1] = ucNibble << 4;
            }
            else
            {
                /* This is the second nibble. */
                keyarray[i >> 1] |= ucNibble;
            }
            i++;
            break;
        }
        keystring++;
    }
    *keylength = i >> 1;
    printf("Key size = %d bytes (%d bits)\n", *keylength, *keylength * 8);
}

/**
 * Encrypt file using Blowfish cipher
 * @param pOutfile
 * @param size
 * @param bf_keystring
 */
void util_encrypt(UCHAR *pOutfile, ULONG size, char *bf_keystring)
{
    util_str2key(bf_keystring, &bf_key[0], &bf_keylength);
    printf("Blowfish status: %d\n",blowfish_setkey( &bf_context, bf_key, bf_keylength ));
    blowfish_setkey( &bf_context, bf_key, bf_keylength );
    /* Encrypt in-place */
    while (size > 0)
    {
        blowfish_encrypt_block( &bf_context, pOutfile, pOutfile );
        pOutfile += BLOWFISH_BLOCKSIZE;
        size -= BLOWFISH_BLOCKSIZE;
    }
}
