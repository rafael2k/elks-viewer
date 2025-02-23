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
#include <signal.h>

#include "bmputils.h"
#include "graphics.h"
#include "utils.h"

// #define DEBUG

uint16_t mode = 0;

void sig_handler(int signo)
{
	if (signo == SIGINT)
		printf("received SIGINT\n");

	// revert to mode in use when software was started
	if (mode)
		set_mode(mode);
}

int bmp_load_and_display(const char *filename, int graph_mode)
{
	uint16_t header;
	uint16_t dib_header_size;
	uint16_t width, height, num_colors;
	uint8_t pixel_format;
	uint8_t rle;
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
	}

	bmp_payload_parse(fp, graph_mode, pixel_format, width, height, rle, palette);

	fclose(fp);
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

	if (signal(SIGINT, sig_handler) == SIG_ERR)
		printf("\ncan't catch SIGINT\n");

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
