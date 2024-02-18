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

float bytesToFloat(uint8_t *bytes, bool big_endian);

uint16_t bytesToUint16(uint8_t *bytes, bool big_endian);

uint32_t bytesToUint32(uint8_t *bytes, bool big_endian);

uint64_t bytesToUint64(uint8_t *bytes, bool big_endian);

float metersPerSecondToKmPerHour(float metersPerSecond);

#endif // __UTILS_H