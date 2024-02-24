#ifndef TILE_H
#define TILE_H

#include <cstdint>
#include <iostream>
#include <vector>

#define TILE_CHUNK_SIZE 224

class Tile {
public:
  Tile(uint32_t x, uint32_t y, uint32_t z, uint32_t dataByteLength);

  ~Tile();

  static std::string getTileKey(uint32_t x, uint32_t y, uint32_t z);

  static std::pair<double, double> convertLatLongToTileXY(double latitude, double longitude, uint8_t zoom);

  const std::string key;
  const uint32_t x;
  const uint32_t y;
  const uint32_t z;
  uint16_t tileWidth;
  uint16_t tileHeight;
  const uint32_t dataByteLength;
  uint8_t *pngData;
  std::vector<uint8_t> imageData;

  void appendPngData(uint16_t chunkIndex, uint8_t *data);

  bool isFullyLoaded() const;

private:
  uint32_t loadedByteLength;
};

#endif // TILE_H