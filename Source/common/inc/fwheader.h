/**
 * @file fwheader.h
 *
 *  Created on: 2013-04-07
 *      Author: jeromeg
 *
 */

#ifndef FWHEADER_H_
#define FWHEADER_H_
#include "port.h"

#define VERSION_STRING_LENGTH   32
#define SIGNATURE_LENGTH        446
#define FW_MAGIC                0x494e4547  /**< "GENI" in little endian */
#define FACTORY_LOCKED          0x00000000  /**< Factory Bank locked     */
#define FACTORY_UNLOCKED        0x55555555  /**< Factory Bank unlocked   */

/* ----------------------- Type definitions ---------------------------------*/

/**
 * Firmware Info Structure
 */
typedef struct {
    ULONG   magic;                          /**< Magic Number ("GENI")  */
    CHAR    version[VERSION_STRING_LENGTH]; /**< Version String         */
    ULONG   length;                         /**< Data Length            */
    ULONG   dcrc;                           /**< Data CRC               */
    UCHAR   reserved[12];                   /**< Reserved (all zeroes   */
    ULONG   hcrc;                           /**< Header CRC             */
} fwInfo;

/**
 * Firmware Header Structure
 */
typedef struct {
    ULONG   seqNum;                         /**< Sequence Number        */
    fwInfo  info;                           /**< Firmware Info          */
    USHORT  siglength;                      /**< Signature Length       */
    UCHAR   sig[SIGNATURE_LENGTH];          /**< Signature Data         */
} fwHeader;

/* Bank ID Definitions */
#define BANK_BOOT       0       /**< Bootload Bank  */
#define BANK_KEY        0       /**< Key File Bank  */
#define BANK_A          1       /**< Bank A         */
#define BANK_B          2       /**< Bank B         */
#define BANK_F          3       /**< Bank F         */
#define BANK_INVALID    255     /**< Invalid Bank   */

fwHeader *getImageHeader(UCHAR ucBank);

#endif /* FWHEADER_H_ */
