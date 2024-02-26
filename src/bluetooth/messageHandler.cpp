#include "messageHandler.h"
#include "bluetoothServer.h"
#include "core/core.h"
#include "camera/camera.h"
#include "utils.h"

#include <pthread.h>
#include <iostream>

#define MESSAGE_OUT_SIGNATURE_BYTE_0 0x0D
#define MESSAGE_OUT_SIGNATURE_BYTE_1 0x25

static uint32_t messageOutIndex = 0;

void handleMessage(unsigned char *data) {
  if (!CORE.isBluetoothConnected) {
    return;
  }

  switch (data[0]) {
    case 1: // PING
      DEBUG("Ping\n");
      if (CORE.isBluetoothConnected) {
        sendMessage(MESSAGE_OUT_PONG, nullptr);
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
    // TODO: Message requesting which tiles at given zoom level are cached.
    // This device will start responding with cached tiles x, y, z values to prevent unnecessary SEND_MAP_TILE_DATA_CHUNK requests.
    default:
      std::cerr << "Unknown message: " << (uint8_t) data[0] << std::endl;
      break;
  }
}

void sendMessage(MessageOutType type, unsigned char *data) {
  if (!CORE.isBluetoothConnected) {
    return;
  }

  CORE.messageOutBuffer[0] = MESSAGE_OUT_SIGNATURE_BYTE_0;
  CORE.messageOutBuffer[1] = MESSAGE_OUT_SIGNATURE_BYTE_1;

  messageOutIndex++;
  auto *indexBytes = (uint8_t *) &messageOutIndex;
  CORE.messageOutBuffer[2] = indexBytes[0];
  CORE.messageOutBuffer[3] = indexBytes[1];
  CORE.messageOutBuffer[4] = indexBytes[2];
  CORE.messageOutBuffer[5] = indexBytes[3];
  CORE.messageOutBuffer[6] = (unsigned char) type;

  if (data != nullptr) {
    memcpy(CORE.messageOutBuffer + 1, data, 64 - 7);
  }

  switch (type) {
    case MESSAGE_OUT_PONG:
      sendBluetoothMessage(CORE.messageOutBuffer);
      break;
  }
}
