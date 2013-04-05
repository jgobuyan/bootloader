#ifndef __PLATFORM_H
#define __PLATFORM_H
#define DEBUG_PUTSTRING(s)  DebugPutString(s)
#define DEBUG_PUTSTRING1(s, n)  DebugPutString1(s, n)
void platform_init(void);
void DebugPutString(uint8_t *s);
void DebugPutString1(uint8_t *s, uint32_t n);

#endif  /* __COMMON_H */
