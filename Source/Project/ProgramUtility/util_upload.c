/**
 * util_upload.c
 *
 *  Created on: 2013-04-10
 *      Author: jeromeg
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "port.h"
#include "bootloader.h"
#include "commands.h"

int util_upload(UCHAR ucMBaddr, char *infile)
{
    int fdin;
    char *pInfile;
    struct stat sb;
    ULONG index = 0;
    int timeout;
    int retry;
    BOOL ret = FALSE;
    ULONG len;
    UCHAR cmd_status;
    fdin = open(infile, O_RDONLY);
    if (fdin == -1)
    {
        perror("Could not open input");
        return TRUE;
    }

    fstat(fdin, &sb);
    /* Round down to nearest block size */
    len = sb.st_size & ~(UPLOAD_BLOCK_SIZE - 1);
    pInfile = mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fdin, 0);

    /* Query bootloader version to ensure it is alive */
    timeout = 10;
    do
    {
        cmd_status = cmd_getheader(ucMBaddr, BANK_BOOT);
    } while ((--timeout) && (cmd_status != BOOT_OK) && (cmd_status != BOOT_EXIT));

    if (!timeout || (cmd_status == BOOT_EXIT))
    {
        fprintf(stderr, "Timeout waiting for response from target\n");
        ret = TRUE;
    }
    else if (cmd_prepareflash(ucMBaddr) == BOOT_OK)
    {
        /* Do whole blocks first to avoid going over the end of mmap */
        while (index < len)
        {
            printf("Block %d\r", index);
            retry = 3;
            while (cmd_uploadblock(ucMBaddr, index / UPLOAD_BLOCK_SIZE,
                    &pInfile[index], UPLOAD_BLOCK_SIZE) != BOOT_OK)
            {
                if ((--retry) == 0)
                {
                    fprintf(stderr, "Too many retries at block %d\n", index);
                    ret = TRUE;
                    break;
                }
            }
            if (ret)
            {
                break;
            }
            index += UPLOAD_BLOCK_SIZE;
        }
        if (!ret && (len < sb.st_size))
        {
            printf("Block %d\r", index);
            if (cmd_uploadblock(ucMBaddr, index / UPLOAD_BLOCK_SIZE,
                    &pInfile[index], sb.st_size - len) != BOOT_OK)
            {
                fprintf(stderr, "Timeout waiting for block upload\n");
                ret = TRUE;
            }
            else
            {
                printf("Upload complete\n");
            }
        }
    }
    else
    {
        perror("Failed preparing Flash");
        ret = TRUE;
    }
    munmap(pInfile, sb.st_size);
    close(fdin);

    return ret;
}
