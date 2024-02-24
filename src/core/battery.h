#ifndef BIKETOURASSISTANT_BATTERY_H
#define BIKETOURASSISTANT_BATTERY_H

#include <cstdint>

class Battery {
public:
  Battery();

  bool needRedraw;

  uint8_t getPercentage() const; // 0-100

//  void update(); //TODO: also monitor device temperature and show proper warning if it's too high
};


#endif //BIKETOURASSISTANT_BATTERY_H
