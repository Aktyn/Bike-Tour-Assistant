#ifndef CORE_H
#define CORE_H

#include "tile.h"
#include "tour.h"
#include "battery.h"
#include "common.h"

#include <cstdint>
#include <iostream>
#include <map>

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
  Battery battery;

  bool isBluetoothConnected;
  bool isRunning;

  bool needMapRedraw;
  bool needSpeedRedraw;
  bool needDirectionRedraw;

  void start();

  void reset();

  void registerTile(uint32_t x, uint32_t y, uint32_t z, uint32_t dataByteLength);

  void appendTileImageData(uint16_t chunkIndex, uint8_t *data);

  void updateLocation(double latitude, double longitude,
                      double speed, double heading,
                      double altitude, double altitudeAccuracy, double accuracy, uint64_t timestamp);

  void drawMap();

  uint8_t getMapZoom() const;

  const std::vector<uint8_t> &getDirectionArrowImageData() const;

  const std::pair<uint16_t, uint16_t> &getDirectionArrowSize() const;

private:
  Core();

  ~Core();

  void clearTiles();

  std::map<std::string, Tile *> tiles;
  Tile *fetchingTile;
  uint8_t mapZoom;

  std::vector<uint8_t> directionArrowImageData;

private:
  std::pair<uint16_t, uint16_t> directionArrowSize;
};

extern Core &CORE;

bool isBluetoothDisconnected();

#endif // CORE_H