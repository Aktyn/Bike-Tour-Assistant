#include "core.h"
#include "display/draw.h"
#include "GUI_BMP.h"
#include "utils.h"

#include <cmath>

Core &CORE = Core::getInstance();

Core::Core() : isBluetoothConnected(false), isRunning(false), needMapRedraw(false), fetchingTile(nullptr) {
  this->mapZoom = 0; // 0 means there is no tiles registered yet
  this->location = {0, 0, 0, 0, 0, 0};
}

Core::~Core() {
  this->clearTiles();
}

void Core::start() {
  this->isRunning = true;
}

void Core::clearTiles() {
  for (const auto &tile: this->tiles) {
    delete tile.second;
  }
  this->tiles.clear();
  this->fetchingTile = nullptr;
}

void
Core::registerTile(uint32_t x, uint32_t y, uint32_t z, uint32_t dataByteLength) {
  if (z != this->mapZoom) {
    // Discard loaded tiles if zoom level has changed
    this->clearTiles();
    this->mapZoom = z;
    this->tour.setZoom(z);
  }

  Tile *tile = new Tile(x, y, z, dataByteLength);
  this->tiles[Tile::getTileKey(x, y, z)] = tile;
  this->fetchingTile = tile;
}

void Core::appendTileImageData(uint16_t chunkIndex, uint8_t *data) {
  if (this->fetchingTile == nullptr) {
    std::cerr << "No fetching tile" << std::endl;
    return;
  }

  this->fetchingTile->appendPngData(chunkIndex, data);

  if (this->fetchingTile->isFullyLoaded()) {
    std::cout << "Tile " << this->fetchingTile->key << " is fully loaded" << std::endl;
    this->fetchingTile = nullptr;
    this->needMapRedraw = true;
  }
}

void Core::updateLocation(float latitude, float longitude, float speed, float heading, uint64_t timestamp) {
  auto previousUpdateTimestamp = this->location.timestamp;

  if (std::round(this->location.speed) != std::round(metersPerSecondToKmPerHour(speed))) {
    this->needSpeedRedraw = true;
  }

  this->location.latitude = latitude;
  this->location.longitude = longitude;
  this->location.speed = metersPerSecondToKmPerHour(speed);
  this->location.heading = heading;
  this->location.timestamp = timestamp;
  this->location.previousUpdateTimestamp = previousUpdateTimestamp;

}

uint16_t *Core::generateMap() {
  //TODO: draw dynamic map based on location and loaded tiles

  if (this->tiles.empty()) {
    return nullptr;
  }

  Tile *tile = this->tiles.begin()->second;
  if (!tile->isFullyLoaded() || tile->imageData.empty()) {
    return nullptr;
  }

  uint16_t *buffer = allocateImageBuffer(MAP_WIDTH, MAP_HEIGHT);
  for (uint16_t y = 0; y < MAP_HEIGHT; y++) {
    for (uint16_t x = 0; x < MAP_WIDTH; x++) {
      uint16_t index = (MAP_HEIGHT - 1 - y) * MAP_WIDTH + (MAP_WIDTH - 1 - x);
      uint16_t tileIndex = (y % tile->tileWidth) * tile->tileWidth + (x % tile->tileHeight);
      buffer[index] = convertRgbColor(
          RGB(
              tile->imageData[tileIndex * 3 + 0],
              tile->imageData[tileIndex * 3 + 1],
              tile->imageData[tileIndex * 3 + 2]
          )
      );
    }
  }

  return buffer; // buffer must be freed
}

uint8_t Core::getMapZoom() const {
  return this->mapZoom;
}

bool isBluetoothDisconnected() {
  return !CORE.isBluetoothConnected;
}