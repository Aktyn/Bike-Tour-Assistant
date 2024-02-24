#include "core.h"
#include "utils.h"
#include "renderer.h"
#include "pngUtils.h"

#include <cmath>

Core &CORE = Core::getInstance();

Core::Core() : isBluetoothConnected(false), isRunning(false),
               needMapRedraw(false), needDirectionRedraw(false), needSpeedRedraw(false),
               fetchingTile(nullptr), location({0, 0, 0, 0, 0, 0}) {
  this->mapZoom = 0; // 0 means there is no tiles registered yet

  this->directionArrowSize = loadPngFile(
      this->directionArrowImageData,
      "../assets/direction_arrow_40x40.png", LCT_RGBA
  );
  DEBUG("Direction arrow size: %dx%d\n", this->directionArrowSize.first, this->directionArrowSize.second);
}

Core::~Core() {
  this->clearTiles();
  this->directionArrowImageData.clear();
}

void Core::start() {
  this->isRunning = true;
}

void Core::reset() {
  this->clearTiles();
  this->tour.clear();
  this->needMapRedraw = true;
  this->needSpeedRedraw = true;
  this->needDirectionRedraw = true;
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

void Core::updateLocation(
    double latitude, double longitude, double speed, double heading,
    double altitude, double altitudeAccuracy, double accuracy, uint64_t timestamp
) {
  auto previousUpdateTimestamp = this->location.timestamp;

  if (std::round(this->location.speed) != std::round(metersPerSecondToKmPerHour(speed))) {
    this->location.speed = metersPerSecondToKmPerHour(speed);
    this->needSpeedRedraw = true;
  }

  if (std::round(this->location.heading) != std::round(heading)) {
    this->location.heading = heading;
    this->needMapRedraw = true;
    this->needDirectionRedraw = true;
  }

  double positionDifference = distanceBetweenCoordinates(
      this->location.latitude, this->location.longitude,
      latitude, longitude
  );

  // Update location if the difference is more than 0.5 meters
  if (positionDifference > 0.5) {
    this->location.latitude = latitude;
    this->location.longitude = longitude;
    this->needMapRedraw = true;
  }

  //TODO: calculate and display current slope based on those values
  this->location.altitude = altitude;
  this->location.altitudeAccuracy = altitudeAccuracy;
  this->location.accuracy = accuracy;

  this->location.timestamp = timestamp;
  this->location.previousUpdateTimestamp = previousUpdateTimestamp;
}

void Core::drawMap() {
  try {
    renderer::renderMap(this->tiles, this->tour, this->location, this->mapZoom);
  } catch (const std::exception &e) {
    std::cerr << "Error rendering map: " << e.what() << std::endl;
  }
}

uint8_t Core::getMapZoom() const {
  return this->mapZoom;
}

const std::vector<uint8_t> &Core::getDirectionArrowImageData() const {
  return directionArrowImageData;
}

const std::pair<uint16_t, uint16_t> &Core::getDirectionArrowSize() const {
  return directionArrowSize;
}

bool isBluetoothDisconnected() {
  return !CORE.isBluetoothConnected;
}