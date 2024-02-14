#ifndef __DISPLAY_LOADER_H
#define __DISPLAY_LOADER_H

#include <stdint.h>
#include <stdbool.h>

#define THREE_DOTS_LOADER_WIDTH 64
#define THREE_DOTS_LOADER_HEIGHT 8

void showThreeDotsLoader(uint16_t x, uint16_t y, bool (*condition)());

#endif // __DISPLAY_LOADER_H