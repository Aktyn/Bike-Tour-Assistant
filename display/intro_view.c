#include "intro_view.h"
#include "loader.h"

#include "draw.h"
#include "DEV_Config.h"
#include "LCD_2inch4.h"
#include "GUI_Paint.h"

#include <stdio.h>
#include <stdlib.h>

#define LOGO_WIDTH 128
#define LOGO_HEIGHT 128

#define BLUETOOTH_ICON_WIDTH 64
#define BLUETOOTH_ICON_HEIGHT 64

void showIntroView(void)
{
  printf("Showing intro view...\r\n");

  LCD_2IN4_Init();
  LCD_2IN4_Clear(BLACK);
  LCD_SetBacklight(1000);

  drawImageFromBitmapFile("../assets/logo128x128.bmp",
                          (LCD_2IN4_WIDTH - LOGO_WIDTH) / 2, (LCD_2IN4_HEIGHT / 2 - LOGO_HEIGHT) / 2,
                          LOGO_WIDTH, LOGO_HEIGHT);

  DEV_Delay_ms(500);

  uint16_t text_y = LCD_2IN4_HEIGHT / 2 - Font20.Height;
  drawLine("Bike Tour", 0, text_y, LCD_2IN4_WIDTH, WHITE, BLACK, &Font20, ALIGN_CENTER);
  text_y += Font20.Height;
  drawLine("Assistant", 0, text_y, LCD_2IN4_WIDTH, WHITE, BLACK, &Font20, ALIGN_CENTER);
  text_y += Font20.Height;

  DEV_Delay_ms(500);
  drawLine("Made by Aktyn", 0, text_y, LCD_2IN4_WIDTH, GRAY, BLACK, &Font16, ALIGN_CENTER);

  DEV_Delay_ms(500);
  drawImageFromBitmapFile("../assets/bluetooth_off.bmp",
                          (LCD_2IN4_WIDTH - BLUETOOTH_ICON_WIDTH) / 2, (LCD_2IN4_HEIGHT * 3 / 2 - BLUETOOTH_ICON_HEIGHT) / 2,
                          BLUETOOTH_ICON_WIDTH, BLUETOOTH_ICON_HEIGHT);

  DEV_Delay_ms(500);
  text_y = (LCD_2IN4_HEIGHT * 3 / 2 + BLUETOOTH_ICON_HEIGHT) / 2;
  drawLine("Waiting for", 0, text_y, LCD_2IN4_WIDTH, GRAY, BLACK, &Font12, ALIGN_CENTER);
  text_y += Font12.Height;
  drawLine("bluetooth connection", 0, text_y, LCD_2IN4_WIDTH, GRAY, BLACK, &Font12, ALIGN_CENTER);

  DEV_Delay_ms(500);

  showThreeDotsLoader((LCD_2IN4_WIDTH - THREE_DOTS_LOADER_WIDTH) / 2, LCD_2IN4_HEIGHT - THREE_DOTS_LOADER_HEIGHT - 8);

  DEV_Delay_ms(100);

  DEV_ModuleExit();
}