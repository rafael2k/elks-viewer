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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>
#include <signal.h>

#include "graphics.h"
#include "utils.h"

// #define DEBUG

uint16_t mode = 0;

#ifndef __C86
void sig_handler(int signo)
{
	if (signo == SIGINT)
		printf("received SIGINT\n");

	// revert to mode in use when software was started
	if (mode)
		set_mode(mode);
}
#endif


// Uncompress BMP image (either RLE8 or RLE4)
void decompress_RLE_BMP(FILE *fp, unsigned char bpp, int width, int height, uint8_t palette){
	int xpos = 0;
	int ypos = 0;
	int i,j,n;
	uint16_t total_bytes_read = 0;
	uint8_t i_max = 0;
	uint8_t _bpp = 0;
	uint8_t count_val[2] = {0,0};//Get mode Get data

	if (bpp == 8)
		_bpp = 3;
	if (bpp == 4)
		_bpp = 2;

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
					uint16_t bar = (8 - (i << _bpp));
					drawpixel(xpos, height - ypos, (count_val[1] & (((1<<bpp)-1) << bar )) >> bar);
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
					drawpixel(xpos, height - ypos, (c >> (8-(i<<_bpp))) & ((1<<bpp)-1));
					i++;
					xpos++;
                }
            }
			// absolute mode runs are padded to 16-bit alignment
			if (total_bytes_read & 1)
				fseek(fp, 1L, SEEK_CUR);
		}
		// Line end
		if ((count_val[0] == 0) && (count_val[1] == 0))
		{
			ypos++;
			xpos = 0;
		}
		// Bitmap end
		if ((count_val[0] == 0) && (count_val[1] == 1))
			break;
		// Deltarecord. I did not find any BMP using this
        if ((count_val[0] == 0) && (count_val[1] == 2))
		{
			count_val[0] = fgetc(fp);
			count_val[1] = fgetc(fp);
			xpos += count_val[0]; ypos += count_val[1];
        }
    }
}


int bmp_load_and_display(const char *filename, int graph_mode)
{
	uint16_t header;
	uint16_t dib_header_size;
	uint16_t width, height, num_colors;
	uint8_t pixel_format;
	uint8_t rle;
	uint8_t *line_buffer; // max 640 pixels per line on 3 byte colors
	uint16_t offset;
	int load_palette = 1; // set to 0 not to touch palette registers
	uint8_t *palette = NULL;
#if 0
	uint8_t *old_palette = NULL;
#endif

	FILE *fp = fopen(filename, "r");
	if (!fp)
		fprintf(stderr, "Error. Could not open file.\n");

		//Read header
	fread(&header, 1, 2, fp);
	if (header != 0x4D42)
		fprintf(stderr, "Not a BMP file %s\n",filename);

	uint32_t bmp_size = 0;
	fread(&bmp_size, 1, 4, fp);
	fseek(fp, 4L, SEEK_CUR);
	uint32_t file_offset = 0;
	fread(&file_offset, 1, 4, fp);

	fread(&dib_header_size, 1, 2, fp );
	fseek(fp, 2L, SEEK_CUR);
	fread(&width, 1 ,2, fp);
	fseek(fp, 2L, SEEK_CUR);
	fread(&height, 1 ,2, fp);
	fseek(fp, 4L, SEEK_CUR);
	fread(&pixel_format, 1, 1, fp);
	fseek(fp, 1L, SEEK_CUR);
	fread(&rle, 1,  1, fp);	//0 none, 1 = 8 bit, 2 = 4 bit

	fseek(fp, 15L,SEEK_CUR);
	fread(&num_colors, 1, 2, fp);

	if (num_colors == 0)
		num_colors = 1 << pixel_format;

	//Advance to palette data
	dib_header_size -=34;
	fseek(fp, (long int) dib_header_size, SEEK_CUR);

#ifdef DEBUG
	fprintf(stderr, "dib_header_size: %hu\n", dib_header_size);
	fprintf(stderr, "width: %hu\n", width);
	fprintf(stderr, "height: %hu\n", height);
	fprintf(stderr, "pixel format: %hu\n", pixel_format);
	fprintf(stderr, "run-length encoding: %hu\n", rle);
	fprintf(stderr, "num colors: %hu\n", num_colors);
	fprintf(stderr, "offset: %u\n", file_offset);

#endif

	uint16_t line_size;
	if (pixel_format == 1)
		line_size = width >> 3;
	if (pixel_format == 4)
		line_size = width >> 1;
	if (pixel_format >= 8)
		line_size = width * (pixel_format >> 3);

	printf("line size: %d\n", line_size);
	int pad_size = (line_size) % 4;

	// load palette
	if (pixel_format <= 8)
	{
		if (load_palette)
		{
			uint8_t col[4];

#if 0 // disabled code for saving and restoring the VGA palette - this is not needed as mode change triggers palette cleanup
			uint16_t offsetop = 0;
			printf("Saving current VGA palette.\n");
			old_palette = malloc(num_colors * 3);
			for(int i = 0; i < num_colors; i++)
			{
				get_palette(col, col+1, col+2, i);
				old_palette[offsetop++] = col[0]; // red
				old_palette[offsetop++] = col[1]; // green
				old_palette[offsetop++] = col[2]; // blue

			}
#endif

#ifdef DEBUG
			printf("Palette: \n");
#endif

			uint16_t offsetp = 0;
			palette = malloc(num_colors * 3);
			for(int i = 0; i < num_colors; i++)
			{
				fread(&col, 1, 4, fp);
				//  blue, green, red, 0x00
#ifdef DEBUG
				printf("%hhu %hhu %hhu\n",col[2], col[1], col[0]);
#endif
				// we just set the palette in hardware if we are using if input image matches the gfx mode
				if ( (pixel_format == 8 && graph_mode == VIDEO_MODE_13) ||
					 (pixel_format == 4 && (graph_mode == VIDEO_MODE_12 || graph_mode == VIDEO_MODE_10)) )
					set_palette(col[2], col[1], col[0], i);

				palette[offsetp++] = col[2];
				palette[offsetp++] = col[1];
				palette[offsetp++] = col[0];
			}
		}
		else
		{
			fseek(fp, (long int) num_colors << 2, 1);
		}

		if (pixel_format == 1)
		{
			line_buffer = malloc(line_size);

			for(int i = height - 1; i >= 0; i--)
			{
				fread(line_buffer, 1, line_size, fp);
				if (pad_size)
					fseek(fp, (long) pad_size, SEEK_CUR);

				// TODO: may be we could verify if we are not printing more pixels than width?
				for (int j = 0; j < line_size; j++)
				{
					int x_off = j << 3;
					// TODO: we need to convert using the provided palette!!
					for (int offst = 7; offst >= 0; offst--)
						drawpixel(x_off + offst, i, (line_buffer[j] >> (7 - offst)) & 1);
				}
			}
			free(line_buffer);
		}
		if (pixel_format == 4)
		{
			if (rle)
			{
				decompress_RLE_BMP(fp,pixel_format,width,height,0);
			}
			else
			{
				line_buffer = malloc(line_size);
				for(int i = height - 1; i >= 0; i--)
				{
					fread(line_buffer, 1, line_size, fp);
					if (pad_size)
						fseek(fp, (long) pad_size, SEEK_CUR);

					// TODO: we need to convert using the provided palette!!
					for (int j = 0; j < line_size; j++)
					{
						drawpixel(j<<1, i, line_buffer[j] >> 4);
						drawpixel((j<<1)+1, i, line_buffer[j] & 0x0F);
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
				line_buffer = malloc(line_size);

				for(int i = height - 1; i >=0; i--)
				{
					fread(line_buffer, 1, line_size, fp);
					if (pad_size)
						fseek(fp, (long) pad_size, SEEK_CUR);

					offset = 0;
					for (int j = 0; j < line_size; j++)
					{
						if (graph_mode == VIDEO_MODE_13)
							drawpixel(j, i, line_buffer[j]);
						else
						{
							uint16_t coltmp = (uint16_t) line_buffer[j] * 3;
							drawpixel(j, i, rgb_to_vga16_fast(palette[coltmp], palette[coltmp + 1], palette[coltmp + 2]));
						}
					}
				}
				free(line_buffer);

			}
		}
	}
	if (pixel_format == 24)
	{
		line_buffer = malloc(line_size);

		// now load an optimized pallet for RGB to 8-bit conversion
		if (graph_mode == VIDEO_MODE_13)
			load_palette1(VIDEO_MODE_13);

		if (graph_mode == VIDEO_MODE_12 || graph_mode == VIDEO_MODE_10)
			load_palette1_4bit(VIDEO_MODE_12);

		for(int i = height - 1; i >= 0; i--)
		{
			offset = 0;
			fread(line_buffer, 1, line_size, fp);
			if (pad_size)
				fseek(fp, (long) pad_size, SEEK_CUR);

			for (int j = 0; j < width; j++)
			{
				uint8_t pixel;
				if (graph_mode == VIDEO_MODE_13)
				{
					pixel = rgb2palette1(line_buffer[offset+2], line_buffer[offset+1], line_buffer[offset]); // blue, green and red
				}
				else
				{
					pixel = rgb_to_vga16_fast(line_buffer[offset+2], line_buffer[offset+1], line_buffer[offset]);
				}
				drawpixel(j, i, pixel);
				offset += 3;
			}
		}
		free(line_buffer);
	}

	if (pixel_format == 16 || pixel_format == 32)
	{
		printf("Pixel Format %hhu: Not Yet Supported!\n",  pixel_format);
		return -1;
	}


#if 0 // we are not saving nor restoring the old palette
	if (pixel_format <= 8 && load_palette && old_palette)
	{
		printf("Restoring palette.\n");
		uint16_t offsetp = 0;
		for(int i = 0; i < num_colors; i++)
		{
			set_palette(old_palette[offsetp], old_palette[offsetp+1], old_palette[offsetp+2], i);
			offsetp += 3;
		}
		free(old_palette);
	}
#endif

	return 0;

}

int main(int argc, char *argv[])
{
	char *filename;
	uint16_t mode_wanted = VIDEO_MODE_13;

	printf("ELKS BMP Viewer v0.2\n");

	if (parse_args(argc, argv, &filename, &mode_wanted))
		return EXIT_FAILURE;

	mode = get_mode();

#ifndef __C86__
	if (signal(SIGINT, sig_handler) == SIG_ERR)
		printf("\ncan't catch SIGINT\n");
#endif

	printf("Source File:               \"%s\"\n", filename);
	printf("Current Graphics Mode:     \"0x%hx\"\n", mode);
	printf("Selected Graphics Mode:    \"0x%hx\"\n\n", mode_wanted);
	printf("Press any key to diplay the image.\n");
	printf("Then press any key to exit!\n");
	getchar();

	set_mode(mode_wanted);

#ifdef DEBUG
	printf("Palette before: \n");
	for (uint16_t i = 0; i < 256; i++)
	{
		uint8_t red, green, blue;
		get_palette(&red, &green, &blue, i);
		printf("%hhu %hhu %hhu\n", red, green, blue);
	}
#endif

	int ret = bmp_load_and_display(filename, mode_wanted);

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
	set_mode(mode);

	if (ret != 0)
		fprintf(stderr, "Error reading bmp file.\n");

	return ret;
}
