#include "Debug.h"
#include "bluetooth/bluetooth_server.h"
#include "display/intro_view.h"
#include "display/draw.h"
#include "camera/camera.h"
#include "core/core.h"
#include "utils.h"

#include <cstdio>
#include <cstdlib>
#include <csignal> //signal()
#include <pthread.h>
#include <ctime>
#include <iostream>
#include <cmath>

extern "C"
{
#include "DEV_Config.h"
#include "LCD_2inch4.h"
#include "GUI_Paint.h"
}

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
    clearScreen(BLACK);
    drawLine("Waiting for map data",
             0, LCD_2IN4_HEIGHT * 3 / 4 - Font16.Height / 2, LCD_2IN4_WIDTH,
             WHITE, BLACK, &Font16, ALIGN_CENTER);

    while (CORE.isBluetoothConnected) {
      if (CORE.needMapRedraw) {
        CORE.needMapRedraw = false;
        uint16_t *mapBuffer = CORE.generateMap();
        if (mapBuffer == nullptr) {
          std::cerr << "Failed to generate map" << std::endl;
          continue;
        }
        drawImageBuffer(mapBuffer, 0, LCD_2IN4_HEIGHT - MAP_HEIGHT, MAP_WIDTH, MAP_HEIGHT);
        free(mapBuffer);
      }

      if (CORE.needSpeedRedraw) {
        CORE.needSpeedRedraw = false;
        std::string speedText = std::to_string((uint16_t) std::round(CORE.location.speed));
        //TODO: draw digits from bitmap files (cache loaded buffers)
        drawLine(speedText.c_str(),
                 0, (LCD_2IN4_HEIGHT - MAP_HEIGHT) / 2 - Font24.Height / 2, LCD_2IN4_WIDTH,
                 WHITE, BLACK, &Font24, ALIGN_CENTER);
      }

      // sleep for 16ms
      usleep(16 * 1000);
    }
  }
  return nullptr;
}

void *takePhotoAsync(void *args) {
  std::cout << "Taking photo" << std::endl;
  std::string file_path = takePhoto();
  if (file_path.empty()) {
    std::cout << "Error taking photo" << std::endl;
    return nullptr;
  }

  if (CORE.isBluetoothConnected) {
    drawImageFromJpgFile(file_path.c_str(), 0, 0, LCD_2IN4_WIDTH);
  }

  return nullptr;
}

void terminate(int signo) {
  Handler_2IN4_LCD(signo);
}

void handleMessage(unsigned char *data) {
  if (!CORE.isBluetoothConnected) {
    return;
  }

  switch (data[0]) {
    case 1: // PING
      DEBUG("Ping\n");
      if (CORE.isBluetoothConnected) {
        // TODO: send pong
      }
      break;
    case 2: { //SET_LIGHTNESS
      std::cout << "Setting backlight to: " << std::to_string((uint8_t) data[1]) << "%" << std::endl;
      uint8_t lightness = data[1];
      LCD_SetBacklight(lightness * 10);
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
      float latitude = bytesToFloat(data + 1, false);
      float longitude = bytesToFloat(data + 5, false);
      float speed = bytesToFloat(data + 9, false);
      float heading = bytesToFloat(data + 13, false);
      uint64_t timestamp = bytesToUint64(data + 17, false);
      DEBUG("Location: %f, %f, %f, %f, %lu\n", latitude, longitude, speed, heading, timestamp);
      CORE.updateLocation(latitude, longitude, speed, heading, timestamp);
    }
      break;
    case 5: // SEND_MAP_TILE_START
    {
      uint32_t x = bytesToUint32(data + 1, false);
      uint32_t y = bytesToUint32(data + 5, false);
      uint32_t z = bytesToUint32(data + 9, false);
      uint16_t tileWidth = bytesToUint16(data + 13, false);
      uint16_t tileHeight = bytesToUint16(data + 15, false);
      uint32_t dataByteLength = bytesToUint32(data + 17, false);
      uint16_t paletteSize = bytesToUint16(data + 21, false);
      DEBUG("Map tile start: %d, %d, %d, %d, %d, %d, %d\n",
            x, y, z, tileWidth, tileHeight, dataByteLength, paletteSize);
      CORE.registerTile(x, y, z, tileWidth, tileHeight, dataByteLength, paletteSize);
    }
      break;
    case 6: // SEND_MAP_TILE_DATA_CHUNK
    {
      uint16_t chunkIndex = bytesToUint16(data + 1, false);
      DEBUG("Map tile data chunk %d\n", chunkIndex);
      CORE.appendTileImageData(chunkIndex, data + 3);
    }
      break;
    case 7: // SEND_MAP_TILE_END
    {
      uint32_t x = bytesToUint32(data + 1, false);
      uint32_t y = bytesToUint32(data + 5, false);
      uint32_t z = bytesToUint32(data + 9, false);
      DEBUG("Map tile end: %d, %d, %d\n", x, y, z);
      CORE.finalizeTile(x, y, z);
    }
      break;
    case 8: //SEND_MAP_TILE_INDEXED_COLORS_BATCH_48
    {
      for (int i = 0; i < 48; i++) {
        uint16_t colorIndex = bytesToUint16(data + 1 + i * 5, false);
        uint8_t red = data[3 + i * 5];
        uint8_t green = data[4 + i * 5];
        uint8_t blue = data[5 + i * 5];
        DEBUG("Map tile indexed color: %d, %d, %d, %d\n", colorIndex, red, green, blue);
        CORE.registerIndexedColor(colorIndex, red, green, blue);
      }
    }
      break;
    default:
      std::cerr << "Unknown message: " << (uint8_t) data[0] << std::endl;
      break;
  }
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
  // pthread_join(display_thread_id, NULL);

  return 0;
}

// TODO: cleanup
//  void LCD_2IN4_test(void)
//  {
//    // Exception handling:ctrl + c
//    signal(SIGINT, Handler_2IN4_LCD);

//   /* Module Init */
//   if (DEV_ModuleInit() != 0)
//   {
//     DEV_ModuleExit();
//     exit(0);
//   }

//   /* LCD Init */
//   printf("2inch4 LCD demo...\n");
//   LCD_2IN4_Init();
//   LCD_2IN4_Clear(WHITE);
//   LCD_SetBacklight(1000); // 1024

//   UDOUBLE image_size = LCD_2IN4_HEIGHT * LCD_2IN4_WIDTH * 2;
//   UWORD *BlackImage;
//   if ((BlackImage = (UWORD *)malloc(image_size)) == NULL)
//   {
//     printf("Failed to apply for black memory...\n");
//     exit(0);
//   }

//   // /*1.Create a new image cache named IMAGE_RGB and fill it with white*/
//   Paint_NewImage(BlackImage, LCD_2IN4_WIDTH, LCD_2IN4_HEIGHT, 0, WHITE, 16);
//   Paint_Clear(WHITE);
//   Paint_SetRotate(ROTATE_0);
//   // /* GUI */
//   printf("drawing...\n");
//   // /*2.Drawing on the image*/
//   Paint_DrawPoint(5, 10, BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT);
//   Paint_DrawPoint(5, 25, BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
//   Paint_DrawPoint(5, 40, BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);
//   Paint_DrawPoint(5, 55, BLACK, DOT_PIXEL_4X4, DOT_STYLE_DFT);

//   Paint_DrawLine(20, 10, 70, 60, RED, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
//   Paint_DrawLine(70, 10, 20, 60, RED, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
//   Paint_DrawLine(170, 15, 170, 55, RED, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
//   Paint_DrawLine(150, 35, 190, 35, RED, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);

//   Paint_DrawRectangle(20, 10, 70, 60, BLUE, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
//   Paint_DrawRectangle(85, 10, 130, 60, BLUE, DOT_PIXEL_1X1, DRAW_FILL_FULL);

//   Paint_DrawCircle(170, 35, 20, GREEN, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
//   Paint_DrawCircle(170, 85, 20, GREEN, DOT_PIXEL_1X1, DRAW_FILL_FULL);

//   Paint_DrawString_EN(5, 70, "hello world", &Font16, WHITE, BLACK);
//   Paint_DrawString_EN(5, 90, "waveshare", &Font20, RED, IMAGE_BACKGROUND);

//   Paint_DrawNum(5, 160, 123456789, &Font20, GREEN, IMAGE_BACKGROUND);
//   // Paint_DrawString_CN(5, 200, "微雪电子", &Font24CN, IMAGE_BACKGROUND, BLUE);
//   // /*3.Refresh the picture in RAM to LCD*/
//   LCD_2IN4_Display((UBYTE *)BlackImage);
//   DEV_Delay_ms(3000);
//   // /* show bmp */
//   printf("show bmp\n");

//   // GUI_ReadBmp("../assets/LCD_2inch4.bmp");
//   // LCD_2IN4_Display((UBYTE *)BlackImage);
//   DEV_Delay_ms(3000);

//   /* Module Exit */
//   free(BlackImage);
//   BlackImage = NULL;

//   /* Custom */
//   for (uint16_t y = 0; y < LCD_2IN4_HEIGHT; y++)
//   {
//     for (uint16_t x = 0; x < LCD_2IN4_WIDTH; x++)
//     {
//       // LCD_2IN4_SetCursor(x, y);
//       // LCD_2IN4_WriteData_Word(CYAN);
//       LCD_2IN4_DrawPaint(x, y, ((x / 16 + y / 16)) % 2 == 0 ? BLACK : WHITE);
//     }
//   }

//   /* -------*/

//   DEV_ModuleExit();
// }
