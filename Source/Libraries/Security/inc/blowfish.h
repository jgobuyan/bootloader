/**
 * blowfish.h
 *
 *  Created on: 2013-04-12
 *      Author: jeromeg
 */

#ifndef BLOWFISH_H_
#define BLOWFISH_H_
#include "port.h"
#define BLOWFISH_BLOCKSIZE 8
#define BLOWFISH_ROUNDS 16

typedef struct {
    ULONG s0[256];
    ULONG s1[256];
    ULONG s2[256];
    ULONG s3[256];
    ULONG p[BLOWFISH_ROUNDS+2];
} BLOWFISH_context;


int blowfish_setkey( BLOWFISH_context *context, const UCHAR *key, ULONG keylen );
void blowfish_encrypt_block( BLOWFISH_context *context, UCHAR *outbuf, const UCHAR *inbuf );
void blowfish_decrypt_block( BLOWFISH_context *context, UCHAR *outbuf, const UCHAR *inbuf );

#endif /* BLOWFISH_H_ */
