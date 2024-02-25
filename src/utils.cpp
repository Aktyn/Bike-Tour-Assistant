#include "utils.h"
#include "Debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <limits.h>
#include <sys/stat.h> // Include this header for R_OK
#include <cmath>

#define MKDIR_COMMAND_MAX (PATH_MAX + 6)

void extendBuffer(char **buffer, uint32_t new_size) {
  char *new_buffer = (char *) realloc(*buffer, new_size * sizeof(char));
  if (new_buffer == nullptr) {
    fprintf(stderr, "Failed to reallocate memory.\n");
    free(*buffer);
    return;
  }
  *buffer = new_buffer;
}

char *executeCommand(const char *command) {
  DEBUG("Executing command: %s\n", command);

  FILE *pipe = popen(command, "r");
  if (!pipe) {
    perror("popen");
    return nullptr;
  }

  char *cmd_output = (char *) malloc(0 * sizeof(char));
  uint32_t cmd_output_size = 0;

  char buffer[128];
  while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    if (cmd_output_size == 0) {
      cmd_output_size += strlen(buffer) + 1;
      free(cmd_output);
      cmd_output = (char *) malloc(cmd_output_size * sizeof(char));
      if (cmd_output == nullptr) {
        fprintf(stderr, "Failed to allocate memory.\n");
        return nullptr;
      }
      strcpy(cmd_output, buffer);
      cmd_output[cmd_output_size - 1] = '\0'; // Ensure null termination
    } else {
      cmd_output_size += strlen(buffer) + 1;
      extendBuffer(&cmd_output, cmd_output_size);
      strcat(cmd_output, buffer);
      cmd_output[cmd_output_size - 1] = '\0'; // Ensure null termination
    }
  }

  int exit_status = pclose(pipe);
  if (WEXITSTATUS(exit_status) != 0) {
    fprintf(stderr, "Command exited with status: %d\n", WEXITSTATUS(exit_status));
    return nullptr;
  }

  return cmd_output;
}

int safeCreateDirectory(const char *path) {
  if (access(path, R_OK) != 0) {
    DEBUG("Creating directory: %s\n", path);

    char mkdir_command_input[MKDIR_COMMAND_MAX];
    snprintf(mkdir_command_input, MKDIR_COMMAND_MAX, "mkdir %s", path);
    mkdir_command_input[MKDIR_COMMAND_MAX - 1] = '\0'; // Ensure null termination
    char *mkdir_command = executeCommand(mkdir_command_input);
    if (mkdir_command == nullptr) {
      printf("Error creating directory: %s\n", path);
      return 1;
    } else {
      free(mkdir_command);
      DEBUG("Directory created: %s\n", path);
    }
  }

  return 0;
}

uint16_t rgbToRgb666(uint8_t red, uint8_t green, uint8_t blue) {
  uint16_t red_666 = (red >> 2) & 0x3F;
  uint16_t green_666 = ((red & 0x03) << 4) | ((green >> 2) & 0x3F);
  uint16_t blue_666 = (blue >> 3) & 0x1F;

  return (red_666 << 12) | (green_666 << 6) | blue_666;
}

uint16_t rgbToRgb565(uint8_t red, uint8_t green, uint8_t blue) {
  uint16_t r = (blue >> 3) & 0x1F;
  uint16_t g = ((blue & 0x07) << 5) | ((red >> 3) & 0x3F);
  uint16_t b = (green >> 3) & 0x1F;

  return (r << 11) | (g << 5) | b;
}

uint16_t rgbToRgb444(uint8_t red, uint8_t green, uint8_t blue) {
  uint16_t red_444 = (red >> 4) & 0x0F;
  uint16_t green_444 = (green >> 4) & 0x0F;
  uint16_t blue_444 = (blue >> 4) & 0x0F;

  return (red_444 << 8) | (green_444 << 4) | blue_444;
}

uint16_t convertRgbColor(uint16_t color) {
  return ((color << 8) & 0xff00) | (color >> 8);
}

uint16_t findNextPowerOf2(uint16_t n) {
  if (n && !(n & (n - 1)))
    return n;

  uint16_t count = 0;

  while (n != 0) {
    n >>= 1;
    count += 1;
  }

  return 1 << count;
}

float bytesToFloat(const uint8_t *bytes, bool big_endian) {
  float f;
  auto *f_ptr = (uint8_t *) &f;
  if (big_endian) {
    f_ptr[3] = bytes[0];
    f_ptr[2] = bytes[1];
    f_ptr[1] = bytes[2];
    f_ptr[0] = bytes[3];
  } else {
    f_ptr[3] = bytes[3];
    f_ptr[2] = bytes[2];
    f_ptr[1] = bytes[1];
    f_ptr[0] = bytes[0];
  }
  return f;
}

double bytesToDouble(const uint8_t *bytes, bool big_endian) {
  double d;
  auto *d_ptr = (uint8_t *) &d;
  if (big_endian) {
    d_ptr[7] = bytes[0];
    d_ptr[6] = bytes[1];
    d_ptr[5] = bytes[2];
    d_ptr[4] = bytes[3];
    d_ptr[3] = bytes[4];
    d_ptr[2] = bytes[5];
    d_ptr[1] = bytes[6];
    d_ptr[0] = bytes[7];
  } else {
    d_ptr[7] = bytes[7];
    d_ptr[6] = bytes[6];
    d_ptr[5] = bytes[5];
    d_ptr[4] = bytes[4];
    d_ptr[3] = bytes[3];
    d_ptr[2] = bytes[2];
    d_ptr[1] = bytes[1];
    d_ptr[0] = bytes[0];
  }
  return d;
}

uint16_t bytesToUint16(const uint8_t *bytes, bool big_endian) {
  uint16_t i;
  auto *i_ptr = (uint8_t *) &i;
  if (big_endian) {
    i_ptr[1] = bytes[0];
    i_ptr[0] = bytes[1];
  } else {
    i_ptr[1] = bytes[1];
    i_ptr[0] = bytes[0];
  }
  return i;
}

uint32_t bytesToUint32(const uint8_t *bytes, bool big_endian) {
  uint32_t i;
  auto *i_ptr = (uint8_t *) &i;
  if (big_endian) {
    i_ptr[3] = bytes[0];
    i_ptr[2] = bytes[1];
    i_ptr[1] = bytes[2];
    i_ptr[0] = bytes[3];
  } else {
    i_ptr[3] = bytes[3];
    i_ptr[2] = bytes[2];
    i_ptr[1] = bytes[1];
    i_ptr[0] = bytes[0];
  }
  return i;
}

uint64_t bytesToUint64(const uint8_t *bytes, bool big_endian) {
  uint64_t i;
  auto *i_ptr = (uint8_t *) &i;
  if (big_endian) {
    i_ptr[7] = bytes[0];
    i_ptr[6] = bytes[1];
    i_ptr[5] = bytes[2];
    i_ptr[4] = bytes[3];
    i_ptr[3] = bytes[4];
    i_ptr[2] = bytes[5];
    i_ptr[1] = bytes[6];
    i_ptr[0] = bytes[7];
  } else {
    i_ptr[7] = bytes[7];
    i_ptr[6] = bytes[6];
    i_ptr[5] = bytes[5];
    i_ptr[4] = bytes[4];
    i_ptr[3] = bytes[3];
    i_ptr[2] = bytes[2];
    i_ptr[1] = bytes[1];
    i_ptr[0] = bytes[0];
  }
  return i;
}

double metersPerSecondToKmPerHour(double metersPerSecond) {
  return metersPerSecond * 3.6;
}

double distanceBetweenCoordinates(double lat1, double lon1, double lat2, double lon2) {
  const double R = 6371e3; // metres
  const double phi1 = degreesToRadians(lat1); // φ, λ in radians
  const double phi2 = degreesToRadians(lat2);
  const double deltaPhi = degreesToRadians(lat2 - lat1);
  const double deltaLambda = degreesToRadians(lon2 - lon1);

  const double a = sin(deltaPhi / 2) * sin(deltaPhi / 2) +
                   cos(phi1) * cos(phi2) *
                   sin(deltaLambda / 2) * sin(deltaLambda / 2);
  const double c = 2 * atan2(sqrt(a), sqrt(1 - a));

  return R * c; // in metres
}

double degreesToRadians(double degrees) {
  return (degrees * M_PI) / 180.0;
}

double radiansToDegrees(double radians) {
  return (radians * 180.0) / M_PI;

}

double mix(double a, double b, double mix) {
  return a * (1.0 - mix) + b * mix;
}

template<typename T>
T calculateLinearlyWeightedAverage(const std::vector<T> &values) {
  static_assert(std::is_arithmetic<T>::value, "Type must be numeric");

  T sum = 0;
  size_t sumOfWeights = 0;

  for (size_t i = 0; i < values.size(); i++) {
    sum += values[i] * (i + 1);
    sumOfWeights += i + 1;
  }

  return sum / T(sumOfWeights);
}

template double calculateLinearlyWeightedAverage<double>(const std::vector<double> &values);