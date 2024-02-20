#ifndef CORE_H
#define CORE_H

#include "tile.h"
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

  bool isBluetoothConnected;
  bool isRunning;

  bool needMapRedraw;
  bool needSpeedRedraw;
  Location location;

  void start();

  void
  registerTile(uint32_t x, uint32_t y, uint32_t z, uint16_t tileWidth, uint16_t tileHeight, uint32_t dataByteLength,
               uint16_t paletteSize);

  void registerIndexedColor(uint16_t colorIndex, uint8_t red, uint8_t green, uint8_t blue);

  void appendTileImageData(uint16_t chunkIndex, uint8_t *data);

  void updateLocation(float latitude, float longitude, float speed, float heading, uint64_t timestamp);

  uint16_t *generateMap();


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