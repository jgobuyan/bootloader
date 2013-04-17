/**
 * cmd.c
 *
 * Functions to keep track of commands sent to the device.
 *
 *  Created on: 2013-04-10
 *      Author: jeromeg
 */

#include "bootloader.h"
static BOOL     xCmd_in_progress = 0;
static UCHAR    ucCmd_status;

/**
 * This function is called when a command is sent.
 */
void cmd_start(void)
{
    ucCmd_status = BOOT_INVALID;
    xCmd_in_progress = 1;
}

/**
 * This function is called when a response is received or a timeout occurs
 * @param status
 */
void cmd_done(UCHAR status)
{
    ucCmd_status = status;
    xCmd_in_progress = 0;
}

/**
 * This function is called to get the current command status
 * @return
 */
UCHAR cmd_status(void)
{
    return ucCmd_status;
}

/**
 * This function is called to check if a command is in progress
 * @return
 */
UCHAR cmd_status_wait(void)
{
    while (xCmd_in_progress);
    return ucCmd_status;
}

BOOL cmd_timeout_callback(void)
{
    ucCmd_status = BOOT_TIMEOUT;
    xCmd_in_progress = 0;
    return TRUE;
}

BOOL cmd_illegalfunc_callback(void )
{
    ucCmd_status = BOOT_INVALID;
    xCmd_in_progress = 0;
    return TRUE;
}
