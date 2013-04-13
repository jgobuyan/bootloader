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
static UCHAR key[256];
static ULONG key_size;
/**
 * Convert string of hex characters and put results into
 * the key array.
 * Valid characters are '0'-'9', 'a'-'f' and 'A'-'F'.
 * Periods can be used as separators and are ignored.
 * The key terminates on the first invalid character or the
 * end of the string. A warning is issued when an invalid
 * character is encountered.
 *
 * @param bf_keystring - NULL terminate string
 */
static void str2key(UCHAR *bf_keystring)
{
    UCHAR i = 0;
    UCHAR ucNibble;
    UCHAR done = FALSE;
    while ((bf_keystring != 0) && !done)
    {
        if (*bf_keystring >= '0' || *bf_keystring <= '9')
        {
            ucNibble = *bf_keystring - '0';
        }
        else if (*bf_keystring >= 'a' || *bf_keystring <= 'f')
        {
            ucNibble = *bf_keystring - 'a' + 10;
        }
        else if (*bf_keystring >= 'A' || *bf_keystring <= 'F')
        {
            ucNibble = *bf_keystring - 'A' + 10;
        }
        else if (*bf_keystring == '.')
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
            printf("Blowfish: Invalid character truncates key.\n");
            done = TRUE;
            break;
        default:
            if (!(i & 0x01))
            {
                /* This is the first nibble. */
                key[i] = ucNibble << 4;
            }
            else
            {
                key[i] |= ucNibble;
                i++;
            }
            break;
        }
        bf_keystring++;
    }
    key_size = i;
    printf("Blowfish key size = %d bytes (%d bits)\n", key_size, key_size * 8);
}

void util_encrypt(UCHAR *pOutfile, ULONG size, UCHAR *bf_keystring)
{
    str2key(bf_keystring);
    printf("Blowfish status: %d\n",blowfish_setkey( &bf_context, key, key_size ));
    blowfish_setkey( &bf_context, key, key_size );
    /* Encrypt in-place */
    while (size > 0)
    {
        blowfish_encrypt_block( &bf_context, pOutfile, pOutfile );
        pOutfile += BLOWFISH_BLOCKSIZE;
        size -= BLOWFISH_BLOCKSIZE;
    }
}
