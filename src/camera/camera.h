#ifndef __CAMERA_H
#define __CAMERA_H

#include <iostream>

/**
 * Taking photos every 3 minutes with the below command for a 24 hours
 * takes about 10gb of space on raspberry pi (2.3mb per photo)
 *
 * libcamera-still -n --timestamp --immediate --autofocus-on-capture
 */
std::string takePhoto(void);

#endif // __CAMERA_H