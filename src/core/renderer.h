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

  void drawSpeed(double speed, const Icons &icons);

  void drawBattery(uint8_t percentage, bool isOverheated);

  void drawDirectionArrow(double heading, const Icons &icons);

  void drawSlope(double slope, double altitude, const Icons &icons);
}

#endif //BIKETOURASSISTANT_RENDERER_H
