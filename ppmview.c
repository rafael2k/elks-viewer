// ELKS PPM Viewer

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

extern void *malloc(size_t size);
extern void *calloc(size_t nmemb, size_t size);
extern void free(void *ptr);


#define  MAXLINE         512

void sig_handler(int signo)
{
	if (signo == SIGINT)
		printf("received SIGINT\n");

	set_mode(TEXT_MODE_3);
}

int isgraph(int c)
{
  return ((c > 0x20) && (c <= 0x7E));
}

static int print_usage()
{
   printf("Usage: ppmview [source_file.ppm]\n");
   printf("source_file: PPM file to decode.\n");
   printf("\n");
   printf("Displays a ppm image in the screen.\n");
   printf("\n");
   return EXIT_FAILURE;
}

int ppm_load_and_display(const char *pFilename, int mode)
{
	unsigned int width, height, maxcolors_val;
	unsigned int i;
	char magic[MAXLINE];
	char line[MAXLINE];
	int count=0;
	int is_ascii = 0;
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
			if (isgraph(line[i]) && (flag == 0)) {
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

	if (strcmp(magic, "P3") == 0) {
		is_ascii = 1;
	} else if (strcmp(magic, "P6") == 0) {
		is_ascii = 0;
	} else {
		fprintf(stderr, "Error: Input file not in PPM format!\n");
		return -1;
	}
	int c, r_val, g_val, b_val;
	int j = 0;
	uint32_t offset = 0;

	uint16_t line_size = width * 3;
	line_buffer = malloc(line_size);

	// read now a whole line before writing to screen
	for(i = 0; i < height; i++)
	{
		if ((c = fgetc(f)) != EOF)
			ungetc(c, f);
		else
		{
			fprintf(stderr, "Premature end-of-file?\n");
			break;
		}

		if (is_ascii == 1) // ascii mode
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
				uint8_t pixel = rgb2vga(line_buffer[offset], line_buffer[offset + 1], line_buffer[offset + 2]);
				plot_pixel(j, i, pixel);
				offset += 3;
			}

		}
		else // binary mode
		{
			offset = 0;
			fread(line_buffer, 1, line_size, f);

			for (j = 0; j < width; j++)
			{
				uint8_t pixel = rgb2vga(line_buffer[offset], line_buffer[offset + 1], line_buffer[offset + 2]);
				plot_pixel(j, i, pixel);
				offset += 3;
			}
		}
	}

	free(line_buffer);

    return 0;
}

//------------------------------------------------------------------------------
int main(int arg_c, char *arg_v[])
{
   int n = 1;
   const char *filename;

   printf("ELKS PPM Viewer v0.1\n");

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

   sleep(1);
   set_mode(VIDEO_MODE_13);

#if 0
   // prints the current palette
   for (uint16_t i = 0; i < 256; i++)
   {
	   uint8_t red, green, blue;
	   get_palette(&red, &green, &blue, i);
	   printf("%hhu %hhu %hhu\n", red, green, blue);
   }
#endif

   int ret = ppm_load_and_display(filename, VIDEO_MODE_13);

   getchar();
   set_mode(TEXT_MODE_3);

   if (ret != 0)
	   fprintf(stderr, "Error reading ppm file.\n");

   return ret;

   return EXIT_SUCCESS;
}
//------------------------------------------------------------------------------

