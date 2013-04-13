#ifndef __PLATFORM_H
#define __PLATFORM_H
#include <stdio.h>

extern uint8_t debugflags;
#define DEBUG_PUTSTRING(s)  if (debugflags) printf("%s\n", s)
#define DEBUG_PUTSTRING1(s, n)  if (debugflags) printf("%s%x\n", s, n)

#endif  /* __PLATFORM_H */
