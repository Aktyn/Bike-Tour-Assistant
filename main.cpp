#include "bluetooth/bluetoothServer.h"
#include "display/intro_view.h"
#include "core/core.h"
#include "core/renderer.h"
#include "bluetooth/messageHandler.h"

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

    resetOutMessagesQueue();
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

void terminate(int signo) {
  Handler_2IN4_LCD(signo);
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
