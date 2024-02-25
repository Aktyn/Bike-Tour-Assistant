#ifndef CORE_H
#define CORE_H

#include "tile.h"
#include "tour.h"
#include "battery.h"
#include "common.h"

#include <cstdint>
#include <iostream>
#include <map>
#include <chrono>

#define LOCATION_HISTORY_SIZE 8

using milliseconds = std::chrono::milliseconds;
using nanoseconds = std::chrono::nanoseconds;
using timestamp = std::chrono::time_point<std::chrono::system_clock, nanoseconds>;

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
  bool needSlopeRedraw;

  void start();

  void reset();

  void update();

  void registerActivity();

  void setBacklight(uint8_t lightness);

  void registerTile(uint32_t x, uint32_t y, uint32_t z, uint32_t dataByteLength);

  void appendTileImageData(uint16_t chunkIndex, uint8_t *data);

  void updateLocation(double latitude, double longitude,
                      double speed, double heading,
                      double altitude, double altitudeAccuracy, double accuracy, uint64_t timestamp);

  void drawMap();

  double getSlope() const;

  uint8_t getMapZoom() const;

  const Icons &getIcons() const;
private:
  Core();

  ~Core();

  void clearTiles();

  timestamp lastActivityTime;
  bool isInactive;
  uint8_t backlightLightness; // 0-100

  std::map<std::string, Tile *> tiles;
  Tile *fetchingTile;
  uint8_t mapZoom;

  Icons icons;
  std::vector <Location> locationHistory;
};

extern Core &CORE;

bool isBluetoothDisconnected();

#endif // CORE_H
