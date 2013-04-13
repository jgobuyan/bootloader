/**
 * util_addheader.c
 *
 *  Created on: 2013-04-09
 *      Author: jeromeg
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "fwheader.h"
#include "crc.h"
#include "bootloader.h"
#define CRC_INIT_VALUE  0x00000000

/**
 * Add firmware header to a binary. Optionally sign and encrypt.
 * @param infile - input binary file
 * @param outfile - output binary file
 * @param version - version string
 * @param dsa_key - signature key. If NULL, no signature is inserted.
 * @param bf_key - Blowfish encryption key. If NULL, no encryption is performed.
 * @return
 */
int util_addheader(char *infile, char *outfile, char *version, char *dsa_key, char *bf_key)
{
    fwHeader header;
    int fdin;
    int fdout;
    char *pInfile;
    char *pOutfile;
    int len;
    struct stat sb;

    memset(&header, 0, sizeof(fwHeader));
    fdin = open(infile, O_RDONLY);
    if (fdin == -1)
    {
        perror("Could not open input");
        return TRUE;
    }
    fstat(fdin, &sb);

    fdout = open(outfile, O_RDWR | O_CREAT, sb.st_mode);
    if (fdout == -1)
    {
        perror("Could not open output");
        return TRUE;
    }

    pInfile = mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fdin, 0);

    /* Fill in header */
    header.info.magic = FW_MAGIC;
    strncpy(&header.info.version[0], version, VERSION_STRING_LENGTH);
    header.info.length = sb.st_size;
    header.info.dcrc = crc32(pInfile, sb.st_size);
    printf("DCRC = %08x\n", header.info.dcrc);
    header.info.hcrc = crc32((char *) &header.info,
            sizeof(fwInfo) - sizeof(ULONG));
    printf("HCRC = %08x\n", header.info.hcrc);
    write(fdout, &header, sizeof(header));
    write(fdout, pInfile, sb.st_size);
    munmap(pInfile, sb.st_size);
    close(fdin);

    /* Round up to next block size */
    len = sb.st_size + sizeof(header);
    while (len & (UPLOAD_BLOCK_SIZE - 1))
    {
        write(fdout, "\0", 1);
        len++;
    }

    pOutfile = mmap(0, len, PROT_READ | PROT_WRITE,
            MAP_SHARED, fdout, 0);

    /* Add signature */
    if (dsa_key)
    {
        util_sign(pOutfile, dsa_key);
    }

    /* Encrypt */
    if (bf_key)
    {
        util_encrypt(pOutfile, bf_key);
    }

    munmap(pInfile, len);
    close(fdout);
    return FALSE;
}

/**
 * Check firmware header
 * @param infile
 * @return TRUE for failure
 */
int util_checkheader(char *infile)
{
    fwHeader *pHeader;
    uint32_t dcrc;
    uint32_t hcrc;
    int fdin;
    char *pInfile;
    struct stat sb;
    int ret = FALSE;

    fdin = open(infile, O_RDONLY);
    if (fdin == -1)
    {
        perror("Could not open input");
        return TRUE;
    }

    fstat(fdin, &sb);
    pInfile = mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fdin, 0);
    pHeader = (fwHeader *)pInfile;
    /* Fill in header */
    if (pHeader->info.magic != FW_MAGIC)
    {
        printf("Bad image magic\n");
        ret = TRUE;
    }
    else
    {
        printf("\n----------------------------------------\n");
        printf("Version: %s\n", pHeader->info.version);
        printf("Size:    %d\n", pHeader->info.length);

        dcrc = crc32(&pInfile[512], pHeader->info.length);
        printf("DCRC:    %08x ", dcrc);
        if (pHeader->info.dcrc == dcrc)
        {
            printf("(OK)\n");
        }
        else
        {
            printf("(BAD, expected %08x)\n", pHeader->info.dcrc);
            ret = TRUE;
        }
        hcrc = crc32((char *) &pHeader->info,
                sizeof(fwInfo) - sizeof(ULONG));
        printf("HCRC:    %08x ", hcrc);
        if (pHeader->info.hcrc == hcrc)
        {
            printf("(OK)\n");
        }
        else
        {
            printf("(BAD, expected %08x)\n", pHeader->info.hcrc);
            ret = TRUE;
        }
        munmap(pInfile, sb.st_size);
        close(fdin);
    }
    return ret;
}
