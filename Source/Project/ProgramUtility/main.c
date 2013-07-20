/**
 * Program Utility main.c
 *
 *  Created on: 2013-04-05
 *      Author: jeromeg
 */


/* ----------------------- Standard includes --------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <ctype.h>
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "bootloader.h"
#include "platform.h"
#include "commands.h"
/* ----------------------- Defines ------------------------------------------*/
#define PROG            "prog_util"

#define FLAG_GET_VERSION    0x00000001
#define FLAG_ADD_HEADER     0x00000002
#define FLAG_CHECK_HEADER   0x00000004
#define FLAG_UPLOAD         0x00000008
#define FLAG_ENCRYPT        0x00000010
#define FLAG_SIGN           0x00000020
#define FLAG_CREATE_KEYFILE 0x00000040
#define FLAG_VALIDATE       0x80000000  /* Debug */

/* ----------------------- Static variables ---------------------------------*/

static enum ThreadState
{
    STOPPED,
    RUNNING,
    SHUTDOWN
} ePollThreadState;

static pthread_mutex_t xLock = PTHREAD_MUTEX_INITIALIZER;
static BOOL     bDoExit;
static CHAR    *pBlowfishKeyString = NULL;
static CHAR    *pRSAKeyFile = NULL;
static UCHAR    ucPort;
/* ----------------------- Static functions ---------------------------------*/
static BOOL     bCreatePollingThread( void );
static enum ThreadState eGetPollingThreadState( void );
static void     vSetPollingThreadState( enum ThreadState eNewState );
static void    *pvPollingThread( void *pvParameter );

uint8_t debugflags = 0;
char *devString;
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
        vSetPollingThreadState( SHUTDOWN );
        bDoExit = TRUE;
        break;
    }
}

void print_usage(void)
{
    printf("Usage:\n\nprog_util <options> <infile> <outfile>\n\n");
    printf("-a <version-string> <infile> <outfile>  - add image header to binary\n");
    printf("-e <key> -s <rsa-file> -k <outfile>     - create binary key file\n");
    printf("-c <file>                               - check image\n");
    printf("-v <bank>                               - query version\n");
    printf("-u <infile>                             - upload and program flash\n");
    printf("\nOptions:\n");
    printf("-p <port>                               - Serial port number\n");
    printf("-e <key>                                - Blowfish key\n");
    printf("-s <rsa-file>                           - RSA key file (DER format)\n");
    printf("-D                                      - display debug\n");
    printf("\nNotes:\n");
    printf("Blowfish and RSA keys must be specified when -k is used.\n");
}

int
main( int argc, char *argv[] )
{
    int     iExitCode = EXIT_SUCCESS;
    UCHAR   ucMBAddr = 1;
    CHAR    cCh;
    ULONG   ulOptFlags = 0;
    UCHAR   ucBank = 0;
    UCHAR   ucStartThread = FALSE;
    CHAR   *pucVersion;
    int     index;
    int     c;
    int     timeout;
    char   *infile = NULL;
    char   *outfile = NULL;
    char   *endptr;
    opterr = 0;
    ucPort = 2; /* Default to first USB serial dongle */
    /*
     * Process command line options
     */
    while ((c = getopt(argc, argv, "a:ce:kp:s:uv:DV")) != -1)
    {
        switch (c)
        {
        case 'a':
            ulOptFlags |= FLAG_ADD_HEADER;
            pucVersion = optarg;
            break;
        case 'c':
            ulOptFlags |= FLAG_CHECK_HEADER;
            break;
        case 'k':
            ulOptFlags |= FLAG_CREATE_KEYFILE;
            break;
        case 'e':
            ulOptFlags |= FLAG_ENCRYPT;
            pBlowfishKeyString = optarg;
            break;
        case 'p':
            ucPort = strtoul(optarg, &endptr, 0);
            /* If argument is not a pure number, it must be a string */
            if (endptr)
            {
            	ucPort = 255;
            	devString = optarg;
            }
            break;
        case 's':
            ulOptFlags |= FLAG_SIGN;
            pRSAKeyFile = optarg;
            break;
        case 'u':
            ulOptFlags |= FLAG_UPLOAD;
            ucStartThread = TRUE;
            break;
        case 'v':
            ulOptFlags |= FLAG_GET_VERSION;
            ucBank = strtoul(optarg, NULL, 0);
            ucStartThread = TRUE;
            break;
        case 'D':
            debugflags = 1;
            break;
        case 'V':
            ulOptFlags |= FLAG_VALIDATE;
            ucStartThread = TRUE;
            break;
        case '?':
            if (optopt == 'v')
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint(optopt))
                fprintf(stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
            return 1;
        default:
            abort();
            break;
        }
    }
    if (optind < argc)
    {
        infile = argv[optind++];
    }
    if (optind < argc)
    {
        outfile = argv[optind++];
    }

    if (argc < 2)
    {
        print_usage();
    }

    if ((ulOptFlags & FLAG_ADD_HEADER) && (ulOptFlags & FLAG_CREATE_KEYFILE))
    {
        printf("Error: cannot specify both -a and -k options\n");
        abort();
    }
    if ((ulOptFlags & FLAG_CREATE_KEYFILE) && (infile == 0))
    {
        /* Request to upload keyfile */
        ucStartThread = TRUE;
    }

    /*
     * Enable signal handlers
     */
    if( !bSetSignal( SIGQUIT, vSigShutdown ) ||
        !bSetSignal( SIGINT, vSigShutdown ) || !bSetSignal( SIGTERM, vSigShutdown ) )
    {
        fprintf( stderr, "%s: can't install signal handlers: %s!\n", PROG, strerror( errno ) );
        iExitCode = EXIT_FAILURE;
    }
    else if (ucStartThread)
    {
        DEBUG_PUTSTRING("Starting MobBus thread");
        if( eMBInit( MB_RTU, 0x0A, ucPort, 115200, MB_PAR_EVEN ) != MB_ENOERR )
        {
            fprintf( stderr, "%s: can't initialize modbus stack!\n", PROG );
            iExitCode = EXIT_FAILURE;
        }
        else
        {
            /* Register Callbacks */
            (void)eMBRegisterCB(MB_FUNC_BOOT_GETHEADER, cmd_getheader_callback);
            (void)eMBRegisterCB(MB_FUNC_BOOT_PREPAREFLASH, cmd_prepareflash_callback);
            (void)eMBRegisterCB(MB_FUNC_BOOT_UPLOADBLOCK, cmd_uploadblock_callback);
            (void)eMBRegisterCB(MB_FUNC_BOOT_VALIDATEIMAGE, cmd_validatesig_callback);
            (void)eMBRegisterCB(MB_FUNC_BOOT_SETKEYS, cmd_setkeys_callback);
            (void)eMBRegisterCB(MB_FUNC_BOOT_LOCKKEYS, cmd_lockkeys_callback);
            (void)eMBRegisterIllegalFuncCB(cmd_illegalfunc_callback);

            (void)eMBRegisterTimeoutCB( cmd_timeout_callback );

            /* Start polling thread */
            vSetPollingThreadState(STOPPED);

            if (bCreatePollingThread() != TRUE)
            {
                printf("Can't start protocol stack! Already running?\n");
            }
            //vMBPortTimersEnable(  );

        }
    }
    if (iExitCode == EXIT_SUCCESS)
    {
        vSetPollingThreadState(RUNNING);

        /* Process commands and options */
        if (ulOptFlags & FLAG_GET_VERSION)
        {
            timeout = 10;
            while ((cmd_getheader(ucMBAddr, ucBank) == BOOT_TIMEOUT)
                    && (--timeout));
        }
        else if (ulOptFlags & FLAG_UPLOAD)
        {
            if (infile)
            {
                util_upload(ucMBAddr, infile);
            }
            else
            {
                fprintf(stderr, "Upload: missing filename\n");
            }
        }
        else if (ulOptFlags & FLAG_ADD_HEADER)
        {
            if (infile && outfile)
            {
                util_addheader(infile, outfile, pucVersion, pRSAKeyFile, pBlowfishKeyString);
            }
            else
            {
                fprintf(stderr, "Add Header: missing filenames\n");
            }
        }
        else if (ulOptFlags & FLAG_CREATE_KEYFILE)
        {
            util_createkeyfile(ucMBAddr, infile, pRSAKeyFile, pBlowfishKeyString);
        }
        if (ulOptFlags & FLAG_CHECK_HEADER)
        {
            if (infile)
            {
                util_checkheader(infile);
            }
            else
            {
                fprintf(stderr, "Check Header: missing filename\n");
            }
        }
        if (ulOptFlags & FLAG_VALIDATE)
        {
            cmd_validatesig(ucMBAddr);
        }
        /* Release hardware resources. */
        if (ucStartThread)
        {
            ( void )eMBClose(  );
        }
        iExitCode = EXIT_SUCCESS;
    }
    return iExitCode;
}

BOOL

bCreatePollingThread( void )
{
    BOOL            bResult;
    pthread_t       xThread;

    if( eGetPollingThreadState(  ) == STOPPED )
    {
        if( pthread_create( &xThread, NULL, pvPollingThread, NULL ) != 0 )
        {
            bResult = FALSE;
        }
        else
        {
            bResult = TRUE;
        }
    }
    else
    {
        bResult = FALSE;
    }

    return bResult;
}

void           *
pvPollingThread( void *pvParameter )
{
    vSetPollingThreadState( RUNNING );
    DEBUG_PUTSTRING("Thread started!");
    if( eMBEnable(  ) == MB_ENOERR )
    {
        do
        {
            if( eMBPoll(  ) != MB_ENOERR )
                break;
        }
        while( eGetPollingThreadState(  ) != SHUTDOWN );
    }

    ( void )eMBDisable(  );
    vSetPollingThreadState( STOPPED );
    DEBUG_PUTSTRING("Thread stopped!");
    cmd_done(BOOT_EXIT);
    return 0;
}

enum ThreadState
eGetPollingThreadState(  )
{
    enum ThreadState eCurState;

    ( void )pthread_mutex_lock( &xLock );
    eCurState = ePollThreadState;
    ( void )pthread_mutex_unlock( &xLock );

    return eCurState;
}

void
vSetPollingThreadState( enum ThreadState eNewState )
{
    ( void )pthread_mutex_lock( &xLock );
    ePollThreadState = eNewState;
    ( void )pthread_mutex_unlock( &xLock );
}

