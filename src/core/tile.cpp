#include "tile.h"
#include "pngUtils.h"

#include <cstring>

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

