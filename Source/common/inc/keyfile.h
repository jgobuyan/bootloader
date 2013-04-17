/**
 * keyfile.h
 *
 *  Created on: 2013-04-17
 *      Author: jeromeg
 */

#ifndef KEYFILE_H_
#define KEYFILE_H_

typedef struct {
    ULONG bf_key_lock;
    ULONG bf_key_size;
    UCHAR bf_key[256];
    LONG rsa_modulus_size;
    LONG rsa_modulus_offset;
    LONG rsa_exponent_size;
    LONG rsa_exponent_offset;
} KeyRing;

#endif /* KEYFILE_H_ */
