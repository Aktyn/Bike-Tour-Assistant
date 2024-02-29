#ifndef BIKETOURASSISTANT_CORE_COMMON_H
#define BIKETOURASSISTANT_CORE_COMMON_H

#include <cstdint>
#include <iostream>
#include <vector>
#include <map>

extern "C"
{
#include "LCD_2inch4.h"
}

#define MAP_WIDTH LCD_2IN4_WIDTH
#define MAP_HEIGHT LCD_2IN4_WIDTH
#define TOP_PANEL_HEIGHT (LCD_2IN4_HEIGHT - MAP_HEIGHT)

struct Location {
  double latitude;
  double longitude;
  double speed;
  /**
   * Degrees starting at due north and continuing clockwise around the compass.
   * Thus, north is 0 degrees, east is 90 degrees, south is 180 degrees, and so on.
   * */
  double heading;
  double altitude;
  double altitudeAccuracy;
  double accuracy;

  uint64_t timestamp;
  uint64_t previousUpdateTimestamp;
};

struct Icons {
  std::vector<uint8_t> directionArrowImageData;
  std::pair<uint16_t, uint16_t> directionArrowSize;
  std::vector<uint8_t> slopeUphillImageData;
  std::vector<uint8_t> slopeDownhillImageData;
  std::pair<uint16_t, uint16_t> slopeIconSize;
  std::vector<std::vector<uint8_t>> digits40x80ImageData;
};

double calculateSlope(const std::vector<Location> &locationHistory);

#endif //BIKETOURASSISTANT_CORE_COMMON_H
