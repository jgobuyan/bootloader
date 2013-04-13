#include "bootloader.h"
ULONG   __dummySeqNum;
fwInfo __fwinfo = {
        .magic = FW_MAGIC,
        .version = "Test"
};
