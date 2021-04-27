#ifndef DEBUG_H
#define	DEBUG_H

// #define _DEBUG_ENABLE_
//#define _DEBUG_TWI_

#ifdef _DEBUG_ENABLE_
void debugStr(char *data);
void debugHex(char data);
#else
#define debugStr(x)
#define debugHex(x)
#endif

#ifdef _DEBUG_TWI_
#define debugTWIStr(x) (debugStr(x))
#else
#define debugTWIStr(x)
#endif

#endif	/* DEBUG_H */

