#include "renderer.h"
#include "display/draw.h"
#include "utils.h"

#include <cmath>

extern "C"
{
#include "GUI_BMP.h"
#include "DEV_Config.h"
#include "LCD_2inch4.h"
#include "GUI_Paint.h"
}

void rotateAroundPivot(double x, double y, double pivotX, double pivotY, double rotation, double &outX, double &outY) {
  if (rotation == 0) {
    outX = x;
    outY = y;
    return;
  }

  auto relativeX = x - pivotX;
  auto relativeY = y - pivotY;
  auto targetAngle = atan2(relativeY, relativeX) + rotation;
  auto len = sqrt(pow(relativeX, 2) + pow(relativeY, 2));
  auto pxRot = cos(targetAngle) * len;
  auto pyRot = sin(targetAngle) * len;

  outX = pxRot + pivotX;
  outY = pyRot + pivotY;
}

bool centerAndTrimLineSegment(
    double startX, double startY, double endX, double endY,
    uint16_t areaWidth, uint16_t areaHeight,
    int16_t &outStartX, int16_t &outStartY, int16_t &outEndX, int16_t &outEndY
) {
  const double centerX = double(areaWidth) / 2.0;
  const double centerY = double(areaHeight) / 2.0;

  auto centeredStartX = centerX + startX;
  auto centeredStartY = centerY + startY;
  auto centeredEndX = centerX + endX;
  auto centeredEndY = centerY + endY;

  if ((centeredStartX < 0 && centeredEndX < 0) ||
      (centeredStartY < 0 && centeredEndY < 0) ||
      (centeredStartX >= areaWidth && centeredEndX >= areaWidth) ||
      (centeredStartY >= areaHeight && centeredEndY >= areaHeight)) {
    return true;
  }

  outStartX = int16_t(centeredStartX);
  outStartY = int16_t(centeredStartY);
  outEndX = int16_t(centeredEndX);
  outEndY = int16_t(centeredEndY);

  return false;
}

uint16_t *renderer::renderMap(
    const std::map<std::string, Tile *> &tiles,
    Tour &tour,
    const Location &location,
    const uint8_t mapZoom
) {
  if (tiles.empty() && tour.empty()) {
    return nullptr;
  }

  auto backgroundColor = RGB(38, 50, 56);
  auto currentLocationOutlineColor = RGB(0, 96, 100);
  auto tourLineColor = RGB(255, 167, 38);

  uint16_t *buffer = allocateImageBuffer(MAP_WIDTH, MAP_HEIGHT);
  Paint_NewImage(buffer, MAP_WIDTH, MAP_HEIGHT, 0, WHITE, 24);
  Paint_Clear(backgroundColor);


  uint16_t tileWidth = 256;
  uint16_t tileHeight = 256;

  const uint16_t centerX = MAP_WIDTH / 2;
  const uint16_t centerY = MAP_HEIGHT / 2;
  auto locationTileXY = Tile::convertLatLongToTileXY(location.latitude, location.longitude, mapZoom);
  auto locationTileX = std::get<0>(locationTileXY);
  auto locationTileY = std::get<1>(locationTileXY);
  double rotationRad = degreesToRadians(location.heading);

  if (!tiles.empty()) {
    auto referenceTile = tiles.begin();
    // Override with real tile size
    tileWidth = referenceTile->second->tileWidth;
    tileHeight = referenceTile->second->tileHeight;

    for (uint16_t y = 0; y < MAP_HEIGHT; y++) {
      for (uint16_t x = 0; x < MAP_WIDTH; x++) {
        uint16_t index = (MAP_HEIGHT - 1 - y) * MAP_WIDTH + (MAP_WIDTH - 1 - x);

        double pixelTileX = double(x - centerX) / double(tileWidth) + locationTileX;
        double pixelTileY = double(y - centerY) / double(tileHeight) + locationTileY;
        rotateAroundPivot(pixelTileX, pixelTileY, locationTileX, locationTileY,
                          -rotationRad, pixelTileX, pixelTileY);

        auto tileKey = Tile::getTileKey(uint32_t(pixelTileX), uint32_t(pixelTileY), mapZoom);
        if (tiles.find(tileKey) == tiles.end()) {
          continue;
        }

        Tile *tile = tiles.at(tileKey);
        if (tile == nullptr || !tile->isFullyLoaded() || tile->imageData.empty()) {
          continue;
        }

        double relativeTileX = pixelTileX - double(tile->x); // 0 to 1
        double relativeTileY = pixelTileY - double(tile->y); // 0 to 1

        ASSERT(relativeTileX >= 0 && relativeTileX <= 1, "Relative tile X out of bounds");
        ASSERT(relativeTileY >= 0 && relativeTileY <= 1, "Relative tile Y out of bounds");
        uint16_t tileImageX = uint16_t(relativeTileY * double(tile->tileWidth));
        uint16_t tileImageY = uint16_t(relativeTileX * double(tile->tileHeight));

#if USE_DEBUG
        if (tileImageX == 0 || tileImageY == 0 ||
            tileImageX == tile->tileWidth - 1 || tileImageY == tile->tileHeight - 1) {
          buffer[index] = GREEN;
          continue;
        }
#endif

        uint16_t tileIndex = tileImageX * tile->tileWidth + tileImageY;
        ASSERT(tileIndex * 3 + 2 < tile->imageData.size(), "Tile index out of bounds");

        buffer[index] = convertRgbColor(
            RGB(
                tile->imageData[tileIndex * 3 + 0],
                tile->imageData[tileIndex * 3 + 1],
                tile->imageData[tileIndex * 3 + 2]
            )
        );
      }
    }
  }

  const std::vector<Tour::ClusteredPoint> &points = tour.getNearbyPoints(
      location.latitude, location.longitude, 3
  );
  if (!points.empty()) {
    for (size_t i = 1; i < points.size(); i++) {
      auto &previous = points[i - 1];

      // Draw line between adjacent points (prevent from looping between two separate segments)
      if (previous.pointIndex + 1 != points[i].pointIndex) {
        continue;
      }

      auto startX = (previous.tileX - locationTileX) * double(tileWidth);
      auto startY = (previous.tileY - locationTileY) * double(tileHeight);
      auto endX = (points[i].tileX - locationTileX) * double(tileWidth);
      auto endY = (points[i].tileY - locationTileY) * double(tileHeight);

      rotateAroundPivot(startX, startY, 0, 0, rotationRad, startX, startY);
      rotateAroundPivot(endX, endY, 0, 0, rotationRad, endX, endY);

      int16_t lineStartX, lineStartY, lineEndX, lineEndY;
      bool outOfBounds = centerAndTrimLineSegment(
          startX, startY, endX, endY,
          MAP_WIDTH, MAP_HEIGHT,
          lineStartX, lineStartY, lineEndX, lineEndY
      );
      if (outOfBounds) {
        continue;
      }

      Paint_DrawLine(lineStartX, lineStartY, lineEndX, lineEndY,
                     tourLineColor, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
    }
  }

  // Draw current location dot
  Paint_DrawCircle(centerX, centerY, 6,
                   currentLocationOutlineColor, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  Paint_DrawCircle(centerX, centerY, 3,
                   BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  // TODO: draw outlined circle as area of uncertainty, based on GPS accuracy

  return buffer; // buffer must be freed
}

