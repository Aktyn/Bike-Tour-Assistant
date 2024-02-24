#ifndef BIKETOURASSISTANT_PNGUTILS_H
#define BIKETOURASSISTANT_PNGUTILS_H

#include <cstdint>
#include <iostream>
#include <vector>
#include "lodepng/lodepng.h"

std::pair<uint16_t, uint16_t> parsePngData(std::vector<uint8_t> &outData, uint8_t *pngData, uint32_t pngDataLength);
std::pair<uint16_t, uint16_t> loadPngFile(std::vector<uint8_t> &outData, const char *filename, LodePNGColorType colortype = LCT_RGB);

#endif //BIKETOURASSISTANT_PNGUTILS_H
