// ELKS BMP Viewer
//
//
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>

#include "graphics.h"

#define DEBUG

int bmp_load_and_display(const char *filename, int mode)
{
	uint16_t header;
	uint16_t dib_header_size;
	uint16_t width, height, num_colors;
	uint8_t pixel_format;
	uint8_t rle;
	uint8_t palette[768]; // we not using it yet...
	uint8_t *line_buffer; // max 640 pixels per line on 3 byte colors


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
		int load_palette = 0;
		if (load_palette)
		{
			for(int i = 0; i < num_colors; i++)
			{
				uint8_t col[3];
				fread(&col, 1, 3, fp);
				palette[(int)(i*3+2)] = col[0] >> 2;
				palette[(int)(i*3+1)] = col[1] >> 2;
				palette[(int)(i*3+0)] = col[2] >> 2;
			}
		}
		else
		{
			fseek(fp,3,1);
		}
		fseek(fp,1,1);

		fprintf(stderr, "ftell %ld\n", ftell(fp));
		if (pixel_format == 4)
		{
			if (rle)
			{
				// decompress_RLE_BMP(fp,pixel_format,width,height,0);
			}
			else
			{
				uint16_t line_size = width >> 1;
				line_buffer = malloc(line_size);
				for(int i = 0; i < height; i++)
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
				// decompress_RLE_BMP(fp,pixel_format,width,height,0);
			}
			else
			{
				uint16_t line_size = width;
				line_buffer = malloc(line_size);

				for(int i = 0; i < height; i++)
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
			// TODO
		}
	}

	if (pixel_format == 24)
	{
		fprintf(stderr, "ftell %ld\n", ftell(fp));
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

	return 0;

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

   printf("Source file:      \"%s\"\n", filename);

   sleep(1);
   set_mode(VIDEO_MODE_13);

   int ret = bmp_load_and_display(filename, VIDEO_MODE_13);
   sleep(10);
   set_mode(TEXT_MODE_3);

   if (ret != 0)
	   fprintf(stderr, "Error reading bmp file.\n");

   return ret;

   return EXIT_SUCCESS;
}
