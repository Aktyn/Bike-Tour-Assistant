#include "loader.h"

#include "draw.h"

#include <stdio.h>
#include <stdlib.h>

extern "C"
{
#include "DEV_Config.h"
#include "LCD_2inch4.h"
#include "GUI_Paint.h"
}

#define DOTS_COUNT 4

void showThreeDotsLoader(uint16_t x, uint16_t y, bool (*condition)())
{
  uint16_t *loaderImage = allocateImageBuffer(THREE_DOTS_LOADER_WIDTH, THREE_DOTS_LOADER_HEIGHT);

  Paint_NewImage(loaderImage, THREE_DOTS_LOADER_WIDTH, THREE_DOTS_LOADER_HEIGHT, 0, WHITE, 24);

  const uint16_t left = THREE_DOTS_LOADER_HEIGHT / 2;
  const uint16_t right = THREE_DOTS_LOADER_WIDTH - THREE_DOTS_LOADER_HEIGHT / 2;
  const uint16_t safe_width = right - left;

  uint16_t offset = 0;

  while (condition() == true)
  {
    Paint_Clear(BLACK);

    for (uint8_t i = 0; i < DOTS_COUNT; i++)
    {
      uint16_t x = (i * (safe_width / (DOTS_COUNT - 1))) + offset;
      uint16_t r = THREE_DOTS_LOADER_HEIGHT / 2 - 1;
      if (x > right)
      {
        r -= (x - right);
      }
      else if (x < left)
      {
        r -= (left - x);
      }
      if (x + r < THREE_DOTS_LOADER_WIDTH)
      {
        Paint_DrawCircle(x, THREE_DOTS_LOADER_HEIGHT / 2, r, CYAN, DOT_PIXEL_8X8, DRAW_FILL_FULL);
      }
    }

    drawImageBuffer(loaderImage, x, y, THREE_DOTS_LOADER_WIDTH, THREE_DOTS_LOADER_HEIGHT);

    DEV_Delay_ms(48);

    offset += 1;
    if (offset >= safe_width / (DOTS_COUNT - 1))
    {
      offset = 0;
    }
  }

  free(loaderImage);
  loaderImage = NULL;
}