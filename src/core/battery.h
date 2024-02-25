#ifndef BIKETOURASSISTANT_BATTERY_H
#define BIKETOURASSISTANT_BATTERY_H

#include <cstdint>
#include <chrono>

using milliseconds = std::chrono::milliseconds;
using nanoseconds = std::chrono::nanoseconds;
using timestamp = std::chrono::time_point<std::chrono::system_clock, nanoseconds>;

class Battery {
public:
  Battery();

  bool needRedraw;

  uint8_t getPercentage() const; // 0-100
  bool isOverheated() const;

  void update();

private:
  timestamp lastUpdateTime;
  uint8_t percentage;
  uint16_t temperature;
};


#endif //BIKETOURASSISTANT_BATTERY_H
