#ifndef __PLATFORM_H
#define __PLATFORM_H
/**
 * Platform functions
 */
#include <stdint.h>
#define DEBUG_PUTSTRING(s)
#define DEBUG_PUTSTRING1(s, n)
void platform_init(void);
void platform_deinit(void);
void platform_initSerialPort(void *init);
uint32_t platform_getSwitchState(void);
void DebugPutString(char *s);
void DebugPutString1(char *s, uint32_t n);
void platform_redLedFlashOn(void);
void platform_redLedFlashOff(void);
void platform_redLedOn(void);
void platform_redLedOff(void);
void platform_blueLedOn(void);
void platform_blueLedOff(void);
void platform_greenLedOn(void);
void platform_greenLedOff(void);
#endif  /* __COMMON_H */
