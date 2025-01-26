// ELKS image viewer

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>
#include <assert.h>

#include "graphics.h"

extern void *malloc(size_t size);
extern void *calloc(size_t nmemb, size_t size);
extern void free(void *ptr);


#define  MAXLINE         512

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
	int x_val, y_val, maxcolors_val;
	unsigned int i;
	char magic[MAXLINE];
	char line[MAXLINE];
	int count=0;
	int is_ascii = 0;

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
				count += sscanf(line, "%2s %d %d %d", magic, &x_val, &y_val, &maxcolors_val);
			} else if (count == 1) {
				count += sscanf(line, "%d %d %d", &x_val, &y_val, &maxcolors_val);
			} else if (count == 2) {
				count += sscanf(line, "%d %d", &y_val, &maxcolors_val);
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
	int c;
	int r_val, g_val, b_val;

	/* Read the rest of the PPM file. */
	i = 0;
	int j = 0, k = 0;
	while ((c = fgetc(f)) != EOF) {
		ungetc(c, f);
		if (is_ascii == 1) {
			if (fscanf(f, "%d %d %d", &r_val, &g_val, &b_val) != 3)
				return -1;
		} else {
			r_val = fgetc(f);
			g_val = fgetc(f);
			b_val = fgetc(f);
		}

		uint8_t pixel = rgb2vga(r_val, g_val, b_val);
		plot_pixel(k, j, pixel);
		k++;
		if (k == x_val)
		{
			k = 0;
			j++;
		}
	}
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

   printf("Source file:      \"%s\"\n", filename);

   sleep(1);
   set_mode(VIDEO_MODE_13);

   int ret = ppm_load_and_display(filename, VIDEO_MODE_13);
   sleep(10);
   set_mode(TEXT_MODE_3);

   if (ret != 0)
	   fprintf(stderr, "Error reading ppm file.\n");

   return ret;

   return EXIT_SUCCESS;
}
//------------------------------------------------------------------------------

