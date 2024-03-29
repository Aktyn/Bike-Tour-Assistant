#ifndef DISPLAY_DRAW_H
#define DISPLAY_DRAW_H

#include "fonts.h"
#include <cstdint>

typedef enum TEXT_ALIGN {
  ALIGN_LEFT = 1,
  ALIGN_RIGHT,
  ALIGN_CENTER
} TEXT_ALIGN;

uint16_t *allocateImageBuffer(uint16_t width, uint16_t height);

uint16_t *allocateImageBufferFromBitmapFile(const char *path, uint16_t width, uint16_t height);

void drawImageBuffer(const uint16_t *image, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

void drawImageFromBitmapFile(const char *path, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

int drawImageFromJpgFile(const char *path, uint16_t x, uint16_t y, uint16_t target_width);

void drawTextLine(const char *text,
                  uint16_t x, uint16_t y, uint16_t width,
                  uint16_t color, uint16_t background, sFONT *font, TEXT_ALIGN align);

void clearScreen(uint16_t color);

#endif // DISPLAY_DRAW_H