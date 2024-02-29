#ifndef BIKETOURASSISTANT_MESSAGEHANDLER_H
#define BIKETOURASSISTANT_MESSAGEHANDLER_H

#include <vector>

#define MESSAGE_OUT_SIZE 64

enum MessageOutType {
  MESSAGE_OUT_PONG = 1,
  MESSAGE_OUT_REQUEST_TILE,
};

void handleMessage(unsigned char *data);

void sendMessage(MessageOutType type, std::vector<unsigned char> data); // Real message data starts from 7th byte
void sendMessage(MessageOutType type);
void onOutMessageConfirmation();
void resetOutMessagesQueue();

#endif //BIKETOURASSISTANT_MESSAGEHANDLER_H
