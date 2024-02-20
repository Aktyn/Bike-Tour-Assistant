#include "pngUtils.h"
#include "lodepng/lodepng.h"

#include <iostream>

void parsePngData() {
  //TODO: use std::vector<unsigned char> png; instead of filename
  std::vector<unsigned char> image; //the raw pixels
  unsigned width, height;

  //decode
  unsigned error = lodepng::decode(image, width, height, "../assets/test_tile.png", LCT_RGB);

  //if there's an error, display it
  if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

  //the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...
  std::cout << "Image size: " << image.size() << std::endl;
}