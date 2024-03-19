#include "messageHandler.h"
#include "bluetoothServer.h"
#include "core/core.h"
#include "utils.h"

#include <iostream>
#include <algorithm>

#define MESSAGE_OUT_SIGNATURE_BYTE_0 0x0D
#define MESSAGE_OUT_SIGNATURE_BYTE_1 0x25


struct AwaitingMessage {
  std::vector<uint8_t> data;
  MessagePriority priority;
};

std::vector<AwaitingMessage> messagesQueue;
static std::vector<uint8_t> messageOutBuffer(MESSAGE_OUT_SIZE, 0);
static uint32_t messageOutIndex = 0;
static bool waitingForOutMessageConfirmation = false;

void handleMessage(uint8_t *data) {
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
      CORE.camera.takePhoto();
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
    case 11: // SET_DISTANCE_PER_PHOTO
    {
      uint16_t distance = bytesToUint16(data + 1, false);
      std::cout << "Setting distance per photo: " << std::to_string(distance) << " meters" << std::endl;
      CORE.camera.setDistancePerPhoto(distance);
    }
      break;
    case 12: // SEND_POINTS_OF_INTEREST_START
    {
      uint16_t pointsCount = bytesToUint16(data + 1, false);
      DEBUG("Receiving points of interest data with %u points\n", pointsCount);
      CORE.tour.resetPointsOfInterest(pointsCount);
    }
      break;
    case 13: // SEND_POINTS_OF_INTEREST_DATA_CHUNK
    {
      uint16_t chunkSize = bytesToUint16(data + 1, false);
      DEBUG("Receiving points of interest data chunk with %u points\n", chunkSize);
      for (uint16_t i = 0; i < chunkSize; i++) {
        float latitude = bytesToFloat(data + 3 + i * 8, false);
        float longitude = bytesToFloat(data + 7 + i * 8, false);
        CORE.tour.pushPointOfInterest(latitude, longitude);
      }
    } break;
    default:
      std::cerr << "Unknown message: " << (uint8_t) data[0] << std::endl;
      break;
  }
}

void sendMessage(MessageOutType type, std::vector<uint8_t> data, MessagePriority priority = PRIORITY_NORMAL) {
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
  data[6] = (uint8_t) type;


  if (waitingForOutMessageConfirmation) {
    messagesQueue.push_back({data, priority});
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
  sendMessage(type, messageOutBuffer, PRIORITY_NORMAL);
}

void onOutMessageConfirmation() {
  if (!messagesQueue.empty()) {
    std::sort(
        messagesQueue.begin(),
        messagesQueue.end(),
        [](const AwaitingMessage &a, const AwaitingMessage &b) {
          return a.priority < b.priority;
        }
    );

    auto message = messagesQueue.front();
    sendBluetoothMessage(&message.data[0]);
    messagesQueue.erase(messagesQueue.begin());
    waitingForOutMessageConfirmation = true;
  } else {
    waitingForOutMessageConfirmation = false;
  }
}

void resetOutMessagesQueue() {
  for (auto &message: messagesQueue) {
    message.data.clear();
  }
  messagesQueue.clear();
  waitingForOutMessageConfirmation = false;
}