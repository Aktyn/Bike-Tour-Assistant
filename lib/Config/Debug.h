/*****************************************************************************
 * | File      	:	Debug.h
 * | Author      :   Waveshare team
 * | Function    :	debug with printf
 * | Info        :
 *----------------
 * |	This version:   V1.0
 * | Date        :   2018-01-11
 * | Info        :   Basic version
 *
 ******************************************************************************/
#ifndef __DEBUG_H
#define __DEBUG_H

#include <stdio.h>

#if USE_DEBUG
#define DEBUG(__info, ...) printf("Debug: " __info, ##__VA_ARGS__)

inline void ASSERT(unsigned condition, const char *message) {
  if (!condition) {
    fprintf(stderr, "Assertion failed: %s\n", message);
  }
}

#else
#define DEBUG(__info, ...)
inline void ASSERT(unsigned condition, const char *message) {
  // noop
}
#endif

#endif //__DEBUG_H
