#include "tour.h"

Tour::Tour() {
  this->zoom = 0;
  this->expectedPointsCount = 0;
}

void Tour::setZoom(uint8_t value) {
  this->zoom = value;

  //TODO: regenerate point clusters based on new zoom level
}

void Tour::clear() {
  this->expectedPointsCount = 0;
  //TODO: clear arrays with points and clusters
}

void Tour::clear(uint16_t _expectedPointsCount) {
  this->clear();
  this->expectedPointsCount = _expectedPointsCount;
}

void Tour::pushPoint(uint16_t pointIndex, float latitude, float longitude) {
  //TODO: push point to array and cluster it based on zoom level (do not cluster if zoom is 0; later create logic for default zoom level)
}
