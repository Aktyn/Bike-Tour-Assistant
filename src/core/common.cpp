#include "common.h"
#include "utils.h"

#include <cmath>

double calculateSlope(const std::vector<Location> &locationHistory) {
  if (locationHistory.size() < 2) {
    return 0;
  }

  std::vector<double> measurements;
  measurements.reserve(locationHistory.size() - 1);
  for (size_t i = 1; i < locationHistory.size(); i++) {
    double altitudeDifference = locationHistory[i].altitude - locationHistory[i - 1].altitude;
    double distance = distanceBetweenCoordinates(locationHistory[i].latitude, locationHistory[i].longitude,
                                        locationHistory[i - 1].latitude, locationHistory[i - 1].longitude);
    double slope = atan(altitudeDifference / distance);
    measurements.push_back(slope == slope ? slope : 0.0);
  }

  return calculateLinearlyWeightedAverage(measurements);
}
