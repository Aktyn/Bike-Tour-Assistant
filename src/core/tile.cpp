#include "tile.h"
#include "pngUtils.h"
#include "utils.h"
#include "Debug.h"

#include <cstring>
#include <cmath>
#include <tuple>

std::string Tile::tilesCacheDirectory;

void Tile::initializeTileCacheDirectory() {
  if (Tile::tilesCacheDirectory.empty()) {
    Tile::tilesCacheDirectory = pwd() + "/../tiles_cache";
  }
}

Tile::Tile(uint32_t x, uint32_t y, uint8_t z,
           uint32_t dataByteLength)
    : x(x), y(y), z(z),
      dataByteLength(dataByteLength), key(Tile::getTileKey(x, y, z)) {
  this->loadedByteLength = 0;
  this->tileWidth = 0;
  this->tileHeight = 0;

  if (dataByteLength > 0) {
    this->pngData = new uint8_t[dataByteLength];
    memset(this->pngData, 0, dataByteLength * sizeof(uint8_t));
  } else {
    this->pngData = nullptr;
  }
}

Tile::Tile(uint32_t x, uint32_t y, uint8_t z, std::vector<uint8_t> &imageData) : Tile(x, y, z, 0) {
  this->imageData = imageData;
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
    this->finalize();
  }
}

void Tile::finalize() {
  initializeTileCacheDirectory();

  auto tileResolution = parsePngData(this->imageData, this->pngData, this->dataByteLength);
  this->tileWidth = std::get<0>(tileResolution);
  this->tileHeight = std::get<1>(tileResolution);


  if (safeCreateDirectory(Tile::tilesCacheDirectory.c_str()) != 0) {
    std::cerr << "Error creating directory for tiles cache" << std::endl;
    return;
  }

  std::string tilePath = Tile::tilesCacheDirectory + "/" + this->key + ".png";
  createOrReplaceFileFromBinaryData(tilePath, this->pngData, this->dataByteLength);
  DEBUG("Tile %s saved to %s\n", this->key.c_str(), tilePath.c_str());
}

bool Tile::isFullyLoaded() const {
  return this->loadedByteLength >= this->dataByteLength;
}

std::string Tile::getTileKey(uint32_t x, uint32_t y, uint8_t z) {
  return std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(z);
}

Tile *Tile::loadFromCache(uint32_t x, uint32_t y, uint8_t z) {
  initializeTileCacheDirectory();

  auto tileKey = Tile::getTileKey(x, y, z);
  std::string tilePath = Tile::tilesCacheDirectory + "/" + tileKey + ".png";

  std::vector<uint8_t> imageData;
  auto tileResolution = loadPngFile(imageData, tilePath, LCT_RGB);
  if (tileResolution.first == 0 || tileResolution.second == 0 || imageData.empty()) {
    std::cerr << "Error loading tile from cache: " << tilePath << std::endl;
    // Remove file if it exists as it was probably corrupted when fetching via bluetooth
    safeDeleteFile(tilePath.c_str());
    return nullptr;
  }

  Tile *tile = new Tile(x, y, z, imageData);
  tile->tileWidth = tileResolution.first;
  tile->tileHeight = tileResolution.second;

  return tile;
}

std::pair<double, double> Tile::convertLatLongToTileXY(double latitude, double longitude, uint8_t zoom) {
  const double latRad = degreesToRadians(latitude);
  const auto n = double(pow(2.0, zoom));

  const double x = ((longitude + 180.0) / 360.0) * n;
  const double y = ((1.0 - asinh(tan(latRad)) / M_PI) / 2.0) * n;

  return std::make_pair(std::fmod(x, n), std::fmod(y, n));
}




