#include "renderer.h"
#include "display/draw.h"
#include "utils.h"

#include <cmath>

extern "C"
{
#include "GUI_BMP.h"
}

#define BACKGROUND_RED 34
#define BACKGROUND_GREEN 46
#define BACKGROUND_BLUE 52
static auto backgroundColor = RGB(BACKGROUND_RED, BACKGROUND_GREEN, BACKGROUND_BLUE);

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

void renderer::prepareMainView() {
  clearScreen(backgroundColor);
  drawTextLine("Waiting for map data",
               0, LCD_2IN4_HEIGHT * 3 / 4 - Font16.Height / 2, LCD_2IN4_WIDTH,
               WHITE, backgroundColor, &Font16, ALIGN_CENTER);
}

void renderer::renderMap(
    const std::map<std::string, Tile *> &tiles,
    Tour &tour,
    const Location &location,
    const uint8_t mapZoom
) {
  if (tiles.empty() && tour.empty()) {
    return;
  }

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
                          rotationRad, pixelTileX, pixelTileY);

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

      rotateAroundPivot(startX, startY, 0, 0, -rotationRad, startX, startY);
      rotateAroundPivot(endX, endY, 0, 0, -rotationRad, endX, endY);

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

  // Draw outlined circle as area of uncertainty, based on GPS accuracy (TODO: add option for hiding it)
  //Spixel = C ∙ cos(latitude) / 2 (zoomlevel + 8); // https://wiki.openstreetmap.org/wiki/Zoom_levels
  const double C = 40075016.686;
  double metersPerPixel = (C * cos(degreesToRadians(location.latitude)) / pow(2, mapZoom)) / double(tileWidth);
  auto accuracyPixelsRadius = uint16_t(location.accuracy / metersPerPixel);
//  DEBUG("metersPerPixel: %f; map zoom: %u, accuracy pixel radius: %u\n", metersPerPixel, mapZoom, accuracyPixelsRadius);
  uint16_t radius = std::min(accuracyPixelsRadius, uint16_t(MAP_WIDTH / 2));
  if (radius > 1) {
    Paint_DrawCircle(centerX, centerY, radius,
                     CYAN, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
  }

  drawImageBuffer(buffer, 0, LCD_2IN4_HEIGHT - MAP_HEIGHT, MAP_WIDTH, MAP_HEIGHT);
  free(buffer);
}

void renderer::drawSpeed(double speed) {
  std::string speedText = std::to_string((uint16_t) std::round(speed));

  //TODO: draw digits from bitmap files (cache loaded buffers)
  drawTextLine(speedText.c_str(),
               TOP_PANEL_HEIGHT, (LCD_2IN4_HEIGHT - MAP_HEIGHT) / 2 - Font50.Height / 2,
               LCD_2IN4_WIDTH - TOP_PANEL_HEIGHT * 2,
               WHITE, backgroundColor, &Font50, ALIGN_CENTER);
}

void renderer::drawBattery(uint8_t percentage, bool isOverheated) {
  const uint16_t imageWidth = TOP_PANEL_HEIGHT;
  const uint16_t imageHeight = TOP_PANEL_HEIGHT;
  const uint16_t widgetWidth = imageWidth / 3;
  const uint16_t widgetHeight = widgetWidth / 2;
  const uint16_t widgetHeadWidth = 4;
  const uint16_t gapY = 4;

  uint16_t *imageBuffer = allocateImageBuffer(imageWidth, imageHeight);
  if (imageBuffer == nullptr) {
    return;
  }

  uint16_t xStart = (imageWidth - widgetWidth) / 2;
  uint16_t yStart = imageHeight / 2 - widgetHeight - gapY / 2;
  double factor = double(percentage) / 100.0;
  auto batteryColor = RGB(
      uint8_t(mix(229, 165, factor)),
      uint8_t(mix(115, 214, factor)),
      uint8_t(mix(115, 167, factor))
  );
  auto errorColor = RGB(229, 115, 115);

  Paint_NewImage(imageBuffer, imageWidth, imageHeight, 0, WHITE, 16);
  Paint_Clear(backgroundColor);

  Paint_DrawRectangle(xStart, yStart, xStart + widgetWidth * uint16_t(percentage) / 100, yStart + widgetHeight,
                      batteryColor, DOT_PIXEL_2X2, DRAW_FILL_FULL);
  Paint_DrawRectangle(xStart, yStart, xStart + widgetWidth, yStart + widgetHeight,
                      batteryColor, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
  Paint_DrawRectangle(xStart + widgetWidth, yStart + widgetHeight / 2 - widgetHeight / 4,
                      xStart + widgetWidth + widgetHeadWidth, yStart + widgetHeight / 2 + widgetHeight / 4,
                      batteryColor, DOT_PIXEL_2X2, DRAW_FILL_FULL);

  std::string batteryPercentageText = std::to_string(percentage) + "%";
  uint16_t aligned_x = (imageWidth - Font16.Width * batteryPercentageText.length()) / 2;
  Paint_DrawString_EN(aligned_x, imageHeight / 2 + gapY / 2, batteryPercentageText.c_str(), &Font16,
                      backgroundColor, batteryColor);

  if (isOverheated) {
    Paint_DrawString_EN(0, 0, "TOO HOT", &Font16,
                        backgroundColor, errorColor);
  }

  drawImageBuffer(imageBuffer, 0, 0, imageWidth, imageHeight);
  free(imageBuffer);
  imageBuffer = nullptr;
}

void renderer::drawDirectionArrow(double heading, const Icons &icons) {
  const uint16_t imageWidth = icons.directionArrowSize.first;
  const uint16_t imageHeight = icons.directionArrowSize.second;
  const auto imageData = icons.directionArrowImageData;

  uint16_t *imageBuffer = allocateImageBuffer(imageWidth, imageHeight);
  if (imageBuffer == nullptr) {
    return;
  }

  auto rotationRad = degreesToRadians(heading);

  for (uint16_t y = 0; y < imageHeight; y++) {
    for (uint16_t x = 0; x < imageWidth; x++) {
      double rotatedX, rotatedY;
      rotateAroundPivot(x, y, imageWidth / 2.0, imageHeight / 2.0, rotationRad,
                        rotatedX, rotatedY);

      uint16_t index = (imageHeight - 1 - y) * imageWidth + (imageWidth - 1 - x);
      if (rotatedX < 0 || rotatedX >= imageWidth || rotatedY < 0 || rotatedY >= imageHeight) {
        imageBuffer[index] = convertRgbColor(backgroundColor);
        continue;
      }

      uint16_t rotatedIndex = uint16_t(rotatedY) * imageWidth + uint16_t(rotatedX);
      uint16_t pixelIndex = rotatedIndex * 4;
      auto alphaFactor = double(imageData[pixelIndex + 3]) / 255.0;
      imageBuffer[index] = convertRgbColor(
          RGB(
              uint8_t(mix(BACKGROUND_RED, imageData[pixelIndex + 0], alphaFactor)),
              uint8_t(mix(BACKGROUND_GREEN, imageData[pixelIndex + 1], alphaFactor)),
              uint8_t(mix(BACKGROUND_BLUE, imageData[pixelIndex + 2], alphaFactor))
          )
      );
    }
  }

  drawImageBuffer(imageBuffer, LCD_2IN4_WIDTH - imageWidth, 0, imageWidth, imageHeight);
  free(imageBuffer);
  imageBuffer = nullptr;
}

void renderer::drawSlope(double slope, const Icons &icons) {
  const uint16_t imageWidth = TOP_PANEL_HEIGHT;
  const uint16_t imageHeight = TOP_PANEL_HEIGHT / 2;

  uint16_t *imageBuffer = allocateImageBuffer(imageWidth, imageHeight);
  if (imageBuffer == nullptr) {
    return;
  }

  Paint_NewImage(imageBuffer, imageWidth, imageHeight, 0, WHITE, 24);
  Paint_Clear(backgroundColor);

  const auto imageData = slope >= 0 ? icons.slopeUphillImageData : icons.slopeDownhillImageData;
  const auto textColor = slope >= 0 ? RGB(255, 235, 238) : RGB(232, 245, 233);

  uint8_t iconColorRed = 176;
  uint8_t iconColorGreen = 190;
  uint8_t iconColorBlue = 197;

  for (uint16_t y = 0; y < icons.slopeIconSize.second; y++) {
    for (uint16_t x = 0; x < icons.slopeIconSize.first; x++) {
      uint16_t index =
          ((imageHeight - icons.slopeIconSize.second) / 2 + (icons.slopeIconSize.second - 1 - y)) * imageWidth +
          (icons.slopeIconSize.first - 1 - x);
      uint16_t pixelIndex = y * icons.slopeIconSize.second + x;

      auto alphaFactor = double(imageData[pixelIndex * 4 + 3]) / 255.0;
      auto r = uint8_t(double(imageData[pixelIndex * 4 + 0]) / 255.0 * double(iconColorRed));
      auto g = uint8_t(double(imageData[pixelIndex * 4 + 1]) / 255.0 * double(iconColorGreen));
      auto b = uint8_t(double(imageData[pixelIndex * 4 + 2]) / 255.0 * double(iconColorBlue));
      imageBuffer[index] = convertRgbColor(
          RGB(
              uint8_t(mix(BACKGROUND_RED, r, alphaFactor)),
              uint8_t(mix(BACKGROUND_GREEN, g, alphaFactor)),
              uint8_t(mix(BACKGROUND_BLUE, b, alphaFactor))
          )
      );
    }
  }

  std::string slopeText = std::to_string((uint16_t) round(radiansToDegrees(slope))) + "deg";
  auto aligned_x = MAX(0, int16_t(imageWidth) -icons.slopeIconSize.first - int16_t(Font16.Width) * slopeText.length());
  Paint_DrawString_EN(uint16_t(aligned_x), (imageHeight - Font16.Height) / 2, slopeText.c_str(), &Font16,
                      backgroundColor, textColor);

  drawImageBuffer(imageBuffer, LCD_2IN4_WIDTH - imageWidth, TOP_PANEL_HEIGHT / 2, imageWidth, imageHeight);
  free(imageBuffer);
  imageBuffer = nullptr;
}
