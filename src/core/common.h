#ifndef BIKETOURASSISTANT_CORE_COMMON_H
#define BIKETOURASSISTANT_CORE_COMMON_H

#include <cstdint>

extern "C"
{
#include "LCD_2inch4.h"
}

#define MAP_WIDTH LCD_2IN4_WIDTH
#define MAP_HEIGHT LCD_2IN4_WIDTH

struct Location {
  float latitude;
  float longitude;
  float speed;
  /**
   * Degrees starting at due north and continuing clockwise around the compass.
   * Thus, north is 0 degrees, east is 90 degrees, south is 180 degrees, and so on.
   * */
  float heading;
  uint64_t timestamp;
  uint64_t previousUpdateTimestamp;
};

#endif //BIKETOURASSISTANT_CORE_COMMON_H
