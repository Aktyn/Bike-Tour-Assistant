#ifndef __CORE_H
#define __CORE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
  bool isBluetoothConnected;
  bool isRunning;
} CORE_STATE;
extern CORE_STATE CoreState;

void Core_Init(void);
bool isBluetoothConnected(void);
bool isBluetoothDisconnected(void);

#endif // __CORE_H