#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <vector>
#include <string>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

char *executeCommand(const char *command);

std::string pwd();

int safeCreateDirectory(const char *path);

void createOrReplaceFileFromBinaryData(const std::string& filePath, const uint8_t* data, uint32_t size);

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

void uint32ToBytes(uint32_t value, uint8_t *bytes, bool big_endian);

double metersPerSecondToKmPerHour(double metersPerSecond);

double distanceBetweenCoordinates(double lat1, double lon1, double lat2, double lon2); // in meters

double degreesToRadians(double degrees);

double radiansToDegrees(double radians);

double mix(double a, double b, double mix);

template<typename T>
T calculateLinearlyWeightedAverage(const std::vector<T> &values);

#endif // UTILS_H