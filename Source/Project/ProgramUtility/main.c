/**
 * @file main.c
 *
 * This program is a multipurpose utility that runs on a workstation. It
 * creates firmware images and key files and provides the capability to
 * upload them to the STM32.
 *
 *  Created on: 2013-04-05
 *      Author: jeromeg
 */

/**
 * @mainpage
 *
 * # Program Utility for the STM32 Secure Bootloader
 *
 * Usage: prog_util {options} {infile} {outfile}
 *
 * ## Options
 *
 * -a {version-string}
 * -b {bank-number}
 * -c                   Check image
 */

/**
 * @addtogroup main
 * @{
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

#define FLAG_GET_VERSION    0x00000001  /**< Get Version Header         */
#define FLAG_ADD_HEADER     0x00000002  /**< Add header to binary       */
#define FLAG_CHECK_HEADER   0x00000004  /**< Check header               */
#define FLAG_UPLOAD         0x00000008  /**< Upload software image      */
#define FLAG_ENCRYPT        0x00000010  /**< Encrypt image              */
#define FLAG_SIGN           0x00000020  /**< Sign image                 */
#define FLAG_CREATE_KEYFILE 0x00000040  /**< Create/Upload key file     */
#define FLAG_LOCK_FILE      0x00000080  /**< Lock key file/factor bank  */
#define FLAG_VALIDATE       0x80000000  /**< Validate signature (debug) */

/* ----------------------- Static variables ---------------------------------*/

static enum ThreadState
{
    STOPPED,//!< STOPPED
    RUNNING,//!< RUNNING
    SHUTDOWN//!< SHUTDOWN
} ePollThreadState;

static pthread_mutex_t xLock = PTHREAD_MUTEX_INITIALIZER;
static BOOL     bDoExit = FALSE;
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

/**
 * bSetSignal
 *
 * Assigns a function to be called when a given signal is raised.
 *
 * @param iSignalNr - Signal number
 * @param pSigHandler - Signal handler function
 * @return TRUE if successful, FALSE if not.
 */
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

/**
 * vSigShutdown
 *
 * Signal handler to shut down threads.
 * @param xSigNr
 */
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

/**
 * print_usage
 *
 * Display usage info for the utility
 */
void print_usage(void)
{
    printf("Usage:\n\nprog_util <options> <infile> <outfile>\n\n");
    printf("\nOptions:\n");
    printf("-a <version-string>                     - add image header to binary\n");
    printf("-b <bank>                               - Bank number\n"
           "                                          0 = Bootloader\n"
           "                                          1 = Bank A\n"
           "                                          2 = Bank B\n"
           "                                          3 = Bank F\n");
    printf("-c                                      - check image\n");
    printf("-e <key>                                - Blowfish key\n");
    printf("-k                                      - create/upload binary key file\n");
    printf("-l                                      - lock keys\n");
    printf("-p <port>                               - Serial port number or full name\n");
    printf("-s <rsa-file>                           - RSA signing key file (DER format)\n");
    printf("-u                                      - upload and program flash\n");
    printf("-v                                      - query version\n");
    printf("-D                                      - display debug\n");
    printf("\nNotes:\n");
    printf("Blowfish and RSA keys must be specified when -k is used.\n");
    printf("\nExamples:\n\n"
           "Add image header to binary\n"
           "    prog_util -a \"My Label\" file.bin file.img\n\n"
           "Add image header to binary, sign with RSA and encrypt with Blowfish\n"
           "    prog_util -e 01234567 -s rsa.der -a \"My Label\" file.bin file.img\n\n"
           "Upload RSA and Blowfish keys over serial port\n"
           "    prog_util -e 01234567 -s rsa.der -k -p /dev/ttyS4\n\n"
           "Lock RSA and Blowfish keys over serial port\n"
           "    prog_util -l -p /dev/ttyS4\n\n"
           "Lock factory bank over serial port\n"
           "    prog_util -f -p /dev/ttyS4\n\n"
           "Upload image to least recent bank over serial port\n"
           "    prog_util -p /dev/ttyS4 -u file.img\n\n"
           "Upload image to Bank A over serial port\n"
           "    prog_util -p /dev/ttyS4 -b 1 -u file.img\n\n"
            );

}

/**
 * Main entry point for Program Utility.
 *
 * Process option flags and execute commands.
 *
 * @param argc
 * @param argv
 * @return
 */
int
main( int argc, char *argv[] )
{
    int     iExitCode = EXIT_SUCCESS;
    UCHAR   ucMBAddr = 1;
    ULONG   ulOptFlags = 0;
    UCHAR   ucBank = 0;
    UCHAR   ucStartThread = FALSE;
    CHAR   *pucVersion;
    UCHAR  ucStatus;

    int     c;
    int     timeout;
    char   *infile = NULL;
    char   *outfile = NULL;
    char   *endptr;
    opterr = 0;
    ucPort = 2; /* Default to first USB serial dongle */
    printf("Secure Bootloader Program Utility\n\n");
    /*
     * Process command line options
     */
    while ((c = getopt(argc, argv, "a:b:ce:fklp:s:uvDV")) != -1)
    {
        switch (c)
        {
        case 'a':
            ulOptFlags |= FLAG_ADD_HEADER;
            pucVersion = optarg;
            break;
        case 'b':
            ucBank = strtoul(optarg, NULL, 0);
            break;
        case 'c':
            ulOptFlags |= FLAG_CHECK_HEADER;
            break;
        case 'e':
            ulOptFlags |= FLAG_ENCRYPT;
            pBlowfishKeyString = optarg;
            break;
        case 'f':
            ulOptFlags |= FLAG_LOCK_FILE;
            ucBank = BANK_F;
            ucStartThread = TRUE;
            break;
        case 'k':
            ulOptFlags |= FLAG_CREATE_KEYFILE;
            break;
        case 'l':
            ulOptFlags |= FLAG_LOCK_FILE;
            ucBank = BANK_BOOT;
            ucStartThread = TRUE;
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
            (void)eMBRegisterCB(MB_FUNC_BOOT_LOCK, cmd_lockfile_callback);
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
            ucStatus = cmd_getheader(ucMBAddr, ucBank);
            while ((ucStatus == BOOT_TIMEOUT)
                    && (--timeout))
            {
                ucStatus = cmd_getheader(ucMBAddr, ucBank);
            }
            if ((ucStatus != BOOT_OK) && (ucStatus != BOOT_BANKEMPTY))
            {
                fprintf(stderr, "Get Version Failed: %s", cmd_errorString(ucStatus));
            }

        }
        else if (ulOptFlags & FLAG_UPLOAD)
        {
            if (infile)
            {
                util_upload(ucMBAddr, infile, ucBank);
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
        else if (ulOptFlags & FLAG_LOCK_FILE)
        {
            timeout = 10;
            ucStatus = cmd_lockfile(ucMBAddr, ucBank);
            while ((ucStatus == BOOT_TIMEOUT)
                    && (--timeout))
            {
                ucStatus = cmd_lockfile(ucMBAddr, ucBank);
            }
            if (ucStatus != BOOT_OK)
            {
                fprintf(stderr, "Lock Failed: %s", cmd_errorString(ucStatus));
            }
            else
            {
                printf("Lock Successful\n");
            }

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

/**
 * Create polling thread
 * @return
 */
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

/**
 * Poll the modbus.
 *
 * @param pvParameter
 */
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

/**
 * Get polling thread state
 * @return
 */
enum ThreadState
eGetPollingThreadState(  )
{
    enum ThreadState eCurState;

    ( void )pthread_mutex_lock( &xLock );
    eCurState = ePollThreadState;
    ( void )pthread_mutex_unlock( &xLock );

    return eCurState;
}

/**
 * Set polling thread state
 * @param eNewState
 */
void
vSetPollingThreadState( enum ThreadState eNewState )
{
    ( void )pthread_mutex_lock( &xLock );
    ePollThreadState = eNewState;
    ( void )pthread_mutex_unlock( &xLock );
}

/**
 * @}
 */
