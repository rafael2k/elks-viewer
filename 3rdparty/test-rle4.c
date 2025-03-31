#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#pragma pack(push, 1)
typedef struct {
    uint16_t type;              // Magic identifier: 0x4d42
    uint32_t size;              // File size in bytes
    uint16_t reserved1;         // Not used
    uint16_t reserved2;         // Not used
    uint32_t offset;            // Offset to image data in bytes from beginning of file
} BMPHeader;

typedef struct {
    uint32_t size;              // Header size in bytes
    int32_t width;              // Width of the image
    int32_t height;             // Height of image
    uint16_t planes;            // Number of color planes
    uint16_t bits;              // Bits per pixel
    uint32_t compression;       // Compression type
    uint32_t imagesize;         // Image size in bytes
    int32_t xresolution;        // Pixels per meter
    int32_t yresolution;        // Pixels per meter
    uint32_t ncolors;           // Number of colors
    uint32_t importantcolors;   // Important colors
} DIBHeader;

typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t reserved;
} RGBQuad;
#pragma pack(pop)

uint8_t* encode_rle4(const uint8_t* data, int width, int height, int* encoded_size) {
    // Calculate worst-case scenario for buffer size
    int max_size = width * height * 2 + height * 2 + 2; // Each pixel could take 2 bytes, plus EOL markers, plus EOF
    uint8_t* encoded = malloc(max_size);
    if (!encoded) return NULL;
    
    int pos = 0;
    int total_pixels = width * height;
    int i = 0;
    int row_start = 0;
    
    while (i < total_pixels) {
        // Check for end of line
        if (i > 0 && (i % width) == 0) {
            encoded[pos++] = 0; // Escape
            encoded[pos++] = 0; // End of line
            row_start = i;
        }
        
        // Find run of identical pixels
        int run_length = 1;
        while (i + run_length < total_pixels && 
               run_length < 255 && 
               (i + run_length) % width != 0 && // Don't cross row boundary
               data[i + run_length] == data[i]) {
            run_length++;
        }
        
        if (run_length >= 2) {
            // Encode run
            encoded[pos++] = (uint8_t)run_length;
            encoded[pos++] = (data[i] << 4) | data[i];
            i += run_length;
        } else {
            // Find literal sequence (up to end of line or 254 pixels)
            int literal_length = 1;
            while (i + literal_length < total_pixels &&
                   literal_length < 254 &&
                   (i + literal_length) % width != 0 && // Don't cross row boundary
                   (literal_length == 1 || data[i + literal_length] != data[i + literal_length - 1])) {
                literal_length++;
            }
            
            // If we stopped because of a repeat, back up
            if (literal_length > 1 && data[i + literal_length - 1] == data[i + literal_length - 2]) {
                literal_length--;
            }
            
            // Encode literal
            encoded[pos++] = 0; // Escape
            encoded[pos++] = (uint8_t)literal_length;
            
            // Pack two 4-bit pixels per byte
            int j;
            for (j = 0; j < literal_length; j++) {
                if (j % 2 == 0) {
                    encoded[pos] = data[i + j] & 0x0F;
                } else {
                    encoded[pos++] |= (data[i + j] & 0x0F) << 4;
                }
            }
            
            // If odd number of pixels, complete the last byte
            if (literal_length % 2 != 0) {
                pos++;
            }
            
            i += literal_length;
        }
    }
    
    // End of bitmap marker
    encoded[pos++] = 0; // Escape
    encoded[pos++] = 1; // End of bitmap
    
    *encoded_size = pos;
    return encoded;
}

int write_bmp_rle4(const char* filename, const uint8_t* pixels, int width, int height, const RGBQuad* palette) {
    // BMP is stored bottom-to-top, so we need to flip the image vertically
    uint8_t* flipped_pixels = malloc(width * height);
    if (!flipped_pixels) return 0;
    
    for (int y = 0; y < height; y++) {
        memcpy(flipped_pixels + (height - 1 - y) * width, 
               pixels + y * width, 
               width);
    }
    
    // Encode pixel data
    int encoded_size;
    uint8_t* encoded_pixels = encode_rle4(flipped_pixels, width, height, &encoded_size);
    free(flipped_pixels);
    
    if (!encoded_pixels) return 0;
    
    // Calculate file size
    uint32_t palette_size = 16 * sizeof(RGBQuad);
    uint32_t offset = sizeof(BMPHeader) + sizeof(DIBHeader) + palette_size;
    uint32_t file_size = offset + encoded_size;
    
    // Create headers
    BMPHeader bmp_header = {
        .type = 0x4D42,
        .size = file_size,
        .reserved1 = 0,
        .reserved2 = 0,
        .offset = offset
    };
    
    DIBHeader dib_header = {
        .size = sizeof(DIBHeader),
        .width = width,
        .height = height,
        .planes = 1,
        .bits = 4,
        .compression = 2, // RLE4
        .imagesize = encoded_size,
        .xresolution = 0,
        .yresolution = 0,
        .ncolors = 16,
        .importantcolors = 16
    };
    
    // Write to file
    FILE* file = fopen(filename, "wb");
    if (!file) {
        free(encoded_pixels);
        return 0;
    }
    
    fwrite(&bmp_header, sizeof(BMPHeader), 1, file);
    fwrite(&dib_header, sizeof(DIBHeader), 1, file);
    fwrite(palette, sizeof(RGBQuad), 16, file);
    fwrite(encoded_pixels, 1, encoded_size, file);
    
    fclose(file);
    free(encoded_pixels);
    return 1;
}

// Example usage with a better test pattern
int main() {
    const int width = 8;
    const int height = 8;
    uint8_t pixels[width * height];
    
    // Create a more distinct test pattern
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if ((x + y) % 2 == 0) {
                pixels[y * width + x] = x % 8; // Vary the color based on x position
            } else {
                pixels[y * width + x] = (x + y) % 8 + 8; // Different set of colors
            }
        }
    }
    
    // More varied palette
    RGBQuad palette[16] = {
        {0, 0, 0},       // 0: Black
        {0, 0, 255},     // 1: Blue
        {0, 255, 0},     // 2: Green
        {0, 255, 255},   // 3: Cyan
        {255, 0, 0},     // 4: Red
        {255, 0, 255},   // 5: Magenta
        {255, 255, 0},   // 6: Yellow
        {255, 255, 255}, // 7: White
        {128, 0, 0},     // 8: Dark Red
        {0, 128, 0},     // 9: Dark Green
        {0, 0, 128},     // 10: Dark Blue
        {128, 128, 0},   // 11: Olive
        {128, 0, 128},   // 12: Purple
        {0, 128, 128},   // 13: Teal
        {192, 192, 192}, // 14: Light Gray
        {64, 64, 64}     // 15: Dark Gray
    };
    
    if (write_bmp_rle4("output_rle4.bmp", pixels, width, height, palette)) {
        printf("RLE4 BMP created successfully!\n");
        return 0;
    } else {
        printf("Failed to create BMP file\n");
        return 1;
    }
}


#if 0
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#pragma pack(push, 1)
typedef struct {
    uint16_t type;              // Magic identifier: 0x4d42
    uint32_t size;              // File size in bytes
    uint16_t reserved1;         // Not used
    uint16_t reserved2;         // Not used
    uint32_t offset;            // Offset to image data in bytes from beginning of file
} BMPHeader;

typedef struct {
    uint32_t size;              // Header size in bytes
    int32_t width;              // Width of the image
    int32_t height;             // Height of image
    uint16_t planes;            // Number of color planes
    uint16_t bits;              // Bits per pixel
    uint32_t compression;       // Compression type
    uint32_t imagesize;         // Image size in bytes
    int32_t xresolution;        // Pixels per meter
    int32_t yresolution;        // Pixels per meter
    uint32_t ncolors;           // Number of colors
    uint32_t importantcolors;   // Important colors
} DIBHeader;

typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t reserved;
} RGBQuad;
#pragma pack(pop)

uint8_t* encode_rle4(const uint8_t* data, int width, int height, int* encoded_size) {
    int capacity = width * height * 2; // Start with generous estimate
    uint8_t* encoded = malloc(capacity);
    if (!encoded) return NULL;
    
    int pos = 0;
    int total_pixels = width * height;
    int i = 0;
    
    while (i < total_pixels) {
        // Check if we need to realloc
        if (pos + 4 > capacity) {
            capacity *= 2;
            uint8_t* new_encoded = realloc(encoded, capacity);
            if (!new_encoded) {
                free(encoded);
                return NULL;
            }
            encoded = new_encoded;
        }
        
        // Find run of identical pixels
        int run_length = 1;
        while (i + run_length < total_pixels && 
               run_length < 255 && 
               data[i + run_length] == data[i]) {
            run_length++;
        }
        
        if (run_length >= 2) {
            // Encode run
            encoded[pos++] = (uint8_t)run_length;
            encoded[pos++] = (data[i] << 4) | data[i];
            i += run_length;
        } else {
            // Find literal sequence
            int literal_start = i;
            int literal_length = 0;
            
            while (i + literal_length < total_pixels && 
                   literal_length < 254 && 
                   (i + literal_length + 1 >= total_pixels || 
                    data[i + literal_length] != data[i + literal_length + 1])) {
                literal_length++;
            }
            
            if (literal_length > 0) {
                // Encode literal
                encoded[pos++] = 0; // Escape
                encoded[pos++] = (uint8_t)literal_length;
                
                // Pack two 4-bit pixels per byte
                for (int j = 0; j < literal_length; j += 2) {
                    uint8_t byte;
                    if (j + 1 < literal_length) {
                        byte = (data[i + j] & 0x0F) | ((data[i + j + 1] & 0x0F) << 4);
                    } else {
                        byte = data[i + j] & 0x0F; // Last pixel if odd count
                    }
                    encoded[pos++] = byte;
                }
                
                // Pad to even number of bytes if odd literal length
                if (literal_length % 2 != 0) {
                    encoded[pos++] = 0;
                }
                
                i += literal_length;
            }
        }
        
        // End of line marker
        if (i % width == 0 && i > 0) {
            if (pos + 2 > capacity) {
                capacity += 2;
                uint8_t* new_encoded = realloc(encoded, capacity);
                if (!new_encoded) {
                    free(encoded);
                    return NULL;
                }
                encoded = new_encoded;
            }
            encoded[pos++] = 0; // Escape
            encoded[pos++] = 0; // End of line
        }
    }
    
    // End of bitmap marker
    if (pos + 2 > capacity) {
        capacity += 2;
        uint8_t* new_encoded = realloc(encoded, capacity);
        if (!new_encoded) {
            free(encoded);
            return NULL;
        }
        encoded = new_encoded;
    }
    encoded[pos++] = 0; // Escape
    encoded[pos++] = 1; // End of bitmap
    
    *encoded_size = pos;
    return encoded;
}

int write_bmp_rle4(const char* filename, const uint8_t* pixels, int width, int height, const RGBQuad* palette) {
    // Encode pixel data
    int encoded_size;
    uint8_t* encoded_pixels = encode_rle4(pixels, width, height, &encoded_size);
    if (!encoded_pixels) return 0;
    
    // Calculate file size
    uint32_t palette_size = 16 * sizeof(RGBQuad);
    uint32_t offset = sizeof(BMPHeader) + sizeof(DIBHeader) + palette_size;
    uint32_t file_size = offset + encoded_size;
    
    // Create headers
    BMPHeader bmp_header = {
        .type = 0x4D42,
        .size = file_size,
        .reserved1 = 0,
        .reserved2 = 0,
        .offset = offset
    };
    
    DIBHeader dib_header = {
        .size = sizeof(DIBHeader),
        .width = width,
        .height = height,
        .planes = 1,
        .bits = 4,
        .compression = 2, // RLE4
        .imagesize = encoded_size,
        .xresolution = 0,
        .yresolution = 0,
        .ncolors = 16,
        .importantcolors = 16
    };
    
    // Write to file
    FILE* file = fopen(filename, "wb");
    if (!file) {
        free(encoded_pixels);
        return 0;
    }
    
    fwrite(&bmp_header, sizeof(BMPHeader), 1, file);
    fwrite(&dib_header, sizeof(DIBHeader), 1, file);
    fwrite(palette, sizeof(RGBQuad), 16, file);
    fwrite(encoded_pixels, 1, encoded_size, file);
    
    fclose(file);
    free(encoded_pixels);
    return 1;
}

// Example usage
int main() {
    // Example 8x8 image with 4-bit pixels (0-15)
    const int width = 8;
    const int height = 8;
    uint8_t pixels[] = {
        0, 0, 0, 0, 1, 1, 1, 1,
        0, 2, 2, 0, 1, 3, 3, 1,
        0, 2, 2, 0, 1, 3, 3, 1,
        0, 0, 0, 0, 1, 1, 1, 1,
        4, 4, 4, 4, 5, 5, 5, 5,
        4, 6, 6, 4, 5, 7, 7, 5,
        4, 6, 6, 4, 5, 7, 7, 5,
        4, 4, 4, 4, 5, 5, 5, 5
    };
    
    // Simple 16-color palette (B, G, R)
    RGBQuad palette[16] = {
        {0, 0, 0}, {0, 0, 255}, {0, 255, 0}, {255, 0, 0},
        {0, 255, 255}, {255, 0, 255}, {255, 255, 0}, {255, 255, 255},
        {0, 0, 128}, {0, 128, 0}, {128, 0, 0}, {0, 128, 128},
        {128, 0, 128}, {128, 128, 0}, {128, 128, 128}, {192, 192, 192}
    };
    
    if (write_bmp_rle4("output_rle4.bmp", pixels, width, height, palette)) {
        printf("RLE4 BMP created successfully!\n");
        return 0;
    } else {
        printf("Failed to create BMP file\n");
        return 1;
    }
}
#endif
