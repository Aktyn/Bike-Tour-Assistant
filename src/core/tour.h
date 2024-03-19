#ifndef BIKETOURASSISTANT_TOUR_H
#define BIKETOURASSISTANT_TOUR_H

#include <cstdint>
#include <iostream>
#include <vector>
#include <map>

class Tour {
public:
  struct PointOfInterest {
    double latitude;
    double longitude;
  };
  struct Point {
    uint16_t pointIndex;
    double latitude;
    double longitude;
  };
  struct ClusteredPoint {
    uint16_t pointIndex;
    double latitude;
    double longitude;
    double tileX;
    double tileY;
  };
  struct PointsCache {
    uint32_t centerTileX;
    uint32_t centerTileY;
    uint8_t tileRadius;
    uint8_t zoom;
    std::vector<ClusteredPoint> clusteredPoints;
  };

  Tour();

  ~Tour();

  void setZoom(uint8_t zoom);

  void clear();

  void clear(uint16_t expectedPointsCount);

  void pushPoint(uint16_t pointIndex, double latitude, double longitude);

  void resetPointsOfInterest(uint16_t pointsCount);

  void pushPointOfInterest(double latitude, double longitude);

  bool empty() const;

  std::vector<ClusteredPoint> &getNearbyPoints(double latitude, double longitude, uint8_t tileRadius);

private:
  uint8_t zoom;

  std::vector<Point> points;
  std::map<std::string, std::vector<ClusteredPoint>> pointClusters;
  PointsCache nearbyPointsCache;

  std::vector<PointOfInterest> pointsOfInterest;
public:
  const std::vector<PointOfInterest> &getPointsOfInterest() const;

private:

  void clusterPoint(Point point);
};


#endif //BIKETOURASSISTANT_TOUR_H
