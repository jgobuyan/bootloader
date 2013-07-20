#ifndef __PLATFORM_H
#define __PLATFORM_H
#define DEBUG_PUTSTRING(s)  DebugPutString(s)
#define DEBUG_PUTSTRING1(s, n)  DebugPutString1(s, n)
void platform_init(void);
void platform_initSerialPort(void *init);
uint32_t platform_getSwitchState(void);
void DebugPutString(char *s);
void DebugPutString1(char *s, uint32_t n);

#endif  /* __COMMON_H */
