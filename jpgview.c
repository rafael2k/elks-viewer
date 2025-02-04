/*  ELKS JPEG Viewer
 * Copyright (C) 2025 Rafael Diniz <rafael@riseup.net>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "picojpeg.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>
#include <signal.h>
#include <assert.h>

#include "graphics.h"

extern void *malloc(size_t size);
extern void *calloc(size_t nmemb, size_t size);
extern void free(void *ptr);

static FILE *g_pInFile;
static uint32_t g_nInFileSize;
static uint32_t g_nInFileOfs;

#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

uint16_t mode = 0;

void sig_handler(int signo)
{
	if (signo == SIGINT)
		printf("received SIGINT\n");

	if (mode)
		set_mode(mode);
}

static int print_usage()
{
   printf("Usage: jpgview [source_file] [dest_file] <reduce>\n");
   printf("source_file: JPEG file to decode. Note: Progressive files are not supported.\n");
   printf("dest_file: Output .raw file.\n");
   printf("reduce: Optional, if 1 the JPEG file is quickly decoded to ~1/8th resolution.\n");
   printf("\n");
   printf("Outputs 8-bit grayscale or truecolor 24-bit raw files.\n");
   return EXIT_FAILURE;
}

unsigned char pjpeg_need_bytes_callback(unsigned char* pBuf, unsigned char buf_size, unsigned char *pBytes_actually_read, void *pCallback_data)
{
   uint32_t n;
   pCallback_data;
   
   n = min(g_nInFileSize - g_nInFileOfs, buf_size);
   if (n && (fread(pBuf, 1, n, g_pInFile) != n))
      return PJPG_STREAM_READ_ERROR;
   *pBytes_actually_read = (unsigned char)(n);
   g_nInFileOfs += n;
   return 0;
}

// Loads JPEG image from specified file and displays it. Returns < 0 on failure.
// On success, 0 is returned, image's width/height is written to *x and *y, and
// the number of components (1 or 3) is written to *comps.
// pScan_type can be NULL, if not it'll be set to the image's pjpeg_scan_type_t.
// Not thread safe.
// If reduce is non-zero, the image will be more quickly decoded at approximately
// 1/8 resolution (the actual returned resolution will depend on the JPEG 
// subsampling factor).
int pjpeg_load_and_display(const char *pFilename, int *x, int *y, int *comps, pjpeg_scan_type_t *pScan_type, int reduce)
{
   pjpeg_image_info_t image_info;
   int mcu_x = 0;
   int mcu_y = 0;
   uint16_t row_pitch;
   uint8_t status;
   uint16_t decoded_width, decoded_height;
   uint16_t row_blocks_per_mcu, col_blocks_per_mcu;

   *x = 0;
   *y = 0;
   *comps = 0;
   if (pScan_type) *pScan_type = PJPG_GRAYSCALE;

   g_pInFile = fopen(pFilename, "rb");
   if (!g_pInFile)
      return -1;

   g_nInFileOfs = 0;

   fseek(g_pInFile, 0, SEEK_END);
   g_nInFileSize = ftell(g_pInFile);
   fseek(g_pInFile, 0, SEEK_SET);
      
   status = pjpeg_decode_init(&image_info, pjpeg_need_bytes_callback, NULL, (unsigned char)reduce);
         
   if (status)
   {
      printf("pjpeg_decode_init() failed with status %u\n", status);
      if (status == PJPG_UNSUPPORTED_MODE)
      {
         printf("Progressive JPEG files are not supported.\n");
      }

      fclose(g_pInFile);
      return -1;
   }

   if (image_info.m_scanType == PJPG_GRAYSCALE)
       load_palette1g(VIDEO_MODE_13);
   else
       load_palette1(VIDEO_MODE_13);

   if (pScan_type)
      *pScan_type = image_info.m_scanType;

   // In reduce mode output 1 pixel per 8x8 block.
   decoded_width = reduce ? (image_info.m_MCUSPerRow * image_info.m_MCUWidth) / 8 : image_info.m_width;
   decoded_height = reduce ? (image_info.m_MCUSPerCol * image_info.m_MCUHeight) / 8 : image_info.m_height;

   // printf("decoded width: %u decoded height: %u\n", decoded_width, decoded_height);

   row_pitch = decoded_width * image_info.m_comps;

   row_blocks_per_mcu = image_info.m_MCUWidth >> 3;
   col_blocks_per_mcu = image_info.m_MCUHeight >> 3;

   for ( ; ; )
   {
      int y, x;

      status = pjpeg_decode_mcu();
      
      if (status)
      {
         if (status != PJPG_NO_MORE_BLOCKS)
         {
            printf("pjpeg_decode_mcu() failed with status %u\n", status);

            fclose(g_pInFile);
            return -1;
         }

         break;
      }

      if (mcu_y >= image_info.m_MCUSPerCol)
      {
         fclose(g_pInFile);
         return -1;
      }

      if (reduce)
      {
         // In reduce mode, only the first pixel of each 8x8 block is valid.
		 int xx = mcu_x * row_blocks_per_mcu;
		 int yy = mcu_y * col_blocks_per_mcu;

         if (image_info.m_scanType == PJPG_GRAYSCALE)
         {
			 drawpixel(xx, yy, image_info.m_pMCUBufR[0]);
         }
         else
         {
            uint16_t y, x;
            for (y = 0; y < col_blocks_per_mcu; y++)
            {
                uint16_t src_ofs = (y * 128U);
                for (x = 0; x < row_blocks_per_mcu; x++)
                {
					uint8_t pixel = rgb2palette1(image_info.m_pMCUBufR[src_ofs], image_info.m_pMCUBufG[src_ofs], image_info.m_pMCUBufB[src_ofs]);
					drawpixel(xx + x, yy + y, pixel);
                    src_ofs += 64;
                }

               xx += row_pitch - row_blocks_per_mcu;
            }
         }
      }
      else
      {
         // Display MCU's pixel blocks to graphics adapter
		  int xx = mcu_x * image_info.m_MCUWidth;
		  int yy = mcu_y * image_info.m_MCUHeight;
		  fprintf(stderr, "x = %u\ny = %u\n\n", mcu_x * image_info.m_MCUWidth, mcu_y * image_info.m_MCUHeight);

		  for (y = 0; y < image_info.m_MCUHeight; y += 8)
		  {
			  const int by_limit = min(8, image_info.m_height - (mcu_y * image_info.m_MCUHeight + y));

			  for (x = 0; x < image_info.m_MCUWidth; x += 8)
			  {
				  // Compute source byte offset of the block in the decoder's MCU buffer.
				  uint16_t src_ofs = (x << 3) + (y << 4);
				  const uint8_t *pSrcR = image_info.m_pMCUBufR + src_ofs;
				  const uint8_t *pSrcG = image_info.m_pMCUBufG + src_ofs;
				  const uint8_t *pSrcB = image_info.m_pMCUBufB + src_ofs;

				  const int bx_limit = min(8, image_info.m_width - (mcu_x * image_info.m_MCUWidth + x));

				  if (image_info.m_scanType == PJPG_GRAYSCALE)
				  {
					  int bx, by;
					  for (by = 0; by < by_limit; by++)
					  {
						  for (bx = 0; bx < bx_limit; bx++)
							  drawpixel(xx + x + bx, yy + y + by, *pSrcR++);

						  pSrcR += (8 - bx_limit);
					  }
				  }
				  else
				  {
					  int bx, by;
					  for (by = 0; by < by_limit; by++)
					  {

						  for (bx = 0; bx < bx_limit; bx++)
						  {
							  uint8_t pixel = rgb2palette1(*pSrcR, *pSrcG, *pSrcB);
							  drawpixel(xx + x + bx, yy + y + by, pixel);
							  pSrcR++;
							  pSrcG++;
							  pSrcB++;

						  }

						  pSrcR += (8 - bx_limit);
						  pSrcG += (8 - bx_limit);
						  pSrcB += (8 - bx_limit);
					  }
				  }
            }

         }
      }

      mcu_x++;
      if (mcu_x == image_info.m_MCUSPerRow)
      {
         mcu_x = 0;
         mcu_y++;
      }
   }

   fclose(g_pInFile);

   *x = decoded_width;
   *y = decoded_height;
   *comps = image_info.m_comps;

   return 0;
}

int main(int arg_c, char *arg_v[])
{
   int n = 1;
   const char *pSrc_filename = NULL;
   int width = 0, height = 0, comps = 0;
   pjpeg_scan_type_t scan_type = PJPG_GRAYSCALE;
   const char* p = "?";
   int reduce = 0;
   
   printf("ELKS JPEG Viewer v0.1\n");
   printf("Based on PicoJPEG\n");

   if ((arg_c < 2) || (arg_c > 3))
      return print_usage();
   
   pSrc_filename = arg_v[n++];
   if (arg_c == 3)
       reduce = atoi(arg_v[n++]) != 0;

   mode = get_mode();

   if (signal(SIGINT, sig_handler) == SIG_ERR)
	   printf("\ncan't catch SIGINT\n");

   printf("Source File:      \"%s\"\n", pSrc_filename);
   printf("Reduce during decoding: %u\n", reduce);
   printf("Current Graphics Mode:     \"%hu\"\n\n", mode);
   printf("Press any key to diplay the image.\n");
   printf("Then press any key to exit!\n");

   getchar();

   set_mode(VIDEO_MODE_13);

   int ret = pjpeg_load_and_display(pSrc_filename, &width, &height, &comps, &scan_type, reduce);

   getchar();

   set_mode(TEXT_MODE_3);

   switch (scan_type)
   {
      case PJPG_GRAYSCALE: p = "GRAYSCALE"; break;
      case PJPG_YH1V1: p = "H1V1"; break;
      case PJPG_YH2V1: p = "H2V1"; break;
      case PJPG_YH1V2: p = "H1V2"; break;
      case PJPG_YH2V2: p = "H2V2"; break;
   }

   if (ret != 0)
	   printf("Error decoding the image.\n");
   else
	   printf("Successfully Decoded Image!\nWidth: %i, Height: %i, Comps: %i, Scan type: %s\n", width, height, comps, p);


   return EXIT_SUCCESS;
}
