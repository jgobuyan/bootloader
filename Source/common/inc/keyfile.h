/**
 * keyfile.h
 *
 *  Created on: 2013-04-17
 *      Author: jeromeg
 */

#ifndef KEYFILE_H_
#define KEYFILE_H_
#define KEYARRAY_SIZE   2048

/**
 * Key Ring header.
 * The Blowfish key is contained in the header.
 * The RSA key is contained after the header. The offsets in the header
 * point to the byte locations.
 */
typedef struct {
    ULONG bf_key_lock;          /**< Flash lock - Unlocked = 0xffffffff */
    ULONG bf_key_size;          /**< Blowfish Key size                  */
    UCHAR bf_key[256];          /**< Blowfish Key array                 */
    LONG rsa_modulus_size;      /**< RSA pubkey modulus size in bytes   */
    LONG rsa_modulus_offset;    /**< RSA pubkey modulus offset          */
    LONG rsa_exponent_size;     /**< RSA pubkey exponent size in bytes  */
    LONG rsa_exponent_offset;   /**< RSA pubkey exponent offset         */
} KeyRing;

#endif /* KEYFILE_H_ */
