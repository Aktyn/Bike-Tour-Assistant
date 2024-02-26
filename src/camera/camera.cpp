#include "camera.h"
#include "utils.h"
#include "Debug.h"
#include "LCD_2inch4.h"
#include "display/draw.h"
#include "core/core.h"

#include <cstdlib>
#include <cstring>
#include <libgen.h>
#include <climits>
#include <ctime>

std::string takePhoto(void)
{
  char *pwd_output = executeCommand("pwd");
  if (pwd_output == NULL)
  {
    printf("Error executing `pwd` command\n");
    return "";
  }

  char directory[PATH_MAX];
  strncpy(directory, dirname(pwd_output), PATH_MAX);
  directory[PATH_MAX - 1] = '\0'; // Ensure null termination

  free(pwd_output);

  char photos_path[PATH_MAX];
  snprintf(photos_path, PATH_MAX, "%s/%s", directory, "photos");
  photos_path[PATH_MAX - 1] = '\0'; // Ensure null termination

  if (safeCreateDirectory(photos_path) != 0)
  {
    printf("Error creating directory for camera photos\n");
    return "";
  }

  time_t current_time;
  time(&current_time);

  char libcamera_command_input[PATH_MAX];
  snprintf(
      libcamera_command_input, PATH_MAX,
      "libcamera-still -n --immediate --autofocus-on-capture -o %s/%ld.jpg", photos_path, current_time);
  libcamera_command_input[PATH_MAX - 1] = '\0'; // Ensure null termination
  char *libcamera_output = executeCommand(libcamera_command_input);
  if (libcamera_output == NULL)
  {
    printf("Error executing `libcamera-still` command\n");
    return "";
  }
  else
  {
    free(libcamera_output);
    DEBUG("Photo taken at %ld\n", current_time);
  }

  return std::string(photos_path) + "/" + std::to_string(current_time) + ".jpg";
}

void *takePhotoAsync(void *args) {
  std::cout << "Taking photo" << std::endl;
  std::string file_path = takePhoto();
  if (file_path.empty()) {
    std::cout << "Error taking photo" << std::endl;
    return nullptr;
  }

  if (CORE.isBluetoothConnected) {
    drawImageFromJpgFile(file_path.c_str(), 0, 0, LCD_2IN4_WIDTH);
  }

  return nullptr;
}