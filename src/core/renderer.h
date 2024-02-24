#ifndef BIKETOURASSISTANT_RENDERER_H
#define BIKETOURASSISTANT_RENDERER_H

#include "tile.h"
#include "tour.h"
#include "common.h"

#include <cstdint>
#include <iostream>
#include <map>

namespace renderer {
  void prepareMainView();

  void renderMap(
      const std::map<std::string, Tile *> &tiles,
      Tour &tour,
      const Location &location,
      uint8_t mapZoom
  );

  void drawSpeed(double speed);

  void drawBattery(uint8_t percentage);

  void drawDirectionArrow(double heading,
                          const std::vector<uint8_t> &imageData, const std::pair<uint16_t, uint16_t> &size);
}

#endif //BIKETOURASSISTANT_RENDERER_H
