// ELKS image viewer

#include "picojpeg.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>
#include <assert.h>

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
//------------------------------------------------------------------------------
typedef unsigned int uint;
//------------------------------------------------------------------------------

#define VGA_256_COLOR_MODE  0x13      /* use to set 256-color mode. */
#define TEXT_MODE           0x03      /* use to set 80x25 text mode. */

uint8_t vgapal[256][3] = {
    /* colors 0-15 */
    {0x00, 0x00, 0x00},

    {0x00, 0x00, 0xAA},
    {0x00, 0xAA, 0x00},
    {0x00, 0xAA, 0xAA},
    {0xAA, 0x00, 0x00},
    {0xAA, 0x00, 0xAA},

    {0xAA, 0x55, 0x00},
    {0xAA, 0xAA, 0xAA},
    {0x55, 0x55, 0x55},
    {0x55, 0x55, 0xFF},
    {0x55, 0xFF, 0x55},

    {0x55, 0xFF, 0xFF},
    {0xFF, 0x55, 0x55},
    {0xFF, 0x55, 0xFF},
    {0xFF, 0xFF, 0x55},
    {0xFF, 0xFF, 0xFF},

    /* grayscale 16-31 (non gamma corrected */
    {0x00, 0x00, 0x00},
    {0x14, 0x14, 0x14},
    {0x20, 0x20, 0x20},
    {0x2C, 0x2C, 0x2C},
    {0x38, 0x38, 0x38},

    {0x45, 0x45, 0x45},
    {0x51, 0x51, 0x51},
    {0x61, 0x61, 0x61},
    {0x71, 0x71, 0x71},
    {0x82, 0x82, 0x82},

    {0x92, 0x92, 0x92},
    {0xA2, 0xA2, 0xA2},
    {0xB6, 0xB6, 0xB6},
    {0xCB, 0xCB, 0xCB},
    {0xE3, 0xE3, 0xE3},

    {0xFF, 0xFF, 0xFF},
    // HERE ------> 1
    /* hue mix 32-55 (1 */
    {0x00, 0x00, 0xFF},
    {0x41, 0x00, 0xFF},
    {0x7D, 0x00, 0xFF},
    {0xBE, 0x00, 0xFF},

    {0xFF, 0x00, 0xFF},
    {0xFF, 0x00, 0xBE},
    {0xFF, 0x00, 0x7D},
    {0xFF, 0x00, 0x41},
    {0xFF, 0x00, 0x00},

    {0xFF, 0x41, 0x00},
    {0xFF, 0x7D, 0x00},
    {0xFF, 0xBE, 0x00},
    {0xFF, 0xFF, 0x00},
    {0xBE, 0xFF, 0x00},

    {0x7D, 0xFF, 0x00},
    {0x41, 0xFF, 0x00},
    {0x00, 0xFF, 0x00},
    {0x00, 0xFF, 0x41},
    {0x00, 0xFF, 0x7D},

    {0x00, 0xFF, 0xBE},
    {0x00, 0xFF, 0xFF},
    {0x00, 0xBE, 0xFF},
    {0x00, 0x7D, 0xFF},
    {0x00, 0x41, 0xFF},

    /* hue mix 56-79 (2 */
    {0x7D, 0x7D, 0xFF},
    {0x9E, 0x7D, 0xFF},
    {0xBE, 0x7D, 0xFF},
    {0xDF, 0x7D, 0xFF},
    {0xFF, 0x7D, 0xFF},

    {0xFF, 0x7D, 0xDF},
    {0xFF, 0x7D, 0xBE},
    {0xFF, 0x7D, 0x9E},
    {0xFF, 0x7D, 0x7D},
    {0xFF, 0x9E, 0x7D},

    {0xFF, 0xBE, 0x7D},
    {0xFF, 0xDF, 0x7D},
    {0xFF, 0xFF, 0x7D},
    {0xDF, 0xFF, 0x7D},
    {0xBE, 0xFF, 0x7D},

    {0x9E, 0xFF, 0x7D},
    {0x7D, 0xFF, 0x7D},
    {0x7D, 0xFF, 0x9E},
    {0x7D, 0xFF, 0xBE},
    {0x7D, 0xFF, 0xDF},

    {0x7D, 0xFF, 0xFF},
    {0x7D, 0xDF, 0xFF},
    {0x7D, 0xBE, 0xFF},
    {0x7D, 0x9E, 0xFF},
    /* hue mix 80-103 (3 */
    {0xB6, 0xB6, 0xFF},

    {0xC7, 0xB6, 0xFF},
    {0xDB, 0xB6, 0xFF},
    {0xEB, 0xB6, 0xFF},
    {0xFF, 0xB6, 0xFF},
    {0xFF, 0xB6, 0xEB},

    {0xFF, 0xB6, 0xDB},
    {0xFF, 0xB6, 0xC7},
    {0xFF, 0xB6, 0xB6},
    {0xFF, 0xC7, 0xB6},
    {0xFF, 0xDB, 0xB6},

    {0xFF, 0xEB, 0xB6},
    {0xFF, 0xFF, 0xB6},
    {0xEB, 0xFF, 0xB6},
    {0xDB, 0xFF, 0xB6},
    {0xC7, 0xFF, 0xB6},

    {0xB6, 0xFF, 0xB6},
    {0xB6, 0xFF, 0xC7},
    {0xB6, 0xFF, 0xDB},
    {0xB6, 0xFF, 0xEB},
    {0xB6, 0xFF, 0xFF},

    {0xB6, 0xEB, 0xFF},
    {0xB6, 0xDB, 0xFF},
    {0xB6, 0xC7, 0xFF},
    // HERE ------> 2
    /* hue mix 104-127 (4 dark 1 */
    {0x00, 0x00, 0x71},
    {0x1C, 0x00, 0x71},
    {0x38, 0x00, 0x71},
    {0x55, 0x00, 0x71},

    {0x71, 0x00, 0x71},
    {0x71, 0x00, 0x55},
    {0x71, 0x00, 0x38},
    {0x71, 0x00, 0x1C},
    {0x71, 0x00, 0x00},

    {0x71, 0x1C, 0x00},
    {0x71, 0x38, 0x00},
    {0x71, 0x55, 0x00},
    {0x71, 0x71, 0x00},
    {0x55, 0x71, 0x00},

    {0x38, 0x71, 0x00},
    {0x1C, 0x71, 0x00},
    {0x00, 0x71, 0x00},
    {0x00, 0x71, 0x1C},
    {0x00, 0x71, 0x38},

    {0x00, 0x71, 0x55},
    {0x00, 0x71, 0x71},
    {0x00, 0x55, 0x71},
    {0x00, 0x38, 0x71},
    {0x00, 0x1C, 0x71},

    /* hue mix 56-79 (2 */
    {0x38, 0x38, 0x71},
    {0x45, 0x38, 0x71},
    {0x55, 0x38, 0x71},
    {0x61, 0x38, 0x71},
    {0x71, 0x38, 0x71},

    {0x71, 0x38, 0x61},
    {0x71, 0x38, 0x55},
    {0x71, 0x38, 0x45},
    {0x71, 0x38, 0x38},
    {0x71, 0x45, 0x38},

    {0x71, 0x55, 0x38},
    {0x71, 0x61, 0x38},
    {0x71, 0x71, 0x38},
    {0x61, 0x71, 0x38},
    {0x55, 0x71, 0x38},

    {0x45, 0x71, 0x38},
    {0x38, 0x71, 0x38},
    {0x38, 0x71, 0x45},
    {0x38, 0x71, 0x55},
    {0x38, 0x71, 0x61},

    {0x38, 0x71, 0x71},
    {0x38, 0x61, 0x71},
    {0x38, 0x55, 0x71},
    {0x38, 0x45, 0x71},
    /* hue mix 80-103 (3 */
    {0x51, 0x51, 0x71},

    {0x59, 0x51, 0x71},
    {0x61, 0x51, 0x71},
    {0x69, 0x51, 0x71},
    {0x71, 0x51, 0x71},
    {0x71, 0x51, 0x69},

    {0x71, 0x51, 0x61},
    {0x71, 0x51, 0x59},
    {0x71, 0x51, 0x51},
    {0x71, 0x59, 0x51},
    {0x71, 0x61, 0x51},

    {0x71, 0x69, 0x51},
    {0x71, 0x71, 0x51},
    {0x69, 0x71, 0x51},
    {0x61, 0x71, 0x51},
    {0x59, 0x71, 0x51},

    {0x51, 0x71, 0x51},
    {0x51, 0x71, 0x59},
    {0x51, 0x71, 0x61},
    {0x51, 0x71, 0x69},
    {0x51, 0x71, 0x71},

    {0x51, 0x69, 0x71},
    {0x51, 0x61, 0x71},
    {0x51, 0x59, 0x71},
    // HERE ------> 3
    /* hue mix 104-127 (4 dark 1 */
    {0x00, 0x00, 0x41},
    {0x10, 0x00, 0x41},
    {0x20, 0x00, 0x41},
    {0x30, 0x00, 0x41},

    {0x41, 0x00, 0x41},
    {0x41, 0x00, 0x30},
    {0x41, 0x00, 0x20},
    {0x41, 0x00, 0x10},
    {0x41, 0x00, 0x00},

    {0x41, 0x10, 0x00},
    {0x41, 0x20, 0x00},
    {0x41, 0x30, 0x00},
    {0x41, 0x41, 0x00},
    {0x30, 0x41, 0x00},

    {0x20, 0x41, 0x00},
    {0x10, 0x41, 0x00},
    {0x00, 0x41, 0x00},
    {0x00, 0x41, 0x10},
    {0x00, 0x41, 0x20},

    {0x00, 0x41, 0x30},
    {0x00, 0x41, 0x41},
    {0x00, 0x30, 0x41},
    {0x00, 0x20, 0x41},
    {0x00, 0x10, 0x41},

    /* hue mix 56-79 (2 */
    {0x20, 0x20, 0x41},
    {0x28, 0x20, 0x41},
    {0x30, 0x20, 0x41},
    {0x3C, 0x20, 0x41},
    {0x41, 0x20, 0x41},

    {0x41, 0x20, 0x3C},
    {0x41, 0x20, 0x30},
    {0x41, 0x20, 0x28},
    {0x41, 0x20, 0x20},
    {0x41, 0x28, 0x20},

    {0x41, 0x30, 0x20},
    {0x41, 0x3C, 0x20},
    {0x41, 0x41, 0x20},
    {0x3C, 0x41, 0x20},
    {0x30, 0x41, 0x20},

    {0x28, 0x41, 0x20},
    {0x20, 0x41, 0x20},
    {0x20, 0x41, 0x28},
    {0x20, 0x41, 0x30},
    {0x20, 0x41, 0x3C},

    {0x20, 0x41, 0x41},
    {0x20, 0x3C, 0x41},
    {0x20, 0x30, 0x41},
    {0x20, 0x28, 0x41},
    /* hue mix 80-103 (3 */
    {0x2C, 0x2C, 0x41},

    {0x30, 0x2C, 0x41},
    {0x34, 0x2C, 0x41},
    {0x3C, 0x2C, 0x41},
    {0x41, 0x2C, 0x41},
    {0x41, 0x2C, 0x3C},

    {0x41, 0x2C, 0x34},
    {0x41, 0x2C, 0x30},
    {0x41, 0x2C, 0x2C},
    {0x41, 0x30, 0x2C},
    {0x41, 0x34, 0x2C},

    {0x41, 0x3C, 0x2C},
    {0x41, 0x41, 0x2C},
    {0x3C, 0x41, 0x2C},
    {0x34, 0x41, 0x2C},
    {0x30, 0x41, 0x2C},

    {0x2C, 0x41, 0x2C},
    {0x2C, 0x41, 0x30},
    {0x2C, 0x41, 0x34},
    {0x2C, 0x41, 0x3C},
    {0x2C, 0x41, 0x41},

    {0x2C, 0x3C, 0x41},
    {0x2C, 0x34, 0x41},
    {0x2C, 0x30, 0x41},

    /* all black */
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},

    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},

};

uint8_t rgb2vga(int r, int g, int b) {

	int closest = 32000;
	int ndx = 0;
	for (int i = 0; i < 248; i++) {
		uint8_t *sample = vgapal[i];
		int rs = (sample[0] > r)? sample[0] - r : r - sample[0];
		int gs = (sample[1] > g)? sample[1] - g : g - sample[1];
		int bs = (sample[2] > b)? sample[2] - b : b - sample[2];
		int dst = rs + gs + bs;

		// printf("dist %d\n", dst);

		if (closest > dst) {
			closest = dst;
			ndx = i;
		}
		else if (dst < 30) {
			ndx = i;
			break;
		}

	}

	return (uint8_t)ndx;
}


uint8_t __far *VGA = (void __far *)0xA0000000L;        /* this points to video VGA memory. */

void plot_pixel(int x,int y, uint8_t color)
{
	// x + (y*320)
     /*  y*320 = y*256 + y*64 = y*2^8 + y*2^6   */
    int offset = (y<<8)+(y<<6)+x;
	VGA[offset] = color;
}

void mode3();
#pragma aux mode3 =								\
"mov AH,0", \
"mov AL,3H", \
"int 10H", \
modify [ AH AL ];

void mode13();
#pragma aux mode13 =								\
"mov AH,0", \
"mov AL,13H", \
"int 10H", \
modify [ AH AL ];


void set_mode(uint8_t mode)
{
	if (mode == VGA_256_COLOR_MODE)
		mode13();
	if (mode == TEXT_MODE)
		mode3();
}



static int print_usage()
{
   printf("Usage: eview [source_file] [dest_file] <reduce>\n");
   printf("source_file: JPEG file to decode. Note: Progressive files are not supported.\n");
   printf("dest_file: Output .raw file.\n");
   printf("reduce: Optional, if 1 the JPEG file is quickly decoded to ~1/8th resolution.\n");
   printf("\n");
   printf("Outputs 8-bit grayscale or truecolor 24-bit raw files.\n");
   return EXIT_FAILURE;
}
//------------------------------------------------------------------------------
static FILE *g_pInFile;
static uint g_nInFileSize;
static uint g_nInFileOfs;
//------------------------------------------------------------------------------
unsigned char pjpeg_need_bytes_callback(unsigned char* pBuf, unsigned char buf_size, unsigned char *pBytes_actually_read, void *pCallback_data)
{
   uint n;
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
uint8_t *pjpeg_load_from_file(const char *pFilename, int *x, int *y, int *comps, pjpeg_scan_type_t *pScan_type, int reduce)
{
   pjpeg_image_info_t image_info;
   int mcu_x = 0;
   int mcu_y = 0;
   uint row_pitch;
   uint8_t *pImage;
   uint8_t status;
   uint decoded_width, decoded_height;
   uint row_blocks_per_mcu, col_blocks_per_mcu;

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
   pImage = (uint8_t *)malloc(row_pitch * decoded_height);
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
            uint y, x;
            for (y = 0; y < col_blocks_per_mcu; y++)
            {
               uint src_ofs = (y * 128U);
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

         for (y = 0; y < image_info.m_MCUHeight; y += 8)
         {
            const int by_limit = min(8, image_info.m_height - (mcu_y * image_info.m_MCUHeight + y));

            for (x = 0; x < image_info.m_MCUWidth; x += 8)
            {
               uint8_t *pDst_block = pDst_row + x * image_info.m_comps;

               // Compute source byte offset of the block in the decoder's MCU buffer.
               uint src_ofs = (x * 8U) + (y * 16U);
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

static void get_pixel(int* pDst, const uint8_t *pSrc, int luma_only, int num_comps)
{
   int r, g, b;
   if (num_comps == 1)
   {
      r = g = b = pSrc[0];
   }
   else if (luma_only)
   {
       // will this run on 16 bit int?
      const int YR = 19595, YG = 38470, YB = 7471;
      r = g = b = (pSrc[0] * YR + pSrc[1] * YG + pSrc[2] * YB + 32768) / 65536;
   }
   else
   {
      r = pSrc[0]; g = pSrc[1]; b = pSrc[2];
   }
   pDst[0] = r; pDst[1] = g; pDst[2] = b;
}

//------------------------------------------------------------------------------
int main(int arg_c, char *arg_v[])
{
   int n = 1;
   const char *pSrc_filename = NULL;
   const char *pDst_filename = NULL;
   int width, height, comps;
   pjpeg_scan_type_t scan_type;
   const char* p = "?";
   uint8_t *pImage;
   int reduce = 0;
   
   printf("ELKS JPG Viewer v0.1\n");

   if ((arg_c < 2) || (arg_c > 4))
      return print_usage();
   
   pSrc_filename = arg_v[n++];
   if (arg_c > 2)
       pDst_filename = arg_v[n++];
   if (arg_c == 4)
      reduce = atoi(arg_v[n++]) != 0;

   printf("Source file:      \"%s\"\n", pSrc_filename);
   if (pDst_filename)
       printf("Destination file: \"%s\"\n", pDst_filename);
   printf("Reduce during decoding: %u\n", reduce);

   set_mode(VGA_256_COLOR_MODE);

   pImage = pjpeg_load_from_file(pSrc_filename, &width, &height, &comps, &scan_type, reduce);
   if (!pImage)
   {
      printf("Failed loading source image!\n");
      return EXIT_FAILURE;
   }

   printf("Width: %i, Height: %i, Comps: %i\n", width, height, comps);

   switch (scan_type)
   {
      case PJPG_GRAYSCALE: p = "GRAYSCALE"; break;
      case PJPG_YH1V1: p = "H1V1"; break;
      case PJPG_YH2V1: p = "H2V1"; break;
      case PJPG_YH1V2: p = "H1V2"; break;
      case PJPG_YH2V2: p = "H2V2"; break;
   }
   printf("Scan type: %s\n", p);


   printf("Test mode - will show a known pattern then show the image for 10s.\n");


   set_mode(VGA_256_COLOR_MODE);

   // known pattern from vgatest
   for (int i=0;i < width;i++)
	   plot_pixel(i,100,5);

   for (int i=0; i < height;i++)
	   plot_pixel(100,i,0xA);

   set_mode(TEXT_MODE);

   printf("Now the decoded image will be displayed.\n");
   sleep (2);

   set_mode(VGA_256_COLOR_MODE);

   // this is wrong, fix image format conversion to VGA memory
   int y; int x;
   for (y = 0; y < height; y++)
   {
	   for (x = 0; x < width; x++)
	   {
		   int pixel[3];
		   get_pixel(pixel, &pImage[(y * width + x) * comps], 0, comps);
//		   fprintf(stderr, "%d %d %d\n", pixel[0], pixel[1], pixel[2]);
           uint8_t eightBitColor = rgb2vga(pixel[0], pixel[1], pixel[2]);
		   plot_pixel(x, y, eightBitColor);
	  }
   }
   sleep (10);
   set_mode(TEXT_MODE);

   if (pDst_filename)
   {
       FILE *fout = fopen(pDst_filename,"w");
       int i;
       for (i = 0; i < height; i++)
           fwrite(pImage, width*comps, 1, fout);
       fclose(fout);
       printf("Successfully wrote destination file %s\n", pDst_filename);
   }

   free(pImage);

   return EXIT_SUCCESS;
}
//------------------------------------------------------------------------------

