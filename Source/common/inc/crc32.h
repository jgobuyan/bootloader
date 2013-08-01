/*
 * crc32.h
 *
 *  Created on: 2013-04-08
 *      Author: jeromeg
 */

#ifndef CRC32_H_
#define CRC32_H_
#include <stdint.h>
#include "port.h"

uint32_t crc32(const void *buf, ULONG size);

#endif /* CRC32_H_ */
