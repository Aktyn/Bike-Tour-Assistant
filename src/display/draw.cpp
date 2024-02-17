#include "draw.h"

#include "utils.h"
#include "Debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stdlib.h>
#include <jpeglib.h>
#include <setjmp.h>

extern "C"
{
#include "DEV_Config.h"
#include "LCD_2inch4.h"
#include "GUI_BMP.h"
#include "GUI_Paint.h"
}

uint16_t *allocateImageBuffer(uint16_t width, uint16_t height)
{
  uint32_t image_size = height * width * sizeof(uint16_t);
  uint16_t *image;
  if ((image = (uint16_t *)malloc(image_size)) == NULL)
  {
    DEBUG("Failed to allocate memory for image buffer...\n");
    return NULL;
  }
  return image;
}

uint16_t *allocateImageBufferFromBitmapFile(const char *path, uint16_t width, uint16_t height)
{
  uint16_t *image = allocateImageBuffer(width, height);
  if (image == NULL)
  {
    return NULL;
  }

  Paint_NewImage(image, width, height, 0, WHITE, 24);
  GUI_ReadBmp(path);

  return image;
}

void drawImageBuffer(const uint16_t *image, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
  y = LCD_2IN4_HEIGHT - y - height;

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
  if (image == NULL)
  {
    return;
  }
  drawImageBuffer(image, x, y, width, height);
  free(image);
  image = NULL;
}

int drawImageFromJpgFile(const char *path, uint16_t x, uint16_t y, uint16_t target_width)
{
  FILE *file = fopen(path, "rb");
  if (!file)
  {
    std::cerr << "Error: could not open file " << path << std::endl;
    return 1;
  }

  DEBUG("Drawing image from file: %s\n", path);

  // Initialize the JPEG decompressor
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);

  // Set the file source
  jpeg_stdio_src(&cinfo, file);

  // Read the JPEG header
  jpeg_read_header(&cinfo, TRUE);

  cinfo.scale_num = 1;
  cinfo.scale_denom = findNextPowerOf2((uint16_t)(cinfo.image_width / target_width));

  // Start the decompression
  jpeg_start_decompress(&cinfo);

  // Allocate memory for the decompressed image
  uint32_t row_stride = cinfo.output_width * cinfo.output_components;
  JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

  DEBUG("output width: %d, output height: %d, scale down: %d, num_components: %d\n",
        cinfo.output_width, cinfo.output_height, cinfo.scale_denom, row_stride);

  uint16_t target_height = (cinfo.image_height * target_width) / cinfo.image_width;
  DEBUG("Target height: %d\n", target_height);
  uint16_t *image = allocateImageBuffer(target_width, target_height);
  if (image == NULL)
  {
    return 1;
  }

  while (cinfo.output_scanline < cinfo.output_height)
  {
    (void)jpeg_read_scanlines(&cinfo, buffer, 1);

    uint16_t y = (uint16_t)(float(cinfo.output_scanline - 1) * float(target_height) / float(cinfo.output_height));

    for (uint16_t x = 0; x < target_width; x++)
    {
      uint16_t mapped_x = (uint32_t)(float(x) * float(cinfo.output_width) / float(target_width));

      image[(target_height - 1 - y) * target_width + (target_width - 1 - x)] =
          convertRgbColor(RGB(
              buffer[0][mapped_x * 3 + 0],
              buffer[0][mapped_x * 3 + 1],
              buffer[0][mapped_x * 3 + 2]));
    }
  }

  // Finish the decompression and cleanup
  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  // Close file and free memory
  fclose(file);

  drawImageBuffer(image, x, y, target_width, target_height);
  free(image);
  image = NULL;

  return 0;
}

void drawLine(const char *text, uint16_t x, uint16_t y, uint16_t width, uint16_t color, uint16_t background, sFONT *font, TEXT_ALIGN align)
{
  uint16_t *textImage = allocateImageBuffer(width, font->Height);
  if (textImage == NULL)
  {
    return;
  }
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