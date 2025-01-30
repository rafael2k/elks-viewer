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

//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
static FILE *g_pInFile;
static uint32_t g_nInFileSize;
static uint32_t g_nInFileOfs;
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
// Loads JPEG image from specified file. Returns NULL on failure.
// On success, the malloc()'d image's width/height is written to *x and *y, and
// the number of components (1 or 3) is written to *comps.
// pScan_type can be NULL, if not it'll be set to the image's pjpeg_scan_type_t.
// Not thread safe.
// If reduce is non-zero, the image will be more quickly decoded at approximately
// 1/8 resolution (the actual returned resolution will depend on the JPEG 
// subsampling factor).
uint8_t __far *pjpeg_load_from_file(const char *pFilename, int *x, int *y, int *comps, pjpeg_scan_type_t *pScan_type, int reduce)
{
   pjpeg_image_info_t image_info;
   int mcu_x = 0;
   int mcu_y = 0;
   uint16_t row_pitch;
   uint8_t *pImage;
   uint8_t status;
   uint16_t decoded_width, decoded_height;
   uint16_t row_blocks_per_mcu, col_blocks_per_mcu;

   *x = 0;
   *y = 0;
   *comps = 0;
   if (pScan_type) *pScan_type = PJPG_GRAYSCALE;

   g_pInFile = fopen(pFilename, "rb");
   if (!g_pInFile)
      return NULL;

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
      return NULL;
   }
   
   if (pScan_type)
      *pScan_type = image_info.m_scanType;

   // In reduce mode output 1 pixel per 8x8 block.
   decoded_width = reduce ? (image_info.m_MCUSPerRow * image_info.m_MCUWidth) / 8 : image_info.m_width;
   decoded_height = reduce ? (image_info.m_MCUSPerCol * image_info.m_MCUHeight) / 8 : image_info.m_height;

   printf("decoded width: %u decoded height: %u\n", decoded_width, decoded_height);

   row_pitch = decoded_width * image_info.m_comps;

   // this will not work...
   pImage = (uint8_t __far *)malloc(row_pitch * decoded_height);

   if (!pImage)
   {
      fclose(g_pInFile);
      return NULL;
   }

   row_blocks_per_mcu = image_info.m_MCUWidth >> 3;
   col_blocks_per_mcu = image_info.m_MCUHeight >> 3;
   
   for ( ; ; )
   {
      int y, x;
      uint8_t *pDst_row;

      status = pjpeg_decode_mcu();
      
      if (status)
      {
         if (status != PJPG_NO_MORE_BLOCKS)
         {
            printf("pjpeg_decode_mcu() failed with status %u\n", status);

            free(pImage);
            fclose(g_pInFile);
            return NULL;
         }

         break;
      }

      if (mcu_y >= image_info.m_MCUSPerCol)
      {
         free(pImage);
         fclose(g_pInFile);
         return NULL;
      }

      if (reduce)
      {
         // In reduce mode, only the first pixel of each 8x8 block is valid.
         pDst_row = pImage + mcu_y * col_blocks_per_mcu * row_pitch + mcu_x * row_blocks_per_mcu * image_info.m_comps;
         if (image_info.m_scanType == PJPG_GRAYSCALE)
         {
            *pDst_row = image_info.m_pMCUBufR[0];
         }
         else
         {
            uint16_t y, x;
            for (y = 0; y < col_blocks_per_mcu; y++)
            {
                uint16_t src_ofs = (y * 128U);
                for (x = 0; x < row_blocks_per_mcu; x++)
                {
                    pDst_row[0] = image_info.m_pMCUBufR[src_ofs];
                    pDst_row[1] = image_info.m_pMCUBufG[src_ofs];
                    pDst_row[2] = image_info.m_pMCUBufB[src_ofs];
                    pDst_row += 3;
                    src_ofs += 64;
                }

               pDst_row += row_pitch - 3 * row_blocks_per_mcu;
            }
         }
      }
      else
      {
         // Copy MCU's pixel blocks into the destination bitmap.
         pDst_row = pImage + (mcu_y * image_info.m_MCUHeight) * row_pitch + (mcu_x * image_info.m_MCUWidth * image_info.m_comps);

         fprintf(stderr, "x = %u\ny = %u\n\n", mcu_x * image_info.m_MCUWidth, mcu_y * image_info.m_MCUHeight);

         for (y = 0; y < image_info.m_MCUHeight; y += 8)
         {
            const int by_limit = min(8, image_info.m_height - (mcu_y * image_info.m_MCUHeight + y));

            for (x = 0; x < image_info.m_MCUWidth; x += 8)
            {
               uint8_t *pDst_block = pDst_row + x * image_info.m_comps;

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
                     uint8_t *pDst = pDst_block;

                     for (bx = 0; bx < bx_limit; bx++)
                        *pDst++ = *pSrcR++;

                     pSrcR += (8 - bx_limit);

                     pDst_block += row_pitch;
                  }
               }
               else
               {
                  int bx, by;
                  for (by = 0; by < by_limit; by++)
                  {
                     uint8_t *pDst = pDst_block;

                     for (bx = 0; bx < bx_limit; bx++)
                     {
                        pDst[0] = *pSrcR++;
                        pDst[1] = *pSrcG++;
                        pDst[2] = *pSrcB++;
                        pDst += 3;
                     }

                     pSrcR += (8 - bx_limit);
                     pSrcG += (8 - bx_limit);
                     pSrcB += (8 - bx_limit);

                     pDst_block += row_pitch;
                  }
               }
            }

            pDst_row += (row_pitch * 8);
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

   return pImage;
}

//------------------------------------------------------------------------------
int main(int arg_c, char *arg_v[])
{
   int n = 1;
   const char *pSrc_filename = NULL;
   int width, height, comps;
   pjpeg_scan_type_t scan_type;
   const char* p = "?";
   uint8_t __far *pImage;
   int reduce = 0;
   
   printf("ELKS PPM Viewer v0.1\n");
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


   pImage = pjpeg_load_from_file(pSrc_filename, &width, &height, &comps, &scan_type, reduce);
   if (!pImage)
   {
      printf("Failed loading source image!\n");
      return EXIT_FAILURE;
   }

   switch (scan_type)
   {
      case PJPG_GRAYSCALE: p = "GRAYSCALE"; break;
      case PJPG_YH1V1: p = "H1V1"; break;
      case PJPG_YH2V1: p = "H2V1"; break;
      case PJPG_YH1V2: p = "H1V2"; break;
      case PJPG_YH2V2: p = "H2V2"; break;
   }

   printf("Width: %i, Height: %i, Comps: %i, Scan type: %s\n", width, height, comps, p);

   getchar();

   set_mode(VIDEO_MODE_13);
   if (scan_type == PJPG_GRAYSCALE)
       load_palette1g(VIDEO_MODE_13);
   else
       load_palette1(VIDEO_MODE_13);

   // the buffer can not be >= 64kB
   int y; int x;
   uint32_t off = 0;
   for (y = 0; y < height; y++)
   {
       for (x = 0; x < width; x++)
       {
//   fprintf(stderr, "%d %d %d\n", pixel[0], pixel[1], pixel[2]);
           if (scan_type == PJPG_GRAYSCALE)
           {
               plot_pixel(x, y, pImage[off++]);
           }
           else
           {
               uint8_t eightBitColor = rgb2palette1(pImage[off], pImage[off + 1], pImage[off + 2]);;
               off += 3;
               plot_pixel(x, y, eightBitColor);
           }
       }
   }
   getchar();

   set_mode(TEXT_MODE_3);

#if 0
   if (pDst_filename)
   {
       FILE *fout = fopen(pDst_filename,"w");
       int i;
       for (i = 0; i < height; i++)
           fwrite(pImage, width*comps, 1, fout);
       fclose(fout);
       printf("Successfully wrote destination file %s\n", pDst_filename);
   }
#endif

   free(pImage);

   return EXIT_SUCCESS;
}
//------------------------------------------------------------------------------

