#ifndef TILE_H
#define TILE_H

#include <cstdint>
#include <iostream>

#define TILE_CHUNK_SIZE 224

class Tile {
public:
  Tile(uint32_t x, uint32_t y, uint32_t z,
       uint16_t tileWidth, uint16_t tileHeight,
       uint32_t dataByteLength, uint16_t paletteSize);

  ~Tile();

  static std::string getTileKey(uint32_t x, uint32_t y, uint32_t z) {
    return std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(z);
  }

  const uint32_t x;
  const uint32_t y;
  const uint32_t z;
  const uint16_t tileWidth;
  const uint16_t tileHeight;
  const uint32_t dataByteLength;
  const uint16_t paletteSize;
  uint8_t *imageData;
  uint16_t *palette;

  void appendImageData(uint16_t chunkIndex, uint8_t *data);

  bool isFullyLoaded() const;

private:
  uint32_t loadedByteLength;
};

#endif // TILE_H