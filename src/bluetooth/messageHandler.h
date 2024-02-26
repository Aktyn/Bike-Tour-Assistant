#ifndef BIKETOURASSISTANT_MESSAGEHANDLER_H
#define BIKETOURASSISTANT_MESSAGEHANDLER_H

enum MessageOutType {
  MESSAGE_OUT_PONG = 1,
//  MESSAGE_OUT_TYPE_ERROR
};

void handleMessage(unsigned char *data);

void sendMessage(MessageOutType type, unsigned char *data);

#endif //BIKETOURASSISTANT_MESSAGEHANDLER_H
