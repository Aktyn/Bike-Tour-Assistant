#include "core.h"
#include "utils.h"
#include "renderer.h"
#include "pngUtils.h"

#include <cmath>

#define INACTIVITY_TIMEOUT 120000 // 2 minutes in milliseconds

Core &CORE = Core::getInstance();

Core::Core() : isBluetoothConnected(false), isRunning(false), isInactive(false), backlightLightness(100),
               needMapRedraw(false), needSpeedRedraw(false), needDirectionRedraw(false), needSlopeRedraw(false),
               fetchingTile(nullptr), messageOutBuffer{0},
               location({
                            0.0, 0.0, 0.0, 0.0,
                            0.0, 0.0, 0.0,
                            0, 0}) {
  this->mapZoom = 0; // 0 means there are no tiles registered yet
  this->locationHistory.reserve(LOCATION_HISTORY_SIZE);

  this->icons.directionArrowSize = loadPngFile(
      this->icons.directionArrowImageData,
      "../assets/direction_arrow_40x40.png", LCT_RGBA
  );
  this->icons.slopeIconSize = loadPngFile(
      this->icons.slopeUphillImageData,
      "../assets/slope_uphill.png", LCT_RGBA
  ) = loadPngFile(
      this->icons.slopeDownhillImageData,
      "../assets/slope_downhill.png", LCT_RGBA
  );

  memset(this->messageOutBuffer, 0, sizeof(this->messageOutBuffer));
}

Core::~Core() {
  this->clearTiles();
  this->icons.directionArrowImageData.clear();
  this->icons.slopeUphillImageData.clear();
  this->icons.slopeDownhillImageData.clear();
  this->locationHistory.clear();
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
  this->registerActivity();
}

void Core::update() {
  if (!this->isBluetoothConnected) {
    return;
  }

  if (!this->isInactive) {
    this->battery.update();

    auto now = std::chrono::system_clock::now();
    auto delta = std::chrono::duration_cast<milliseconds>(now - this->lastActivityTime);

    if (delta.count() >= INACTIVITY_TIMEOUT) {
      DEBUG("No activity for %u seconds, turning off the display\n", INACTIVITY_TIMEOUT / 1000);
      this->isInactive = true;
      LCD_SetBacklight(0);
    }
  }
}

void Core::registerActivity() {
  if (this->isInactive) {
    this->isInactive = false;
    LCD_SetBacklight(this->backlightLightness * 10);
  }
  this->lastActivityTime = std::chrono::system_clock::now();
}

void Core::setBacklight(uint8_t lightness) {
  LCD_SetBacklight(lightness * 10);
  this->backlightLightness = lightness;
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
    this->registerActivity();
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
    this->registerActivity();
  }

  if (std::round(this->location.heading) != std::round(heading)) {
    this->location.heading = heading;
    this->needMapRedraw = true;
    this->needDirectionRedraw = true;
    this->registerActivity();
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
    this->registerActivity();
  }

  this->location.altitude = altitude;
  this->location.altitudeAccuracy = altitudeAccuracy;
  this->location.accuracy = accuracy;

  this->needSlopeRedraw = true;

  this->location.timestamp = timestamp;
  this->location.previousUpdateTimestamp = previousUpdateTimestamp;

  this->locationHistory.push_back(this->location);
  while (this->locationHistory.size() > LOCATION_HISTORY_SIZE) {
    this->locationHistory.erase(this->locationHistory.begin());
  }
}

void Core::drawMap() {
  try {
    renderer::renderMap(this->tiles, this->tour, this->location, this->mapZoom);
  } catch (const std::exception &e) {
    std::cerr << "Error rendering map: " << e.what() << std::endl;
  }
}

double Core::getSlope() const {
  return calculateSlope(this->locationHistory);
}

uint8_t Core::getMapZoom() const {
  return this->mapZoom;
}

const Icons &Core::getIcons() const {
  return this->icons;
}


bool isBluetoothDisconnected() {
  return !CORE.isBluetoothConnected;
}