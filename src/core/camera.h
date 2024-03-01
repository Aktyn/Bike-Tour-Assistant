#ifndef BIKETOURASSISTANT_CAMERA_H
#define BIKETOURASSISTANT_CAMERA_H


class Camera {
public:
  Camera();

  ~Camera();

  void takePhoto();

  void updateLocation(double latitude, double longitude);

private:
  double latitude;
public:
  double getLatitude() const;

  double getLongitude() const;

private:
  double longitude;
  double distancePerPhoto;
public:
  void setDistancePerPhoto(double distancePerPhoto);
  // distance in meters determining when to take a photo
};


#endif //BIKETOURASSISTANT_CAMERA_H
