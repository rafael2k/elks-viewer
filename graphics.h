/*  ELKS Viewer Graphics Routines
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

#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include <stdint.h>

#define TEXT_MODE_0         0x00      /* 40x25 B/W text mode. */
#define TEXT_MODE_1         0x01      /* 40x25 4-bit color text mode. */
#define TEXT_MODE_2         0x02      /* 80x25 4-bit shades of gray text mode. */
#define TEXT_MODE_3         0x03      /* 80x25 4-bit color text mode. */
#define VIDEO_MODE_5        0x05      /* 320x200 2-bit / CGA B800   */
#define VIDEO_MODE_6        0x06      /* 640x200 1-bit / CGA B800   */
#define VIDEO_MODE_D        0x0D      /* 320x200 4-bit / EGA A000   */
#define VIDEO_MODE_10       0x10      /* 640x350 4-bit / EGA A000   */
#define VIDEO_MODE_12       0x12      /* 640x480 4-bit / VGA A000   */
#define VIDEO_MODE_13       0x13      /* 320x200 8-bit / VGA A000   */


/*
	AL = 00  40x25 B/W text (CGA,EGA,MCGA,VGA)
	   = 01  40x25 16 color text (CGA,EGA,MCGA,VGA)
	   = 02  80x25 16 shades of gray text (CGA,EGA,MCGA,VGA)
	   = 03  80x25 16 color text (CGA,EGA,MCGA,VGA)
	   = 04  320x200 4 color graphics (CGA,EGA,MCGA,VGA)
	   = 05  320x200 4 color graphics (CGA,EGA,MCGA,VGA)
	   = 06  640x200 B/W graphics (CGA,EGA,MCGA,VGA)
	   = 07  80x25 Monochrome text (MDA,HERC,EGA,VGA)
	   = 08  160x200 16 color graphics (PCjr)
	   = 09  320x200 16 color graphics (PCjr)
	   = 0A  640x200 4 color graphics (PCjr)
	   = 0B  Reserved (EGA BIOS function 11)
	   = 0C  Reserved (EGA BIOS function 11)
	   = 0D  320x200 16 color graphics (EGA,VGA)
	   = 0E  640x200 16 color graphics (EGA,VGA)
	   = 0F  640x350 Monochrome graphics (EGA,VGA)
	   = 10  640x350 16 color graphics (EGA or VGA with 128K)
		 640x350 4 color graphics (64K EGA)
	   = 11  640x480 B/W graphics (MCGA,VGA)
	   = 12  640x480 16 color graphics (VGA)
	   = 13  320x200 256 color graphics (MCGA,VGA)
*/

extern uint16_t width_internal, heigth_internal, mode_internal;


void drawpixel(int x,int y, uint8_t color);
void set_mode(uint8_t mode_set);
uint16_t get_mode();
void set_palette(uint8_t red, uint8_t green, uint8_t blue, uint16_t idx);
void get_palette(uint8_t *red, uint8_t *green, uint8_t *blue, uint16_t idx);

void load_palette1(uint8_t mode_set);
void load_palette1g(uint8_t mode_set);

uint8_t rgb2palette1(uint8_t r, uint8_t g, uint8_t b);

#ifdef __C86__
// init 4-bit color routines in assembly
void vga_init(void);
void vga_drawpixel(int x, int y, int c);
void vga_drawhline(int x1, int x2, int y, int c);
void vga_drawvline(int x, int y1, int y2, int c);
int vga_readpixel(int x, int y);
#endif

#if 0
// this is very slow - don't use it
uint8_t rgb2vga(int r, int g, int b);
#endif

#endif // GRAPHICS_H_
