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

#include <string.h>
#include <malloc.h>

#include "bmputils.h"
#include "graphics.h"

// Implementation using parts of: https://github.com/mills32/Little-Game-Engine-for-VGA-EGA
// Uncompress BMP image encoded using RLE8 or RLE4
void decompress_RLE_BMP(FILE *fp, unsigned char bpp, int width, int height, int graph_mode, uint8_t palette, uint8_t *pic_palette){
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
                    // TODO: support when graph mode is 8-bit and image is 4-bit
                    if (graph_mode == VIDEO_MODE_13)
                        drawpixel(xpos, height - ypos, (count_val[1] & (((1<<bpp)-1) << bar )) >> bar);
                    else
                    {
                        if (bpp == 8)
                        {
                            uint16_t coltmp = (count_val[1] & (((1<<bpp)-1) << bar )) >> bar;
                            coltmp *= 3;
                        
                            drawpixel(xpos, height - ypos, rgb_to_vga16_fast(pic_palette[coltmp], pic_palette[coltmp + 1], pic_palette[coltmp + 2]));
                        }
                        else
                        {
                            drawpixel(xpos, height - ypos, (count_val[1] & (((1<<bpp)-1) << bar )) >> bar);
                        }
                    }
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
                    if (graph_mode == VIDEO_MODE_13)
                        drawpixel(xpos, height - ypos, (c >> (8-(i<<_bpp))) & ((1<<bpp)-1));
                    else
                    {
                        if (bpp == 8)
                        {
                            uint16_t coltmp = (c >> (8-(i<<_bpp))) & ((1<<bpp)-1);
                            coltmp *= 3;
                            drawpixel(xpos, height - ypos, rgb_to_vga16_fast(pic_palette[coltmp], pic_palette[coltmp + 1], pic_palette[coltmp + 2]));
                        }
                        else
                        {
                            drawpixel(xpos, height - ypos, (c >> (8-(i<<_bpp))) & ((1<<bpp)-1));
                        }
                    }
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

int bmp_payload_parse(FILE *fp, int graph_mode, uint8_t pixel_format, uint16_t width, uint16_t height, uint8_t rle, uint8_t *palette)
{
    uint8_t *line_buffer = NULL; // max 640 pixels per line on 3 byte colors
    uint16_t offset;

    uint16_t line_size;
    if (pixel_format == 1)
        line_size = width >> 3;
    if (pixel_format == 4)
        line_size = width >> 1;
    if (pixel_format >= 8)
        line_size = width * (pixel_format >> 3);

    printf("line size: %d\n", line_size);
    int pad_size = (4 - ((line_size) % 4)) % 4;



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
            decompress_RLE_BMP(fp,pixel_format,width,height, graph_mode, 0, palette);
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
        if (graph_mode == VIDEO_MODE_12 || graph_mode == VIDEO_MODE_10)
            load_palette1_4bit(graph_mode);

        if (rle)
        {
            decompress_RLE_BMP(fp,pixel_format,width,height,graph_mode, 0, palette);
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
                        uint16_t coltmp = line_buffer[j];
                        coltmp *= 3;
                        drawpixel(j, i, rgb_to_vga16_fast(palette[coltmp], palette[coltmp + 1], palette[coltmp + 2]));
                    }
                }
            }
            free(line_buffer);

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
