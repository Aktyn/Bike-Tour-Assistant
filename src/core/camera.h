#ifndef BIKETOURASSISTANT_CAMERA_H
#define BIKETOURASSISTANT_CAMERA_H


class Camera {
public:
  Camera();

  ~Camera();

  double getLatitude() const;

  double getLongitude() const;

  void takePhoto();

  void updateLocation(double latitude, double longitude);

  void setDistancePerPhoto(double distancePerPhoto);

private:
  double latitude;
  double longitude;
  double distancePerPhoto; // distance in meters determining when to take a photo
};


#endif //BIKETOURASSISTANT_CAMERA_H
