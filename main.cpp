#include "bluetooth/bluetooth_server.h"
#include "display/intro_view.h"
#include "core/core.h"
#include "core/renderer.h"
#include "camera/camera.h"
#include "utils.h"
#include "display/draw.h"

#include <cstdlib>
#include <csignal> //signal()
#include <pthread.h>
#include <iostream>
#include <chrono>

void *bluetoothThread(void (*onMessage)(unsigned char *data)) {
  startBluetoothServer(onMessage);
  pthread_exit(nullptr);
  return nullptr;
}

void *displayThread(void *args) {
  while (CORE.isRunning) {
    if (!CORE.isBluetoothConnected) {
      showIntroView(); // This function includes while loop breaking on bluetooth connection
    }

    renderer::prepareMainView();
    CORE.reset();

    while (CORE.isBluetoothConnected) {
      CORE.update();

      if (CORE.needMapRedraw) {
        CORE.needMapRedraw = false;

        auto startTime = std::chrono::high_resolution_clock::now();

        CORE.drawMap();

        auto endTime = std::chrono::high_resolution_clock::now();
        auto executionDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        DEBUG("Map render took %lld milliseconds\n", executionDuration.count());
      }

      if (CORE.needSpeedRedraw) {
        CORE.needSpeedRedraw = false;
        renderer::drawSpeed(CORE.location.speed);
      }

      if (CORE.needDirectionRedraw) {
        CORE.needDirectionRedraw = false;
        renderer::drawDirectionArrow(CORE.location.heading, CORE.getIcons());
      }

      if (CORE.needSlopeRedraw) {
        CORE.needSlopeRedraw = false;
        renderer::drawSlope(CORE.getSlope(), CORE.getIcons());
      }

      if (CORE.battery.needRedraw) {
        CORE.battery.needRedraw = false;
        renderer::drawBattery(CORE.battery.getPercentage(), CORE.battery.isOverheated());
      }

      // sleep for 16ms
      usleep(16 * 1000);
    }
  }
  return nullptr;
}

void *takePhotoAsync(void *args) {
  std::cout << "Taking photo" << std::endl;
  std::string file_path = takePhoto();
  if (file_path.empty()) {
    std::cout << "Error taking photo" << std::endl;
    return nullptr;
  }

  if (CORE.isBluetoothConnected) {
    drawImageFromJpgFile(file_path.c_str(), 0, 0, LCD_2IN4_WIDTH);
  }

  return nullptr;
}

void terminate(int signo) {
  Handler_2IN4_LCD(signo);
}

void handleMessage(unsigned char *data) {
  if (!CORE.isBluetoothConnected) {
    return;
  }

  switch (data[0]) {
    case 1: // PING
      DEBUG("Ping\n");
      if (CORE.isBluetoothConnected) {
        // TODO: send pong
      }
      break;
    case 2: { //SET_LIGHTNESS
      std::cout << "Setting backlight to: " << std::to_string((uint8_t) data[1]) << "%" << std::endl;
      uint8_t lightness = data[1];
      CORE.setBacklight(lightness);
    }
      break;
    case 3: //TAKE_PHOTO
    {
      pthread_t photo_thread_id;
      pthread_create(&photo_thread_id, nullptr, takePhotoAsync, nullptr);
    }
      break;
    case 4: // LOCATION_UPDATE
    {
      double latitude = bytesToDouble(data + 1, false);
      double longitude = bytesToDouble(data + 9, false);
      double speed = bytesToDouble(data + 17, false);
      double heading = bytesToDouble(data + 25, false);
      double altitude = bytesToDouble(data + 33, false);
      double altitudeAccuracy = bytesToDouble(data + 41, false);
      double accuracy = bytesToDouble(data + 49, false);
      uint64_t timestamp = bytesToUint64(data + 57, false);
      DEBUG("Location: %f, %f, %f, %f, %f, %f, %llu\n",
            latitude, longitude, speed, heading, altitude, accuracy, timestamp);
      CORE.updateLocation(latitude, longitude, speed, heading, altitude, altitudeAccuracy, accuracy, timestamp);
    }
      break;
    case 5: // SEND_MAP_TILE_START
    {
      uint32_t x = bytesToUint32(data + 1, false);
      uint32_t y = bytesToUint32(data + 5, false);
      uint32_t z = bytesToUint32(data + 9, false);
      uint32_t dataByteLength = bytesToUint32(data + 13, false);
      DEBUG("Map tile start: %u, %u, %u, %u\n",
            x, y, z, dataByteLength);
      CORE.registerTile(x, y, z, dataByteLength);
    }
      break;
    case 6: // SEND_MAP_TILE_DATA_CHUNK
    {
      uint16_t chunkIndex = bytesToUint16(data + 1, false);
      // DEBUG("Map tile data chunk %d\n", chunkIndex);
      CORE.appendTileImageData(chunkIndex, data + 3);
    }
      break;
    case 7: // CLEAR_TOUR_DATA
    {
      DEBUG("Clearing tour data\n");
      CORE.tour.clear();
    }
      break;
    case 8: // SEND_TOUR_START
    {
      uint16_t pointsCount = bytesToUint16(data + 1, false);
      DEBUG("Receiving tour data with %u points\n", pointsCount);
      CORE.tour.clear(pointsCount);
    }
      break;
    case 9: // SEND_TOUR_DATA_CHUNK
    {
      uint16_t chunkSize = bytesToUint16(data + 1, false);
      DEBUG("Receiving tour data chunk with %u points\n", chunkSize);
      for (uint16_t i = 0; i < chunkSize; i++) {
        //NOTE: point index is important for sorting and to mark connections between adjacent points
        uint16_t pointIndex = bytesToUint16(data + 3 + i * 10, false);
        float latitude = bytesToFloat(data + 5 + i * 10, false);
        float longitude = bytesToFloat(data + 9 + i * 10, false);
        CORE.tour.pushPoint(pointIndex, latitude, longitude);
      }
    }
      break;
    default:
      std::cerr << "Unknown message: " << (uint8_t) data[0] << std::endl;
      break;
  }
}

int main() {
#if USE_DEV_LIB
  std::cout << "Using dev lib" << std::endl;
#endif

  // Exception handling:ctrl + c
  signal(SIGINT, terminate);

  CORE.start();


  /* LCD module Init */
  if (DEV_ModuleInit() != 0) {
    DEV_ModuleExit();
    exit(0);
  }

  pthread_t display_thread_id;
  pthread_t bluetooth_thread_id;

  pthread_create(&display_thread_id, nullptr, displayThread, nullptr);
  pthread_create(&bluetooth_thread_id, nullptr,
                 (void *(*)(void *)) bluetoothThread, (void *) handleMessage);

  pthread_join(bluetooth_thread_id, nullptr);
  pthread_cancel(display_thread_id);

  return 0;
}
