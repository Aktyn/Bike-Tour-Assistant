#ifndef BIKETOURASSISTANT_TOUR_H
#define BIKETOURASSISTANT_TOUR_H

#include <cstdint>

class Tour {
public:
  Tour();

  void setZoom(uint8_t zoom);

  void clear();

  void clear(uint16_t expectedPointsCount);

  void pushPoint(uint16_t pointIndex, float latitude, float longitude);

private:
  uint8_t zoom;
  uint16_t expectedPointsCount;
};


#endif //BIKETOURASSISTANT_TOUR_H
