#include "battery.h"

Battery::Battery() : needRedraw(true) {
  //
}


uint8_t Battery::getPercentage() const {
  return 75;
}
