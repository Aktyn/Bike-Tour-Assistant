#include "tile.h"
#include "pngUtils.h"

#include <cstring>
#include <cmath>
#include <tuple>

Tile::Tile(uint32_t x, uint32_t y, uint32_t z,
           uint32_t dataByteLength)
    : x(x), y(y), z(z),
      dataByteLength(dataByteLength), key(Tile::getTileKey(x, y, z)) {

  this->loadedByteLength = 0;
  this->tileWidth = 0;
  this->tileHeight = 0;

  this->pngData = new uint8_t[dataByteLength];
  memset(this->pngData, 0, dataByteLength * sizeof(uint8_t));
}

Tile::~Tile() {
  delete[] this->pngData;
  this->imageData.clear();
}

void Tile::appendPngData(uint16_t chunkIndex, uint8_t *data) {
  uint32_t bytesLeftToLoad = this->dataByteLength - this->loadedByteLength;
  uint32_t chunkSize = TILE_CHUNK_SIZE;
  if (chunkSize > bytesLeftToLoad) {
    chunkSize = bytesLeftToLoad;
  }

  uint32_t offset = TILE_CHUNK_SIZE * uint32_t(chunkIndex);
  memcpy(this->pngData + offset, data, chunkSize);

  this->loadedByteLength += chunkSize;

  if (this->isFullyLoaded() && this->imageData.empty()) {
    auto tileResolution = parsePngData(this->imageData, this->pngData, this->dataByteLength);
    this->tileWidth = std::get<0>(tileResolution);
    this->tileHeight = std::get<1>(tileResolution);
  }
}

bool Tile::isFullyLoaded() const {
  return this->loadedByteLength >= this->dataByteLength;
}


std::string Tile::getTileKey(uint32_t x, uint32_t y, uint32_t z) {
  return std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(z);
}

std::pair<double, double> Tile::convertLatLongToTileXY(float latitude, float longitude, uint8_t zoom) {
  const double latRad = (latitude * M_PI) / 180.0;
  const auto n = double(pow(2.0, zoom));

  const double x = ((longitude + 180.0) / 360.0) * n;
  const double y = ((1.0 - asinh(tan(latRad)) / double(M_PI)) / 2.0) * n;

  return std::make_pair(std::fmod(x, n), std::fmod(y, n));
}