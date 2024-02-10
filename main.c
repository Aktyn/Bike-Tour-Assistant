#include "LCD_2inch4.h"
#include "DEV_Config.h"
#include "display/intro_view.h"
#include "bluetooth/bluetooth_server.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h> //signal()
#include <pthread.h>

void *bluetoothThread(void *input)
{
  startBluetoothServer(input);
  pthread_exit(0);
  return NULL;
}

void *displayThread(void *args)
{
  showIntroView();
  pthread_exit(0);
  return NULL;
}

void terminate(int signo)
{
  Handler_2IN4_LCD(signo);
}

void handleMessage(unsigned char *data)
{
  printf("Data to process: %s\n", data); // TODO
}

int main()
{
#if USE_DEV_LIB
  printf("Using dev lib\r\n");
#endif

  // Exception handling:ctrl + c
  signal(SIGINT, terminate);

  /* LCD module Init */
  if (DEV_ModuleInit() != 0)
  {
    DEV_ModuleExit();
    exit(0);
  }

  pthread_t display_thread_id;
  pthread_t bluetooth_thread_id;

  pthread_create(&display_thread_id, NULL, displayThread, NULL);
  pthread_create(&bluetooth_thread_id, NULL, bluetoothThread, handleMessage);

  pthread_join(bluetooth_thread_id, NULL);
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
//   printf("2inch4 LCD demo...\r\n");
//   LCD_2IN4_Init();
//   LCD_2IN4_Clear(WHITE);
//   LCD_SetBacklight(1000); // 1024

//   UDOUBLE image_size = LCD_2IN4_HEIGHT * LCD_2IN4_WIDTH * 2;
//   UWORD *BlackImage;
//   if ((BlackImage = (UWORD *)malloc(image_size)) == NULL)
//   {
//     printf("Failed to apply for black memory...\r\n");
//     exit(0);
//   }

//   // /*1.Create a new image cache named IMAGE_RGB and fill it with white*/
//   Paint_NewImage(BlackImage, LCD_2IN4_WIDTH, LCD_2IN4_HEIGHT, 0, WHITE, 16);
//   Paint_Clear(WHITE);
//   Paint_SetRotate(ROTATE_0);
//   // /* GUI */
//   printf("drawing...\r\n");
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
//   printf("show bmp\r\n");

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