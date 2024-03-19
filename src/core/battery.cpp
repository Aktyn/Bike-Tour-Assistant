#include "battery.h"
#include "utils.h"
#include "Debug.h"
#include <cstdlib>

#define DANGEROUS_TEMPERATURE 90 // Celsius
#define BATTERY_MEASURE_INTERVAL 30000 // 30 seconds

Battery::Battery() : needRedraw(true), percentage(100), temperature(0) {
  // noop
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
  if (delta.count() < BATTERY_MEASURE_INTERVAL) {
    return;
  }
  this->lastUpdateTime = now;

  try {
    std::string batteryCommand = std::string("python3 ") + pwd() + "/../measure_battery.py";
    char *battery_output = executeCommand(batteryCommand.c_str());

    auto percentageValue = uint8_t(atoi(battery_output));
    if (percentageValue != this->percentage) {
      this->needRedraw = true;
      this->percentage = percentageValue;
    }
    free(battery_output);

    char *temperature_output = executeCommand("vcgencmd measure_temp"); //example temperature_output: temp=45.6'C
    this->temperature = uint16_t(atof(temperature_output + 5));
    DEBUG("Device temperature: %u Celsius; Battery percentage: %u%%\n", this->temperature, this->percentage);
    free(temperature_output);

    if (this->temperature > DANGEROUS_TEMPERATURE) {
      DEBUG("Device temperature is too high!!! (%u)\n", this->temperature);
      this->needRedraw = true;
    }
  }
  catch (std::exception &e) {
    DEBUG("Error measuring temperature: %s\n", e.what());
  }
}
