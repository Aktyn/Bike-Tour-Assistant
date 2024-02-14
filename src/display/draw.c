#include "draw.h"

#include "DEV_Config.h"
#include "LCD_2inch4.h"
#include "GUI_Paint.h"
#include "GUI_BMP.h"

#include <stdio.h>
#include <stdlib.h>

uint16_t *allocateImageBuffer(uint16_t width, uint16_t height)
{
  uint32_t image_size = height * width * 2;
  uint16_t *image;
  if ((image = (uint16_t *)malloc(image_size)) == NULL)
  {
    DEBUG("Failed to allocate memory for image buffer...\n");
    exit(0);
  }
  return image;
}

uint16_t *allocateImageBufferFromBitmapFile(const char *path, uint16_t width, uint16_t height)
{
  uint16_t *image = allocateImageBuffer(width, height);

  Paint_NewImage(image, width, height, 0, WHITE, 24);
  Paint_SetRotate(ROTATE_0);
  GUI_ReadBmp(path);

  return image;
}

void drawImageBuffer(const uint16_t *image, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
  LCD_2IN4_SetWindow(x, y, width + x, height + y);
  DEV_Digital_Write(LCD_DC, 1);
  for (UWORD i = 0; i < height; i++)
  {
    DEV_SPI_Write_nByte((UBYTE *)image + width * 2 * i, width * 2);
  }
}

void drawImageFromBitmapFile(const char *path, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
  uint16_t *image = allocateImageBufferFromBitmapFile(path, width, height);
  drawImageBuffer(image, x, y, width, height);
  free(image);
  image = NULL;
}

void drawLine(const char *text, uint16_t x, uint16_t y, uint16_t width, uint16_t color, uint16_t background, sFONT *font, TEXT_ALIGN align)
{
  uint16_t *textImage = allocateImageBuffer(width, font->Height);

  uint16_t aligned_x = 0;
  switch (align)
  {
  case ALIGN_LEFT:
    aligned_x = 0;
    break;
  case ALIGN_RIGHT:
    aligned_x = width - font->Width * strlen(text);
    break;
  case ALIGN_CENTER:
    aligned_x = (width - font->Width * strlen(text)) / 2;
    break;
  }

  Paint_NewImage(textImage, width, font->Height, 0, WHITE, 16);
  Paint_Clear(background);
  Paint_DrawString_EN(aligned_x, 0, text, font, background, color);
  drawImageBuffer(textImage, x, y, width, font->Height);

  free(textImage);
  textImage = NULL;
}

void clearScreen(uint16_t color)
{
  LCD_2IN4_Clear(color);
}