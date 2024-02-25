#include "battery.h"
#include "utils.h"
#include "Debug.h"
#include <cstdlib>

#define DANGEROUS_TEMPERATURE 90

Battery::Battery() : needRedraw(true), percentage(100), temperature(0) {
  //
}


uint8_t Battery::getPercentage() const {
  return this->percentage;
}

bool Battery::isOverheated() const {
  return this->temperature > DANGEROUS_TEMPERATURE;
}

void Battery::update() {
  auto now = std::chrono::system_clock::now();
  auto delta = std::chrono::duration_cast<milliseconds>(now - this->lastUpdateTime);
  if (delta.count() < 5000) {
    return;
  }
  this->lastUpdateTime = now;

  try {
    // TODO: monitor battery level and combine debug message
    // this->needRedraw = true;

    char *output = executeCommand("vcgencmd measure_temp"); //example output: temp=45.6'C
    this->temperature = uint16_t(atof(output + 5));
    DEBUG("Device temperature: %u Celsius\n", this->temperature);
    free(output);

    if (this->temperature > DANGEROUS_TEMPERATURE) {
      DEBUG("Device temperature is too high!!! (%u)\n", this->temperature);
      this->needRedraw = true;
    }
  }
  catch (std::exception &e) {
    DEBUG("Error measuring temperature: %s\n", e.what());
  }
}
