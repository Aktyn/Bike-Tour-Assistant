#include "camera.h"
#include "utils.h"
#include "Debug.h"
#include "core/core.h"

#include <cstdlib>
#include <climits>
#include <ctime>
#include <pthread.h>

std::string libCameraStill(double latitude, double longitude);

void *takePhotoAsync(void *args);

Camera::Camera() : latitude(0), longitude(0), distancePerPhoto(0) {
  // noop
}

Camera::~Camera() = default;

void Camera::takePhoto() { // NOLINT(*-convert-member-functions-to-static)
  pthread_t photo_thread_id;
  pthread_create(&photo_thread_id, nullptr, takePhotoAsync, this);
}

void Camera::updateLocation(double in_latitude, double in_longitude) {
  if (this->distancePerPhoto <= 0) {
    return;
  }

  // Register initial location
  if (this->latitude == 0 && this->longitude == 0) {
    this->latitude = in_latitude;
    this->longitude = in_longitude;
    return;
  }

  double distance = distanceBetweenCoordinates(this->latitude, this->longitude, in_latitude, in_longitude);
  if (distance >= this->distancePerPhoto) {
    this->latitude = in_latitude;
    this->longitude = in_longitude;
    this->takePhoto();
  }
}

double Camera::getLatitude() const {
  return latitude;
}

double Camera::getLongitude() const {
  return longitude;
}

void Camera::setDistancePerPhoto(double in_distancePerPhoto) {
  Camera::distancePerPhoto = in_distancePerPhoto;
}

void *takePhotoAsync(void *args) {
  auto camera = static_cast<Camera *>(args);

  std::cout << "Taking photo" << std::endl;
  std::string file_path = libCameraStill(camera->getLatitude(), camera->getLongitude());
  if (file_path.empty()) {
    std::cout << "Error taking photo" << std::endl;
  }
  return nullptr;
}

std::string libCameraStill(const double latitude, const double longitude) {
  auto photos_path = pwd() + "/photos";

  if (safeCreateDirectory(photos_path.c_str()) != 0) {
    printf("Error creating directory for camera photos\n");
    return "";
  }

  time_t current_time;
  time(&current_time);

  char libcamera_command_input[PATH_MAX];
  snprintf(
      libcamera_command_input, PATH_MAX,
      "libcamera-still -n --immediate --autofocus-on-capture -o %s/%ld.jpg", photos_path.c_str(), current_time);
  libcamera_command_input[PATH_MAX - 1] = '\0'; // Ensure null termination
  char *libcamera_output = executeCommand(libcamera_command_input);
  if (libcamera_output == nullptr) {
    printf("Error executing `libcamera-still` command\n");
    return "";
  } else {
    free(libcamera_output);
    DEBUG("Photo taken at %ld\n", current_time);
  }

  auto resultFilePath = photos_path + "/" + std::to_string(current_time) + ".jpg";

  if (latitude != 0 && longitude != 0) {
    DEBUG("Adding GPS data to photo: %s\n", resultFilePath.c_str());
    auto exiftool_command_input = std::string("exiftool -GPSLatitude=") +
                                  std::to_string(latitude) + " -GPSLongitude=" +
                                  std::to_string(longitude) + " " +
                                  resultFilePath;
    char *exiftool_command_output = executeCommand(exiftool_command_input.c_str());
    if (exiftool_command_output == nullptr) {
      printf("Error executing `exiftool` command\n");
    } else {
      free(exiftool_command_output);
      DEBUG("GPS data added to photo: %s\n", resultFilePath.c_str());
    }

    //remove original photo files
    auto rm_command_input = std::string("rm ") + photos_path + "/*.jpg_original";
    char *rm_command_output = executeCommand(rm_command_input.c_str());
    if(rm_command_output == nullptr) {
      printf("Error executing `rm` command\n");
    } else {
      free(rm_command_output);
      DEBUG("Original photo files removed: %s\n", rm_command_input.c_str());
    }
  }

  return resultFilePath;
}