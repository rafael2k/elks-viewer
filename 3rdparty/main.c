#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_PALETTE_SIZE 5

int verbose = 0;

typedef struct {
  uint8_t r, g, b;
} Color;

float calculate_color_distance(Color a, Color b) {
  return sqrtf((a.r - b.r) * (a.r - b.r) + (a.g - b.g) * (a.g - b.g) +
               (a.b - b.b) * (a.b - b.b));
}

int assign_to_nearest_cluster(Color pixel, Color *centroids, int num_clusters) {
  int best_cluster = 0;
  float min_distance = calculate_color_distance(pixel, centroids[0]);

  for (int i = 1; i < num_clusters; i++) {
    float distance = calculate_color_distance(pixel, centroids[i]);
    if (distance < min_distance) {
      min_distance = distance;
      best_cluster = i;
    }
  }
  return best_cluster;
}

void convert_color_to_hex(Color color, char *hex) {
  sprintf(hex, "#%02X%02X%02X", color.r, color.g, color.b);
}

void generate_color_palette(uint8_t *pixels, int width, int height,
                            int channels, Color *palette, int palette_size) {
  if (!palette || !pixels) {
    printf("Error: Invalid inputs for palette generation.\n");
    return;
  }

  for (int i = 0; i < palette_size; i++) {
    int index = (rand() % (width * height)) * channels;
    palette[i].r = pixels[index];
    palette[i].g = pixels[index + 1];
    palette[i].b = pixels[index + 2];
  }

  int *assignments = (int *)malloc(width * height * sizeof(int));
  if (!assignments) {
    printf("Error: Memory allocation failed for assignments.\n");
    return;
  }

  for (int iter = 0; iter < 10; iter++) {
    if (verbose) {
      printf("Iteration %d\n", iter + 1);
    }

    for (int i = 0; i < width * height; i++) {
      Color pixel = {pixels[i * channels], pixels[i * channels + 1],
                     pixels[i * channels + 2]};
      assignments[i] = assign_to_nearest_cluster(pixel, palette, palette_size);
    }

    int *counts = (int *)calloc(palette_size, sizeof(int));
    uint64_t *sum_r = (uint64_t *)calloc(palette_size, sizeof(uint64_t));
    uint64_t *sum_g = (uint64_t *)calloc(palette_size, sizeof(uint64_t));
    uint64_t *sum_b = (uint64_t *)calloc(palette_size, sizeof(uint64_t));

    if (!counts || !sum_r || !sum_g || !sum_b) {
      printf("Error: Memory allocation failed during clustering.\n");
      free(assignments);
      free(counts);
      free(sum_r);
      free(sum_g);
      free(sum_b);
      return;
    }

    for (int i = 0; i < width * height; i++) {
      int cluster = assignments[i];
      counts[cluster]++;
      sum_r[cluster] += pixels[i * channels];
      sum_g[cluster] += pixels[i * channels + 1];
      sum_b[cluster] += pixels[i * channels + 2];
    }

    for (int i = 0; i < palette_size; i++) {
      if (counts[i] > 0) {
        palette[i].r = sum_r[i] / counts[i];
        palette[i].g = sum_g[i] / counts[i];
        palette[i].b = sum_b[i] / counts[i];
      }
    }

    free(counts);
    free(sum_r);
    free(sum_g);
    free(sum_b);

    if (verbose) {
      printf("Updated Centroids:\n");
      for (int i = 0; i < palette_size; i++) {
        char hex[8];
        convert_color_to_hex(palette[i], hex);
        printf("Color %d: %s\n", i + 1, hex);
      }
    }
  }

  free(assignments);
}

int printUsage() {
  printf("Usage: cpig image_path [options]\n");
  printf("Options:\n");
  printf("  -c, --colors   Number of colors in the palette (default: %d)\n",
         DEFAULT_PALETTE_SIZE);
  printf("  -v, --verbose  Enable verbose output\n");
  printf("  -h, --help     Print this help message\n");
  return 1;
}

int main(int argc, char *argv[]) {
  char image_file[512] = {0};
  int palette_size = DEFAULT_PALETTE_SIZE;

  if (argc < 2 || strcmp(argv[1], "-h") == 0 ||
      strcmp(argv[1], "--help") == 0) {
    return printUsage();
  }

  strcpy(image_file, argv[1]);

  for (int i = 2; i < argc; i++) {
    if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--colors") == 0) {
      if (i + 1 >= argc) {
        printf("Error: Missing argument for option %s\n", argv[i]);
        return printUsage();
      }
      palette_size = atoi(argv[i + 1]);
      i++;
    } else if (strcmp(argv[i], "-v") == 0 ||
               strcmp(argv[i], "--verbose") == 0) {
      verbose = 1;
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      return printUsage();
    } else {
      printf("Error: Unknown option: %s\n", argv[i]);
      return printUsage();
    }
  }

  int width, height, channels;
  uint8_t *image_data = stbi_load(image_file, &width, &height, &channels, 0);
  if (!image_data) {
    printf("Error: Failed to load image: %s\n", image_file);
    return 1;
  }

  if (verbose) {
    printf("Image: %s\n", image_file);
    printf("Dimensions: %dx%d, Channels: %d\n", width, height, channels);
  }

  Color *palette = (Color *)malloc(palette_size * sizeof(Color));
  if (!palette) {
    printf("Error: Memory allocation failed for palette.\n");
    stbi_image_free(image_data);
    return 1;
  }

  generate_color_palette(image_data, width, height, channels, palette,
                         palette_size);

  for (int i = 0; i < palette_size; i++) {
    char hex[8];
    convert_color_to_hex(palette[i], hex);
    printf("%s\n", hex);
  }

  free(palette);
  stbi_image_free(image_data);
  return 0;
}
