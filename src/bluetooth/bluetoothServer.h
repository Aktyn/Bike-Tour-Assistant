#ifndef __BLUETOOTH_SERVER_H
#define __BLUETOOTH_SERVER_H

void startBluetoothServer(void (*onMessage)(unsigned char *data));

void sendBluetoothMessage(unsigned char *data);

#endif // __BLUETOOTH_SERVER_H