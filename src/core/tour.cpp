#include "tour.h"
#include "tile.h"
#include "Debug.h"

#include <tuple>
#include <algorithm>

Tour::Tour() :
    zoom(0),
    nearbyPointsCache({0, 0, 0, 0, std::vector<ClusteredPoint>()}) {
  // noop
}

Tour::~Tour() {
  this->clear();
}

void Tour::setZoom(uint8_t value) {
  this->zoom = value;

  this->pointClusters.clear();
  for (const auto &point: this->points) {
    this->clusterPoint(point);
  }
}

void Tour::clear() {
  this->points.clear();
  this->pointClusters.clear();
  this->nearbyPointsCache.clusteredPoints.clear();
  this->nearbyPointsCache.tileRadius = 0; // invalidate cache
}

void Tour::clear(uint16_t expectedPointsCount) {
  this->clear();
  this->points.reserve(expectedPointsCount);
}

void Tour::pushPoint(uint16_t pointIndex, double latitude, double longitude) {
  Point point = {pointIndex, latitude, longitude};
  this->points.push_back(point);
  this->clusterPoint(point);
}

bool Tour::empty() const {
  return this->points.empty();
}

void Tour::clusterPoint(Tour::Point point) {
  if (this->zoom != 0) {
    auto tileXY = Tile::convertLatLongToTileXY(point.latitude, point.longitude, this->zoom);
    auto tileKey = Tile::getTileKey(uint32_t(std::get<0>(tileXY)), uint32_t(std::get<1>(tileXY)), this->zoom);

    if (this->pointClusters.find(tileKey) == this->pointClusters.end()) {
      this->pointClusters[tileKey] = std::vector<ClusteredPoint>();
    }
    this->pointClusters[tileKey].push_back({point.pointIndex,
                                            point.latitude, point.longitude,
                                            std::get<0>(tileXY), std::get<1>(tileXY)
                                           });

    this->nearbyPointsCache.tileRadius = 0; // invalidate cache
  }
}

std::vector<Tour::ClusteredPoint> &Tour::getNearbyPoints(double latitude, double longitude, uint8_t tileRadius) {
  auto centerTileXY = Tile::convertLatLongToTileXY(latitude, longitude, this->zoom);

  if (this->nearbyPointsCache.centerTileX == uint32_t(std::get<0>(centerTileXY)) &&
      this->nearbyPointsCache.centerTileY == uint32_t(std::get<1>(centerTileXY)) &&
      this->nearbyPointsCache.tileRadius == tileRadius &&
      this->nearbyPointsCache.zoom == this->zoom) {
    return this->nearbyPointsCache.clusteredPoints;
  }

  DEBUG("Recalculating nearby points cache. Total tour length: %zu\n", this->points.size());

  this->nearbyPointsCache.centerTileX = uint32_t(std::get<0>(centerTileXY));
  this->nearbyPointsCache.centerTileY = uint32_t(std::get<1>(centerTileXY));
  this->nearbyPointsCache.tileRadius = tileRadius;
  this->nearbyPointsCache.zoom = this->zoom;
  this->nearbyPointsCache.clusteredPoints.clear();

  for (int x = -tileRadius; x <= tileRadius; x++) {
    for (int y = -tileRadius; y <= tileRadius; y++) {
      auto tileKey = Tile::getTileKey(
          this->nearbyPointsCache.centerTileX + x, this->nearbyPointsCache.centerTileY + y,
          this->zoom);
      if (this->pointClusters.find(tileKey) != this->pointClusters.end()) {
        std::vector<Tour::ClusteredPoint> &tilePoints = this->pointClusters[tileKey];
        for (auto &point: tilePoints) {
          this->nearbyPointsCache.clusteredPoints.push_back(point);
        }
      }
    }
  }

  std::sort(
      this->nearbyPointsCache.clusteredPoints.begin(),
      this->nearbyPointsCache.clusteredPoints.end(),
      [](const Tour::ClusteredPoint &a, const Tour::ClusteredPoint &b) {
        return a.pointIndex < b.pointIndex;
      }
  );
  return this->nearbyPointsCache.clusteredPoints;
}


