#ifndef BIKETOURASSISTANT_PNGUTILS_H
#define BIKETOURASSISTANT_PNGUTILS_H

#include <cstdint>
#include <iostream>
#include <vector>

std::pair<uint16_t, uint16_t> parsePngData(std::vector<uint8_t> &outData, uint8_t *pngData, uint32_t pngDataLength);

#endif //BIKETOURASSISTANT_PNGUTILS_H
