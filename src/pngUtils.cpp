#include "pngUtils.h"

#include <iostream>

std::pair<uint16_t, uint16_t> parsePngData(std::vector<uint8_t> &outData, uint8_t *pngData, uint32_t pngDataLength) {
  unsigned width, height;
  outData.clear();
  unsigned error = lodepng::decode(outData, width, height, pngData, pngDataLength, LCT_RGB);
  if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

  return std::make_pair(width, height);
}

std::pair<uint16_t, uint16_t>
loadPngFile(std::vector<uint8_t> &outData, const char *filename, LodePNGColorType colortype) {
  unsigned width, height;
  outData.clear();
  unsigned error = lodepng::decode(outData, width, height, filename, colortype);
  if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

  return std::make_pair(width, height);
}
