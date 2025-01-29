/*  ELKS BMP Viewer
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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>
#include <signal.h>

#include "graphics.h"

#define DEBUG

// Uncompress BMP image (either RLE8 or RLE4)
// FROM GIMP: removed most multiplications, all divisions, and delta
void decompress_RLE_BMP(FILE *fp, unsigned char bpp, int width, int height, uint8_t palette){
	int xpos = 0; int ypos = 0;
	int i,j,n;
	uint16_t total_bytes_read = 0;
	uint8_t i_max = 0;
	uint8_t _bpp = 0;
	uint8_t count_val[2] = {0,0};//Get mode Get data

	if (bpp == 8) _bpp = 3;
	if (bpp == 4) _bpp = 2;

	while (ypos < height && xpos <= width)
	{
		// uint16_t y_offset = ypos * width;
		fread(count_val, 1, 2, fp);
		// Count + Color - record
		if (count_val[0] != 0)
		{
            // encoded mode run - count == run_length; val == pixel data
			if (count_val[1])
				count_val[1] += palette;
			for (j = 0; ( j < count_val[0]) && (xpos < width);)
			{
				for (i = 1;((i <= (8 >> _bpp)) && (xpos < width) && ( j < count_val[0]));i++, xpos++, j++)
				{
					plot_pixel(xpos, ypos, (count_val[1] & (((1<<bpp)-1) << (8 - (i << _bpp)))) >> (8 - (i << _bpp)));
					//LT_tile_tempdata[y_offset + xpos] = (count_val[1] & (((1<<bpp)-1) << (8 - (i << _bpp)))) >> (8 - (i << _bpp));
				}
			}
		}
		// uncompressed record
		if ((count_val[0] == 0) && (count_val[1] > 2))
		{
			n = count_val[1];
			total_bytes_read = 0;
			for (j = 0; j < n; j += (8 >> _bpp))
			{
				// read the next byte in the record
				uint8_t c; uint8_t d = 0;
				fread(&c, 1, 1, fp);
				if (c)
					d = palette;
				else
					d = 0;
				c += d;
                total_bytes_read++;
				// read all pixels from that byte
				i_max = 8 >> _bpp;
				if (n - j < i_max) i_max = n - j;
                i = 1;
				while ((i <= i_max) && (xpos < width))
				{
					plot_pixel(xpos, ypos, (c >> (8-(i<<_bpp))) & ((1<<bpp)-1));
                    // LT_tile_tempdata[y_offset + xpos] = (c >> (8-(i<<_bpp))) & ((1<<bpp)-1);
                    i++; xpos++;
                }
            }
			// absolute mode runs are padded to 16-bit alignment
			if (total_bytes_read & 1)
				fseek(fp,1,SEEK_CUR);
		}
		// Line end
		if ((count_val[0] == 0) && (count_val[1] == 0)){ypos++;xpos = 0;}
		// Bitmap end
		if ((count_val[0] == 0) && (count_val[1] == 1)) break;
		// Deltarecord. I did not find any BMP using this
        if ((count_val[0] == 0) && (count_val[1] == 2))
		{
			count_val[0] = fgetc(fp);
			count_val[1] = fgetc(fp);
			xpos += count_val[0]; ypos += count_val[1];
        }
    }
}


int bmp_load_and_display(const char *filename, int mode)
{
	uint16_t header;
	uint16_t dib_header_size;
	uint16_t width, height, num_colors;
	uint8_t pixel_format;
	uint8_t rle;
	uint8_t *old_palette = NULL;
	uint8_t *line_buffer; // max 640 pixels per line on 3 byte colors
	int load_palette = 1; // set to 0 and no palette operation will be made

	FILE *fp = fopen(filename, "r");
	if (!fp)
		fprintf(stderr, "Error. Could not open file.\n");

		//Read header
	fread(&header, 1, 2, fp);
	if (header != 0x4D42)
		fprintf(stderr, "Not a BMP file %s\n",filename);

	uint32_t bmp_size = 0;
	fread(&bmp_size, 1, 4, fp);
	fseek(fp, 4,SEEK_CUR);
	uint32_t offset = 0;
	fread(&offset, 1, 4, fp);

	fread(&dib_header_size, 1, 2, fp );
	fseek(fp, 2, SEEK_CUR);
	fread(&width, 1 ,2, fp);
	fseek(fp, 2, SEEK_CUR);
	fread(&height, 1 ,2, fp);
	fseek(fp, 4, SEEK_CUR);
	fread(&pixel_format, 1, 1, fp);
	fseek(fp, 1, SEEK_CUR);
	fread(&rle, 1,  1, fp);	//0 none, 1 = 8 bit, 2 = 4 bit

	fseek(fp, 15,SEEK_CUR);
	fread(&num_colors, 1, 2, fp);

	if (num_colors == 0)
		num_colors = 1 << pixel_format;

	//Advance to palette data
	dib_header_size -=34;
	fseek(fp, dib_header_size, SEEK_CUR);

#ifdef DEBUG
	fprintf(stderr, "dib_header_size: %hu\n", dib_header_size);
	fprintf(stderr, "width: %hu\n", width);
	fprintf(stderr, "height: %hu\n", height);
	fprintf(stderr, "pixel format: %hu\n", pixel_format);
	fprintf(stderr, "run-length encoding: %hu\n", rle);
	fprintf(stderr, "num colors: %hu\n", num_colors);
	fprintf(stderr, "offset: %u\n", offset);

#endif
	// load palette
	if (pixel_format <= 8)
	{
		if (load_palette)
		{
			uint8_t col[4];
			offset = 0;
			printf("Saving current VGA palette.\n");
			old_palette = malloc(num_colors * 3);
			for(int i = 0; i < num_colors; i++)
			{
				get_palette(col, col+1, col+2, i);

				old_palette[offset++] = col[0] >> 2; // red
				old_palette[offset++] = col[1] >> 2; // green
				old_palette[offset++] = col[2] >> 2; // blue
			}
#ifdef DEBUG
			printf("Palette: \n");
#endif
			for(int i = 0; i < num_colors; i++)
			{
				fread(&col, 1, 4, fp);
				//  blue, green, red, 0x00
#ifdef DEBUG
				printf("%hhu %hhu %hhu\n",col[2], col[1], col[0]);
#endif
				set_palette(col[2], col[1], col[0], i);
			}
		}
		else
		{
			fseek(fp, num_colors << 2, 1);
		}
		// fprintf(stderr, "ftell %ld\n", ftell(fp));

		// TODO: set the palette...

		if (pixel_format == 4)
		{
			if (rle)
			{
				decompress_RLE_BMP(fp,pixel_format,width,height,0);
			}
			else
			{
				uint16_t line_size = width >> 1;
				line_buffer = malloc(line_size);
				for(int i = height - 1; i >= 0; i--)
				{
					fread(line_buffer, 1, line_size, fp);

					for (int j = 0; j < line_size; j++)
					{
						plot_pixel(j<<1, i, line_buffer[j] >> 4);
						plot_pixel((j<<1)+1, i, line_buffer[j] & 0x0F);
					}
				}
				free(line_buffer);

			}
		}
		if (pixel_format == 8)
		{
			if (rle)
			{
				decompress_RLE_BMP(fp,pixel_format,width,height,0);
			}
			else
			{
				uint16_t line_size = width;
				line_buffer = malloc(line_size);

				for(int i = height - 1; i >=0; i--)
				{
					fread(line_buffer, 1, line_size, fp);

					for (int j = 0; j < line_size; j++)
					{
						plot_pixel(j, i, line_buffer[j]);
					}
				}
				free(line_buffer);

			}
		}
		if (pixel_format == 1)
		{
			if (rle)
			{
				// decompress_RLE_BMP(fp,pixel_format,width,height,0);
			}
			else
			{
				uint16_t line_size = width >> 3;
				line_buffer = malloc(line_size);

				for(int i = height - 1; i >= 0; i--)
				{
					fread(line_buffer, 1, line_size, fp);

					for (int j = 0; j < line_size; j++)
					{
						int x_off = j << 3;
						for (int offset = 7; offset >= 0; offset--)
							plot_pixel(x_off + offset, i, (line_buffer[j] >> (7 - offset)) & 1);
					}
				}
				free(line_buffer);
			}
		}
	}

	if (pixel_format == 24)
	{
		uint16_t line_size = width * 3;
		line_buffer = malloc(line_size);

		for(int i = height - 1; i >= 0; i--)
		{
			offset = 0;
			fread(line_buffer, 1, line_size, fp);

			for (int j = 0; j < width; j++)
			{
				uint8_t pixel = rgb2vga(line_buffer[offset+2], line_buffer[offset+1], line_buffer[offset]); // blue, green and red
				plot_pixel(j, i, pixel);
				offset += 3;
			}
		}
		free(line_buffer);
	}

	if (pixel_format <= 8 && load_palette && old_palette)
	{
		// for (int i = 0; i < num_colors; i++)
		printf("Restoring palette.\n");
		int offset = 0;
		for(int i = 0; i < num_colors; i++)
		{
			set_palette(old_palette[offset], old_palette[offset+1], old_palette[offset+2], i);
			offset += 3;
		}
		free(old_palette);
	}


	return 0;

}

void sig_handler(int signo)
{
	if (signo == SIGINT)
		printf("received SIGINT\n");

	set_mode(TEXT_MODE_3);
}

static int print_usage()
{
   printf("Usage: bmpview [source_file.bmp]\n");
   printf("source_file: BMP file to decode.\n");
   printf("\n");
   printf("Displays a BMP image in the screen.\n");
   printf("\n");
   return EXIT_FAILURE;
}

int main(int arg_c, char *arg_v[])
{
   int n = 1;
   const char *filename;

   printf("ELKS BMP Viewer v0.1\n");

   if (arg_c != 2)
      return print_usage();

   filename = arg_v[n];

   if (signal(SIGINT, sig_handler) == SIG_ERR)
	   printf("\ncan't catch SIGINT\n");

   uint16_t mode = get_mode();

   printf("Source file:      \"%s\"\n", filename);
   printf("Current Mode:     \"%hu\"\n\n", mode & 0xff);
   printf("Press any key to diplay the image.\n");

   printf("Then press any key to exit!\n");
   getchar();

   set_mode(VIDEO_MODE_13);

#ifdef DEBUG
   printf("Palette before: \n");
   for (uint16_t i = 0; i < 256; i++)
   {
	   uint8_t red, green, blue;
	   get_palette(&red, &green, &blue, i);
	   printf("%hhu %hhu %hhu\n", red, green, blue);
   }
#endif

   int ret = bmp_load_and_display(filename, VIDEO_MODE_13);

#ifdef DEBUG
   printf("Palette after: \n");
   for (uint16_t i = 0; i < 256; i++)
   {
	   uint8_t red, green, blue;
	   get_palette(&red, &green, &blue, i);
	   printf("%hhu %hhu %hhu\n", red, green, blue);
   }
#endif

   getchar();
   set_mode(TEXT_MODE_3);

   if (ret != 0)
	   fprintf(stderr, "Error reading bmp file.\n");

   return ret;

   return EXIT_SUCCESS;
}
