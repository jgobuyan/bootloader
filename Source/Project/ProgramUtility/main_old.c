/*
 * FreeModbus Libary: Linux Demo Application
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: demo.c,v 1.2 2006/10/12 08:12:06 wolti Exp $
 */

/* ----------------------- Standard includes --------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "bootloader.h"
/* ----------------------- Defines ------------------------------------------*/
#define PROG            "prog_util"

#define REG_INPUT_START 1000
#define REG_INPUT_NREGS 4
#define REG_HOLDING_START 2000
#define REG_HOLDING_NREGS 130

/* ----------------------- Static variables ---------------------------------*/

static BOOL     bDoExit;
static UCHAR    ucMBFrame[1048];
extern eMBErrorCode eMBSendFrame(UCHAR *ucMBFrame, USHORT usLength);

/* ----------------------- Static functions ---------------------------------*/

/* ----------------------- Start implementation -----------------------------*/
BOOL
bSetSignal( int iSignalNr, void ( *pSigHandler ) ( int ) )
{
    BOOL            bResult;
    struct sigaction xNewSig, xOldSig;

    xNewSig.sa_handler = pSigHandler;
    sigemptyset( &xNewSig.sa_mask );
    xNewSig.sa_flags = 0;
    if( sigaction( iSignalNr, &xNewSig, &xOldSig ) != 0 )
    {
        bResult = FALSE;
    }
    else
    {
        bResult = TRUE;
    }
    return bResult;
}

void
vSigShutdown( int xSigNr )
{
    switch ( xSigNr )
    {
    case SIGQUIT:
    case SIGINT:
    case SIGTERM:
        bDoExit = TRUE;
        break;
    }
}

eMBException
eMBGetHeaderCB( UCHAR * pucFrame, USHORT * pusLength )
{
    fwInfo *info = pucFrame[MB_PDU_FUNC_BOOT_FWHEADER_OFF];
    printf ("Version: %s\n", info->version);
    return MB_EX_NONE;
}

int
main( int argc, char *argv[] )
{
    int             iExitCode;
    CHAR            cCh;

    if( !bSetSignal( SIGQUIT, vSigShutdown ) ||
        !bSetSignal( SIGINT, vSigShutdown ) || !bSetSignal( SIGTERM, vSigShutdown ) )
    {
        fprintf( stderr, "%s: can't install signal handlers: %s!\n", PROG, strerror( errno ) );
        iExitCode = EXIT_FAILURE;
    }
    else if( eMBInit( MB_RTU, 0x0A, 2, 115200, MB_PAR_EVEN ) != MB_ENOERR )
    {
        fprintf( stderr, "%s: can't initialize modbus stack!\n", PROG );
        iExitCode = EXIT_FAILURE;
    }
    else
    {
        /* Register Callbacks
         *
         */

        eMBRegisterCB(MB_FUNC_BOOT_GETHEADER, eMBGetHeaderCB);

        /* CLI interface. */
        printf( "Type 'q' for quit or 'h' for help!\n" );
        bDoExit = FALSE;
        do
        {
            printf( "> " );
            cCh = getchar(  );

            switch ( cCh )
            {
            case 'q':
                bDoExit = TRUE;
                break;
            case 'b':
                ucMBFrame[0] = 1;
                ucMBFrame[1] = MB_FUNC_BOOT_GETHEADER;
                ucMBFrame[2] = 0;
                ucMBFrame[3] = 0;
                eMBSendFrame(ucMBFrame, MB_FUNC_BOOT_GETHEADER_REQ_SIZE);
                break;
            case 'h':
                printf( "Programming Utility application help:\n" );
                printf( "  'b' ... query firmware version\n" );
                printf( "  'q' ... quit application.\n" );
                printf( "  'h' ... this information.\n" );
                printf( "\n" );
                printf( "Copyright 2006 Christian Walter <wolti@sil.at>\n" );
                break;
            default:
                if( !bDoExit && ( cCh != '\n' ) )
                {
                    printf( "illegal command '%c'!\n", cCh );
                }
                break;
            }

            /* eat up everything untill return character. */
            while( !bDoExit && ( cCh != '\n' ) )
            {
                cCh = getchar(  );
            }
        }
        while( !bDoExit );

        /* Release hardware resources. */
        ( void )eMBClose(  );
        iExitCode = EXIT_SUCCESS;
    }
    return iExitCode;
}


