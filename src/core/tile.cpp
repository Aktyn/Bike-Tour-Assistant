#include "tile.h"

#include <cstring>

Tile::Tile(uint32_t x, uint32_t y, uint32_t z,
           uint16_t tileWidth, uint16_t tileHeight,
           uint32_t dataByteLength, uint16_t paletteSize)
    : x(x), y(y), z(z),
      tileWidth(tileWidth), tileHeight(tileHeight),
      dataByteLength(dataByteLength), paletteSize(paletteSize) {

  this->loadedByteLength = 0;

  this->imageData = new uint8_t[dataByteLength];
  memset(this->imageData, 0, dataByteLength * sizeof(uint8_t));
  this->palette = new uint16_t[paletteSize];
  memset(this->palette, 0, paletteSize * sizeof(uint16_t));
}

Tile::~Tile() {
  delete[] this->imageData;
  delete[] this->palette;
}

void Tile::appendImageData(uint16_t chunkIndex, uint8_t *data) {
  uint32_t bytesLeftToLoad = this->dataByteLength - this->loadedByteLength;
  uint32_t chunkSize = TILE_CHUNK_SIZE;
  if (chunkSize > bytesLeftToLoad) {
    chunkSize = bytesLeftToLoad;
  }

  uint32_t offset = TILE_CHUNK_SIZE * uint32_t(chunkIndex);
  memcpy(this->imageData + offset, data, chunkSize);

  this->loadedByteLength += chunkSize;
}

bool Tile::isFullyLoaded() const {
  return this->loadedByteLength >= this->dataByteLength;
}

