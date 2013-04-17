/**
 * blowfish.h
 *
 *  Created on: 2013-04-12
 *      Author: jeromeg
 */

#ifndef BLOWFISH_H_
#define BLOWFISH_H_
#define BLOWFISH_BLOCKSIZE 8
#define BLOWFISH_ROUNDS 16

typedef struct {
    unsigned long s0[256];
    unsigned long s1[256];
    unsigned long s2[256];
    unsigned long s3[256];
    unsigned long p[BLOWFISH_ROUNDS+2];
} BLOWFISH_context;


int blowfish_setkey(BLOWFISH_context *context, const unsigned char *key,
        unsigned long keylen);
void blowfish_encrypt_block(BLOWFISH_context *context, unsigned char *outbuf,
        const unsigned char *inbuf);
void blowfish_decrypt_block(BLOWFISH_context *context, unsigned char *outbuf,
        const unsigned char *inbuf);

#endif /* BLOWFISH_H_ */
