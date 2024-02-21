#ifndef CORE_H
#define CORE_H

#include "tile.h"
#include "tour.h"
#include "common.h"

#include <cstdint>
#include <iostream>
#include <map>

extern "C"
{
#include "LCD_2inch4.h"
}

#define MAP_WIDTH LCD_2IN4_WIDTH
#define MAP_HEIGHT LCD_2IN4_WIDTH


class Core {
public:
  Core(const Core &) = delete;

  Core &operator=(const Core &) = delete;

  static Core &getInstance() {
    static Core instance;
    return instance;
  }

  Tour tour;
  Location location;

  bool isBluetoothConnected;
  bool isRunning;

  bool needMapRedraw;
  bool needSpeedRedraw;

  void start();

  void
  registerTile(uint32_t x, uint32_t y, uint32_t z, uint32_t dataByteLength);

  void appendTileImageData(uint16_t chunkIndex, uint8_t *data);

  void updateLocation(float latitude, float longitude, float speed, float heading, uint64_t timestamp);

  uint16_t *generateMap();

  uint8_t getMapZoom() const;

private:
  Core();

  ~Core();

  void clearTiles();

  std::map<std::string, Tile *> tiles;
  Tile *fetchingTile;
  uint8_t mapZoom;
};

extern Core &CORE;

bool isBluetoothDisconnected();

#endif // CORE_H