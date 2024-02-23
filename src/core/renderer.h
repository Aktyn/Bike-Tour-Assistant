#ifndef BIKETOURASSISTANT_RENDERER_H
#define BIKETOURASSISTANT_RENDERER_H

#include "tile.h"
#include "tour.h"
#include "common.h"

#include <cstdint>
#include <iostream>
#include <map>

namespace renderer {
  uint16_t *renderMap(
      const std::map<std::string, Tile *> &tiles,
      Tour &tour,
      const Location &location,
      uint8_t mapZoom
  ); // Returns a pointer to the rendered map (user must free the memory after use)
}

#endif //BIKETOURASSISTANT_RENDERER_H
