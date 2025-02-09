/*  ELKS PPM/PGM Viewer
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

extern void *malloc(size_t size);
extern void *calloc(size_t nmemb, size_t size);
extern void free(void *ptr);


#define  MAXLINE         512

uint16_t mode = 0;

#ifndef __C86__
void sig_handler(int signo)
{
	if (signo == SIGINT)
		printf("received SIGINT\n");

	if (mode)
		set_mode(mode);
}
#endif


int is_graph(int c)
{
  return ((c > 0x20) && (c <= 0x7E));
}

int ppm_load_and_display(const char *pFilename, int graph_mode)
{
	unsigned int width, height, maxcolors_val;
	unsigned int i;
	char magic[MAXLINE];
	char line[MAXLINE];
	int count=0;
	int is_ascii = 0;
	int is_gray = 0;
	uint8_t *line_buffer;

	FILE *f = fopen(pFilename, "rb");
	if (!f)
    {
        fprintf(stderr,"Error opening the file\n");
		return -1;
    }

	/* Read the PPM file header. */
	while (fgets(line, MAXLINE, f) != NULL) {
		int flag = 0;
		for (i = 0; i < strlen(line); i++) {
			if (is_graph(line[i]) && (flag == 0)) {
				if ((line[i] == '#') && (flag == 0)) {
					flag = 1;
				}
			}
		}
		if (flag == 0) {
			if (count == 0) {
				count += sscanf(line, "%2s %d %d %d", magic, &width, &height, &maxcolors_val);
			} else if (count == 1) {
				count += sscanf(line, "%d %d %d", &width, &height, &maxcolors_val);
			} else if (count == 2) {
				count += sscanf(line, "%d %d", &height, &maxcolors_val);
			} else if (count == 3) {
				count += sscanf(line, "%d", &maxcolors_val);
			}
		}
		if (count == 4) {
			break;
		}
	}


	if (magic[0] == 'P')
	{
		switch(magic[1])
		{
		case '2':
			is_ascii = 1;
			is_gray = 1;
			break;
		case '3':
			is_ascii = 1;
			is_gray = 0;
			break;
		case '6':
			is_ascii = 0;
			is_gray = 0;
			break;
		case '5':
		is_ascii = 0;
		is_gray = 1;
		break;
		default:
			fprintf(stderr, "Error: Input file not in PPM (P3 or P6) or PGM (P2 or P5) format!\n");
			return -1;
		}
	}
	int c, r_val, g_val, b_val;
	int j = 0;
	uint16_t offset = 0;

	uint16_t line_size;
	if (is_gray)
		line_size = width;
	else
		line_size = width * 3;

	line_buffer = malloc(line_size);

	if (is_gray)
		load_palette1g(VIDEO_MODE_13);
	else
		load_palette1(VIDEO_MODE_13);

	// read now a whole line before writing to screen
	for(i = 0; i < height; i++)
	{
		if ((c = fgetc(f)) != EOF)
			ungetc(c, f);
		else
		{
			fprintf(stderr, "Premature end-of-file.\n");
			break;
		}

		if (is_ascii == 1 && is_gray == 0) // PPM P3
		{
			offset = 0;
			for ( j = 0; j < width; j++ )
			{
				if (fscanf(f, "%d %d %d", &r_val, &g_val, &b_val) != 3)
					return -1;

				line_buffer[offset] = r_val;
				line_buffer[offset+1] = g_val;
				line_buffer[offset+2] = b_val;
				offset += 3;
			}

			offset = 0;
			for (j = 0; j < width; j++)
			{
				uint8_t pixel = rgb2palette1(line_buffer[offset], line_buffer[offset + 1], line_buffer[offset + 2]);
				drawpixel(j, i, pixel);
				offset += 3;
			}
		}
		else if (is_ascii == 0 && is_gray == 0) // PPM P6
		{
			offset = 0;
			fread(line_buffer, 1, line_size, f);

			for (j = 0; j < width; j++)
			{
				uint8_t pixel = rgb2palette1(line_buffer[offset], line_buffer[offset + 1], line_buffer[offset + 2]);
				drawpixel(j, i, pixel);
				offset += 3;
			}
		}
		else if (is_ascii == 1 && is_gray == 1) // PGM P2
		{
			for ( j = 0; j < width; j++ )
			{
				if (fscanf(f, "%d", &r_val) != 1)
					return -1;

				line_buffer[j] = r_val;
			}

			for (j = 0; j < width; j++)
			{
				drawpixel(j, i, line_buffer[j]);
			}
		}
		else if (is_ascii == 0 && is_gray == 1) // PGM P5
		{
			fread(line_buffer, 1, line_size, f);

			for (j = 0; j < width; j++)
			{
				drawpixel(j, i, line_buffer[j]);
			}
		}
	}

	free(line_buffer);

    return 0;
}

//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	char *filename;
	uint16_t mode_wanted = VIDEO_MODE_13;

	printf("ELKS PPM/PGM Viewer v0.2\n");


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
   // prints the current palette
   for (uint16_t i = 0; i < 256; i++)
   {
	   uint8_t red, green, blue;
	   get_palette(&red, &green, &blue, i);
	   printf("%hhu %hhu %hhu\n", red, green, blue);
   }
#endif

   int ret = ppm_load_and_display(filename, mode_wanted);

   getchar();
   set_mode(mode);

   if (ret != 0)
	   fprintf(stderr, "Error reading ppm file.\n");

   return ret;
}
//------------------------------------------------------------------------------

