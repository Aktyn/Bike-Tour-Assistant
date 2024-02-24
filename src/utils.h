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

float bytesToFloat(const uint8_t *bytes, bool big_endian);

double bytesToDouble(const uint8_t *bytes, bool big_endian);

uint16_t bytesToUint16(const uint8_t *bytes, bool big_endian);

uint32_t bytesToUint32(const uint8_t *bytes, bool big_endian);

uint64_t bytesToUint64(const uint8_t *bytes, bool big_endian);

double metersPerSecondToKmPerHour(double metersPerSecond);

double distanceBetweenCoordinates(double lat1, double lon1, double lat2, double lon2); // in meters

double degreesToRadians(double degrees);

double mix(double a, double b, double mix);

#endif // __UTILS_H