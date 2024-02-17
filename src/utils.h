#ifndef __UTILS_H
#define __UTILS_H

#include <stdint.h>

char *executeCommand(const char *command);
int safeCreateDirectory(const char *path);
uint16_t rgbToRgb666(uint8_t red, uint8_t green, uint8_t blue);
uint16_t rgbToRgb565(uint8_t red, uint8_t green, uint8_t blue);
uint16_t rgbToRgb444(uint8_t red, uint8_t green, uint8_t blue);
uint16_t convertRgbColor(uint16_t color);
uint16_t findNextPowerOf2(uint16_t n);

#endif // __UTILS_H