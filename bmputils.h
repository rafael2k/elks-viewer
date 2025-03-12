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

#ifndef BMP_INTERNALS_H_
#define BMP_INTERNALS_H_

#include <stdio.h>
#include <stdint.h>

void decompress_RLE_BMP(FILE *fp, unsigned char bpp, int width, int height, int graph_mode, uint8_t palette, uint8_t *pic_palette);
int bmp_payload_parse(FILE *fp, int graph_mode, uint8_t pixel_format, uint16_t width, uint16_t height, uint8_t rle, uint8_t *palette);

#endif // BMP_INTERNALS_H_
