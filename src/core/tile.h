#ifndef TILE_H
#define TILE_H

#include <cstdint>
#include <iostream>
#include <vector>

#define TILE_CHUNK_SIZE 224

class Tile {
public:
  Tile(uint32_t x, uint32_t y, uint8_t z, uint32_t dataByteLength);

  Tile(uint32_t x, uint32_t y, uint8_t z, std::vector<uint8_t> &imageData);

  ~Tile();

  static std::string getTileKey(uint32_t x, uint32_t y, uint8_t z);

  // Returns pointer to a new Tile object that must be deleted by the caller
  static Tile *loadFromCache(uint32_t x, uint32_t y, uint8_t z);

  static std::pair<double, double> convertLatLongToTileXY(double latitude, double longitude, uint8_t zoom);

  const std::string key;
  const uint32_t x;
  const uint32_t y;
  const uint8_t z;
  uint16_t tileWidth;
  uint16_t tileHeight;
  const uint32_t dataByteLength;
  std::vector<uint8_t> imageData;

  void appendPngData(uint16_t chunkIndex, uint8_t *data);

  bool isFullyLoaded() const;

private:
  static void initializeTileCacheDirectory();
  static std::string tilesCacheDirectory;
  uint32_t loadedByteLength;
  uint8_t *pngData;

  void finalize();
};

#endif // TILE_H