#include "messageHandler.h"
#include "bluetoothServer.h"
#include "core/core.h"
#include "camera/camera.h"
#include "utils.h"

#include <pthread.h>
#include <iostream>
#include <queue>

#define MESSAGE_OUT_SIGNATURE_BYTE_0 0x0D
#define MESSAGE_OUT_SIGNATURE_BYTE_1 0x25

static std::vector<unsigned char> messageOutBuffer(MESSAGE_OUT_SIZE, 0);
std::queue<std::vector<unsigned char>> messagesQueue;
static uint32_t messageOutIndex = 0;
static bool waitingForOutMessageConfirmation = false;

void handleMessage(unsigned char *data) {
  if (!CORE.isBluetoothConnected) {
    return;
  }

  switch (data[0]) {
    case 1: // PING
      DEBUG("Ping\n");
      if (CORE.isBluetoothConnected) {
        sendMessage(MESSAGE_OUT_PONG);
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
      uint8_t mapZoom = data[65];
      DEBUG("Location: %f, %f, %f, %f, %f, %f, %llu\n",
            latitude, longitude, speed, heading, altitude, accuracy, timestamp);
      CORE.updateLocation(latitude, longitude, speed, heading, altitude, altitudeAccuracy, accuracy,
                          timestamp, mapZoom);
    }
      break;
    case 5: // SEND_MAP_TILE_START
    {
      uint32_t x = bytesToUint32(data + 1, false);
      uint32_t y = bytesToUint32(data + 5, false);
      uint8_t z = bytesToUint32(data + 9, false);
      uint32_t dataByteLength = bytesToUint32(data + 10, false);
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
    case 10: // CONFIRM_RECEIVED_MESSAGE
    {
      onOutMessageConfirmation();
    }
      break;
    default:
      std::cerr << "Unknown message: " << (uint8_t) data[0] << std::endl;
      break;
  }
}

void sendMessage(MessageOutType type, std::vector<unsigned char> data) {
  if (!CORE.isBluetoothConnected) {
    return;
  }

  data[0] = MESSAGE_OUT_SIGNATURE_BYTE_0;
  data[1] = MESSAGE_OUT_SIGNATURE_BYTE_1;

  messageOutIndex++;
  auto *indexBytes = (uint8_t *) &messageOutIndex;
  data[2] = indexBytes[0];
  data[3] = indexBytes[1];
  data[4] = indexBytes[2];
  data[5] = indexBytes[3];
  data[6] = (unsigned char) type;


  if (waitingForOutMessageConfirmation) {
    messagesQueue.push(data);
    return;
  }

  switch (type) {
    default:
      std::cerr << "Unknown message type: " << (uint8_t) type << std::endl;
      return;
    case MESSAGE_OUT_PONG:
    case MESSAGE_OUT_REQUEST_TILE:
      sendBluetoothMessage(&data[0]);
      break;
  }

  waitingForOutMessageConfirmation = true;
}

void sendMessage(MessageOutType type) {
  sendMessage(type, messageOutBuffer);
}

void onOutMessageConfirmation() {
  if (!messagesQueue.empty()) {
    auto message = messagesQueue.front();
    messagesQueue.pop();
    sendBluetoothMessage(&message[0]);
    waitingForOutMessageConfirmation = true;
  } else {
    waitingForOutMessageConfirmation = false;
  }
}

void resetOutMessagesQueue() {
  messagesQueue = std::queue<std::vector<unsigned char>>();
  waitingForOutMessageConfirmation = false;
}