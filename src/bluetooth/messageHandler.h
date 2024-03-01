#ifndef BIKETOURASSISTANT_MESSAGEHANDLER_H
#define BIKETOURASSISTANT_MESSAGEHANDLER_H

#include <vector>
#include <cstdint>

#define MESSAGE_OUT_SIZE 64

enum MessagePriority {
  PRIORITY_VERY_HIGH = 1,
  PRIORITY_HIGH,
  PRIORITY_NORMAL,
  PRIORITY_LOW,
  PRIORITY_VERY_LOW
};

enum MessageOutType {
  MESSAGE_OUT_PONG = 1,
  MESSAGE_OUT_REQUEST_TILE,
};

void handleMessage(uint8_t *data);

void sendMessage(MessageOutType type, std::vector<uint8_t> data, MessagePriority priority); // Real message data starts from 7th byte
void sendMessage(MessageOutType type);
void onOutMessageConfirmation();
void resetOutMessagesQueue();

#endif //BIKETOURASSISTANT_MESSAGEHANDLER_H
