#include "core.h"

CORE_STATE CoreState;

void Core_Init()
{
  CoreState.isBluetoothConnected = false;
  CoreState.isRunning = true;
}

bool isBluetoothConnected(void)
{
  return CoreState.isBluetoothConnected;
}

bool isBluetoothDisconnected(void)
{
  return !CoreState.isBluetoothConnected;
}