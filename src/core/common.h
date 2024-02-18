#ifndef BIKETOURASSISTANT_CORE_COMMON_H
#define BIKETOURASSISTANT_CORE_COMMON_H

#include <cstdint>

struct Location {
  float latitude;
  float longitude;
  float speed;
  float heading;
  uint64_t timestamp;
  uint64_t previousUpdateTimestamp;
};

#endif //BIKETOURASSISTANT_CORE_COMMON_H
