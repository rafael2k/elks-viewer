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

#include "graphics.h"


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// #define DEBUG

#ifdef __WATCOMC__
uint8_t __far *CGA = (void __far *)0xB8000000L;        /* this points to video CGA memory. */
uint8_t __far *VGA = (void __far *)0xA0000000L;        /* this points to video VGA memory. */
#endif

#if defined(__WATCOMC__)
//    on return:
//  AH = number of screen columns
//	AL = mode currently set (see VIDEO MODES)
//	BH = current display page
uint16_t get_mode_a();
#pragma aux get_mode_a value [ax] parm [ ax ] =	\
	"mov ax,0F00h",								\
	"int 10h",									\
	modify [ ax bx ];

void set_mode_a(uint16_t mode);
#pragma aux set_mode_a parm [ax] =				\
	"int 10h",									\
	modify [ ax ];

uint16_t pal_cx, pal_dx;

void set_palette_a(uint16_t ax);
#pragma aux set_palette_a parm [ ax ] =			\
	"mov bx,ax",								\
	"mov cx,pal_cx",							\
	"mov dx,pal_dx",							\
	"mov ax, 1010h",							\
	"int 10h",									\
	modify [ ax bx cx dx ];

void get_palette_a(uint16_t bx);
#pragma aux get_palette_a parm [ bx ] =			\
	"mov bh,0",									\
	"mov ax,1015h",								\
	"int 10h",									\
	"mov pal_cx,cx",							\
	"mov pal_dx,dx",							\
	modify [ ax bx cx dx ];

#pragma aux set_palette_a parm [ ax ] =			\
	"mov bx,ax",								\
	"mov cx,pal_cx",							\
	"mov dx,pal_dx",							\
	"mov ax, 1010h",							\
	"int 10h",									\
	modify [ ax bx cx dx ];

#elif defined(__C86__)

/* use BIOS to set video mode */
static void get_mode_a(uint16_t *mode_set)
{
	asm(
        "push   bx\n"
        "push   cx\n"
		"push   dx\n"
		"push   di\n"
		"mov    ax,#0x0F00\n"
		"int    0x10\n"
		"mov    di,[bp+4]\n"
		"mov    [di],ax\n"
		"pop    di\n"
        "pop    dx\n"
        "pop    cx\n"
        "pop    bx\n"
		);

}

static void set_mode_a(uint16_t mode_set)
{
    asm(
        "push   si\n"
        "push   di\n"
        "push   ds\n"
        "push   es\n"
        "mov    ax,[bp+4]\n"    /* AH=0, AL=mode */
        "int    0x10\n"
        "pop    es\n"
        "pop    ds\n"
        "pop    di\n"
        "pop    si\n"
		);
}

/* PAL write color byte at video offset */
static void writevid(uint16_t offset, uint8_t c)
{
    asm(
        "push   ds\n"
        "push   bx\n"
        "mov    ax,#0xA000\n"
        "mov    ds,ax\n"
        "mov    bx,[bp+4]\n"    /* offset */
        "mov    al,[bp+6]\n"    /* color */
        "mov    [bx],al\n"
        "pop    bx\n"
        "pop    ds\n"
		);
}

static void set_palette_a(uint16_t idx, uint16_t cx, uint16_t dx)
{
    asm(
        "push   bx\n"
        "push   cx\n"
		"push   dx\n"
        "mov    bx,[bp+4]\n"    /* index */
        "mov    cx,[bp+6]\n"    /* cx */
        "mov    dx,[bp+8]\n"    /* cx */
		"mov    ax,#0x1010\n"
		"int    0x10\n"
        "pop    dx\n"
        "pop    cx\n"
        "pop    bx\n"
		);

}

static void get_palette_a(uint16_t idx, uint16_t *cx, uint16_t *dx)
{
    asm(
        "push   bx\n"
        "push   cx\n"
		"push   dx\n"
		"push   di\n"
		"mov    ax,#0x1015\n"
		"mov    bx,[bp+4]\n"    /* index */
		"int    0x10\n"
		"mov    di,[bp+6]\n"
		"mov    [di],cx\n"
		"mov    di,[bp+8]\n"
		"mov    [di],dx\n"
		"pop    di\n"
        "pop    dx\n"
        "pop    cx\n"
        "pop    bx\n"
		);

}


#endif

uint16_t width_internal, heigth_internal, mode_internal;

// this is just for mode 13h. TODO: make this generic
void drawpixel(int x,int y, uint8_t color)
{
	int offset;

	switch (mode_internal)
	{
	case VIDEO_MODE_10:
	case VIDEO_MODE_12:
#if defined(__C86__)
		vga_drawpixel(x, y, (int)color);
#endif
		break;

	case VIDEO_MODE_13:
		/*  y*320 = y*256 + y*64 = y*2^8 + y*2^6   */
		offset = (y<<8)+(y<<6)+x;
#if defined(__C86__)
		writevid(offset, color);
#elif defined(__WATCOMC__)
		VGA[offset] = color;
#endif
		break;

	case TEXT_MODE_3:
	case TEXT_MODE_2:
	case TEXT_MODE_1:
	case TEXT_MODE_0:
		break;

	default:
		printf("Unsupported mode: %02x\n", mode_internal);
	}
}


void set_mode(uint8_t mode_set)
{
	mode_internal = mode_set;

	switch (mode_set)
	{
	case VIDEO_MODE_10:
		width_internal = 640;
		heigth_internal = 350;
		set_mode_a(mode_set);
#ifdef __C86__
		vga_init();
#else
		printf("Mode unsupported on for OWC: %02x\n", mode_set);
#endif
		break;

	case VIDEO_MODE_12:
		width_internal = 640;
		heigth_internal = 480;
		set_mode_a(mode_set);
#ifdef __C86__
		vga_init();
#else
		printf("Mode unsupported on for OWC: %02x\n", mode_set);
#endif
		break;

	case VIDEO_MODE_13:
		width_internal = 320;
		heigth_internal = 200;
		set_mode_a(mode_set);
		break;

	case TEXT_MODE_3:
	case TEXT_MODE_2:
	case TEXT_MODE_1:
	case TEXT_MODE_0:
		width_internal = 0;
		heigth_internal = 0;
		set_mode_a(mode_set);
		break;

	default:
		set_mode_a(mode_set);
		printf("Unsupported mode: %02x\n", mode_set);
	}
}


uint16_t get_mode()
{
#if defined(__WATCOMC__)
	mode_internal = get_mode_a() & 0xff;
#elif defined(__C86__)
	uint16_t mode_set;
	get_mode_a(&mode_set);
	mode_internal = mode_set & 0xff;
#endif
	return mode_internal;
}

void set_palette(uint8_t red, uint8_t green, uint8_t blue, uint16_t idx)
{
#if defined(__WATCOMC__)
	pal_cx = ((green >> 2) << 8) | (blue >> 2);
	pal_dx = (red >> 2) << 8;

	set_palette_a(idx);
#elif defined(__C86__)
	set_palette_a(idx, ((green >> 2) << 8) | (blue >> 2), (red >> 2) << 8);
#endif
}

// returns the palette of index in colors[0,1,2]
void get_palette(uint8_t *red, uint8_t *green, uint8_t *blue, uint16_t idx)
{
#if defined(__WATCOMC__)
	get_palette_a(idx);

	*red = (uint8_t) ((pal_dx >> 8) << 2);
	*green = (uint8_t) (pal_cx >> 8) << 2;
	*blue = (uint8_t) (pal_cx & 0xff) << 2;
#elif defined(__C86__)
	uint16_t cx, dx;
	get_palette_a(idx, &cx, &dx);
	*red = (uint8_t) ((dx >> 8) << 2);
	*green = (uint8_t) (cx >> 8) << 2;
	*blue = (uint8_t) (cx & 0xff) << 2;
#endif
}

// this is our grayscale palette
void load_palette1g(uint8_t mode_set)
{
	uint8_t luminance;
	if (mode_set == VIDEO_MODE_13)
	{
		int num_colors = 256;

		for(int i = 0; i < num_colors; i++)
		{
			luminance = (uint8_t) i;
			set_palette(luminance, luminance, luminance, i);
		}
	}
}


// format for light conversion (palette1) is RRRGGGBB
void load_palette1(uint8_t mode_set)
{
	uint8_t red, green, blue;
	uint8_t color;

	if (mode_set == VIDEO_MODE_13)
	{
		int num_colors = 256;
		red = 0;
		green = 0;
		blue = 0;

		for(int i = 0; i < num_colors; i++)
		{
			color = (red << 5) | (green << 2) | (blue & 0x3);

			set_palette((red << 5) + 16, (green << 5) + 16, (blue << 6) + 32, color);

#ifdef DEBUG
			fprintf(stderr, "%hhu %hhu %hhu %hhu\n", color, (red << 5) + 16, (green << 5) + 16, (blue << 6) + 32);
#endif

			red++;
			if (red == 8)
			{
				green++;
				red = 0;
			}
			if (green == 8)
			{
				blue++;
				green = 0;
			}
			if (blue == 4)
			{
				blue = 0;
			}

		}
	}
}

// simple and works
uint8_t rgb2palette1(uint8_t r, uint8_t g, uint8_t b)
{
	return ((r >> 5) << 5) | ((g >> 5) << 2) | (b >> 6);
}

#if 0
// v2, some testing, but we prefer speed over beatifulness for now
uint8_t rgb2palette1(uint8_t r, uint8_t g, uint8_t b)
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;

	if ((r & 0x10) && ((r & 0xF0) != 0xF0) )
		red = ((r >> 5) + 1) << 5;
	else
		red = (r >> 5) << 5;

	if ((g & 0x10) && ((g & 0xF0) != 0xF0) )
		green = ((g >> 5) + 1) << 2;
	else
		green = (g >> 5) << 2;

	if ((b & 0x20) && ((b & 0xE0) != 0xE0) )
		blue = (b >> 6) + 1;
	else
		blue = b >> 6;

	return red | green | blue;
}
#endif


// Everything below here will be deleted as we don't need the default VGA palette for nothing basically..
#if 0
uint8_t vgapal[256][3] =
{
    /* colors 0-15 */
    {0x00, 0x00, 0x00},

    {0x00, 0x00, 0xAA},
    {0x00, 0xAA, 0x00},
    {0x00, 0xAA, 0xAA},
    {0xAA, 0x00, 0x00},
    {0xAA, 0x00, 0xAA},

    {0xAA, 0x55, 0x00},
    {0xAA, 0xAA, 0xAA},
    {0x55, 0x55, 0x55},
    {0x55, 0x55, 0xFF},
    {0x55, 0xFF, 0x55},

    {0x55, 0xFF, 0xFF},
    {0xFF, 0x55, 0x55},
    {0xFF, 0x55, 0xFF},
    {0xFF, 0xFF, 0x55},
    {0xFF, 0xFF, 0xFF},

    /* grayscale 16-31 (non gamma corrected */
    {0x00, 0x00, 0x00},
    {0x14, 0x14, 0x14},
    {0x20, 0x20, 0x20},
    {0x2C, 0x2C, 0x2C},
    {0x38, 0x38, 0x38},

    {0x45, 0x45, 0x45},
    {0x51, 0x51, 0x51},
    {0x61, 0x61, 0x61},
    {0x71, 0x71, 0x71},
    {0x82, 0x82, 0x82},

    {0x92, 0x92, 0x92},
    {0xA2, 0xA2, 0xA2},
    {0xB6, 0xB6, 0xB6},
    {0xCB, 0xCB, 0xCB},
    {0xE3, 0xE3, 0xE3},

    {0xFF, 0xFF, 0xFF},
    // HERE ------> 1
    /* hue mix 32-55 (1 */
    {0x00, 0x00, 0xFF},
    {0x41, 0x00, 0xFF},
    {0x7D, 0x00, 0xFF},
    {0xBE, 0x00, 0xFF},

    {0xFF, 0x00, 0xFF},
    {0xFF, 0x00, 0xBE},
    {0xFF, 0x00, 0x7D},
    {0xFF, 0x00, 0x41},
    {0xFF, 0x00, 0x00},

    {0xFF, 0x41, 0x00},
    {0xFF, 0x7D, 0x00},
    {0xFF, 0xBE, 0x00},
    {0xFF, 0xFF, 0x00},
    {0xBE, 0xFF, 0x00},

    {0x7D, 0xFF, 0x00},
    {0x41, 0xFF, 0x00},
    {0x00, 0xFF, 0x00},
    {0x00, 0xFF, 0x41},
    {0x00, 0xFF, 0x7D},

    {0x00, 0xFF, 0xBE},
    {0x00, 0xFF, 0xFF},
    {0x00, 0xBE, 0xFF},
    {0x00, 0x7D, 0xFF},
    {0x00, 0x41, 0xFF},

    /* hue mix 56-79 (2 */
    {0x7D, 0x7D, 0xFF},
    {0x9E, 0x7D, 0xFF},
    {0xBE, 0x7D, 0xFF},
    {0xDF, 0x7D, 0xFF},
    {0xFF, 0x7D, 0xFF},

    {0xFF, 0x7D, 0xDF},
    {0xFF, 0x7D, 0xBE},
    {0xFF, 0x7D, 0x9E},
    {0xFF, 0x7D, 0x7D},
    {0xFF, 0x9E, 0x7D},

    {0xFF, 0xBE, 0x7D},
    {0xFF, 0xDF, 0x7D},
    {0xFF, 0xFF, 0x7D},
    {0xDF, 0xFF, 0x7D},
    {0xBE, 0xFF, 0x7D},

    {0x9E, 0xFF, 0x7D},
    {0x7D, 0xFF, 0x7D},
    {0x7D, 0xFF, 0x9E},
    {0x7D, 0xFF, 0xBE},
    {0x7D, 0xFF, 0xDF},

    {0x7D, 0xFF, 0xFF},
    {0x7D, 0xDF, 0xFF},
    {0x7D, 0xBE, 0xFF},
    {0x7D, 0x9E, 0xFF},
    /* hue mix 80-103 (3 */
    {0xB6, 0xB6, 0xFF},

    {0xC7, 0xB6, 0xFF},
    {0xDB, 0xB6, 0xFF},
    {0xEB, 0xB6, 0xFF},
    {0xFF, 0xB6, 0xFF},
    {0xFF, 0xB6, 0xEB},

    {0xFF, 0xB6, 0xDB},
    {0xFF, 0xB6, 0xC7},
    {0xFF, 0xB6, 0xB6},
    {0xFF, 0xC7, 0xB6},
    {0xFF, 0xDB, 0xB6},

    {0xFF, 0xEB, 0xB6},
    {0xFF, 0xFF, 0xB6},
    {0xEB, 0xFF, 0xB6},
    {0xDB, 0xFF, 0xB6},
    {0xC7, 0xFF, 0xB6},

    {0xB6, 0xFF, 0xB6},
    {0xB6, 0xFF, 0xC7},
    {0xB6, 0xFF, 0xDB},
    {0xB6, 0xFF, 0xEB},
    {0xB6, 0xFF, 0xFF},

    {0xB6, 0xEB, 0xFF},
    {0xB6, 0xDB, 0xFF},
    {0xB6, 0xC7, 0xFF},
    // HERE ------> 2
    /* hue mix 104-127 (4 dark 1 */
    {0x00, 0x00, 0x71},
    {0x1C, 0x00, 0x71},
    {0x38, 0x00, 0x71},
    {0x55, 0x00, 0x71},

    {0x71, 0x00, 0x71},
    {0x71, 0x00, 0x55},
    {0x71, 0x00, 0x38},
    {0x71, 0x00, 0x1C},
    {0x71, 0x00, 0x00},

    {0x71, 0x1C, 0x00},
    {0x71, 0x38, 0x00},
    {0x71, 0x55, 0x00},
    {0x71, 0x71, 0x00},
    {0x55, 0x71, 0x00},

    {0x38, 0x71, 0x00},
    {0x1C, 0x71, 0x00},
    {0x00, 0x71, 0x00},
    {0x00, 0x71, 0x1C},
    {0x00, 0x71, 0x38},

    {0x00, 0x71, 0x55},
    {0x00, 0x71, 0x71},
    {0x00, 0x55, 0x71},
    {0x00, 0x38, 0x71},
    {0x00, 0x1C, 0x71},

    /* hue mix 56-79 (2 */
    {0x38, 0x38, 0x71},
    {0x45, 0x38, 0x71},
    {0x55, 0x38, 0x71},
    {0x61, 0x38, 0x71},
    {0x71, 0x38, 0x71},

    {0x71, 0x38, 0x61},
    {0x71, 0x38, 0x55},
    {0x71, 0x38, 0x45},
    {0x71, 0x38, 0x38},
    {0x71, 0x45, 0x38},

    {0x71, 0x55, 0x38},
    {0x71, 0x61, 0x38},
    {0x71, 0x71, 0x38},
    {0x61, 0x71, 0x38},
    {0x55, 0x71, 0x38},

    {0x45, 0x71, 0x38},
    {0x38, 0x71, 0x38},
    {0x38, 0x71, 0x45},
    {0x38, 0x71, 0x55},
    {0x38, 0x71, 0x61},

    {0x38, 0x71, 0x71},
    {0x38, 0x61, 0x71},
    {0x38, 0x55, 0x71},
    {0x38, 0x45, 0x71},
    /* hue mix 80-103 (3 */
    {0x51, 0x51, 0x71},

    {0x59, 0x51, 0x71},
    {0x61, 0x51, 0x71},
    {0x69, 0x51, 0x71},
    {0x71, 0x51, 0x71},
    {0x71, 0x51, 0x69},

    {0x71, 0x51, 0x61},
    {0x71, 0x51, 0x59},
    {0x71, 0x51, 0x51},
    {0x71, 0x59, 0x51},
    {0x71, 0x61, 0x51},

    {0x71, 0x69, 0x51},
    {0x71, 0x71, 0x51},
    {0x69, 0x71, 0x51},
    {0x61, 0x71, 0x51},
    {0x59, 0x71, 0x51},

    {0x51, 0x71, 0x51},
    {0x51, 0x71, 0x59},
    {0x51, 0x71, 0x61},
    {0x51, 0x71, 0x69},
    {0x51, 0x71, 0x71},

    {0x51, 0x69, 0x71},
    {0x51, 0x61, 0x71},
    {0x51, 0x59, 0x71},
    // HERE ------> 3
    /* hue mix 104-127 (4 dark 1 */
    {0x00, 0x00, 0x41},
    {0x10, 0x00, 0x41},
    {0x20, 0x00, 0x41},
    {0x30, 0x00, 0x41},

    {0x41, 0x00, 0x41},
    {0x41, 0x00, 0x30},
    {0x41, 0x00, 0x20},
    {0x41, 0x00, 0x10},
    {0x41, 0x00, 0x00},

    {0x41, 0x10, 0x00},
    {0x41, 0x20, 0x00},
    {0x41, 0x30, 0x00},
    {0x41, 0x41, 0x00},
    {0x30, 0x41, 0x00},

    {0x20, 0x41, 0x00},
    {0x10, 0x41, 0x00},
    {0x00, 0x41, 0x00},
    {0x00, 0x41, 0x10},
    {0x00, 0x41, 0x20},

    {0x00, 0x41, 0x30},
    {0x00, 0x41, 0x41},
    {0x00, 0x30, 0x41},
    {0x00, 0x20, 0x41},
    {0x00, 0x10, 0x41},

    /* hue mix 56-79 (2 */
    {0x20, 0x20, 0x41},
    {0x28, 0x20, 0x41},
    {0x30, 0x20, 0x41},
    {0x3C, 0x20, 0x41},
    {0x41, 0x20, 0x41},

    {0x41, 0x20, 0x3C},
    {0x41, 0x20, 0x30},
    {0x41, 0x20, 0x28},
    {0x41, 0x20, 0x20},
    {0x41, 0x28, 0x20},

    {0x41, 0x30, 0x20},
    {0x41, 0x3C, 0x20},
    {0x41, 0x41, 0x20},
    {0x3C, 0x41, 0x20},
    {0x30, 0x41, 0x20},

    {0x28, 0x41, 0x20},
    {0x20, 0x41, 0x20},
    {0x20, 0x41, 0x28},
    {0x20, 0x41, 0x30},
    {0x20, 0x41, 0x3C},

    {0x20, 0x41, 0x41},
    {0x20, 0x3C, 0x41},
    {0x20, 0x30, 0x41},
    {0x20, 0x28, 0x41},
    /* hue mix 80-103 (3 */
    {0x2C, 0x2C, 0x41},

    {0x30, 0x2C, 0x41},
    {0x34, 0x2C, 0x41},
    {0x3C, 0x2C, 0x41},
    {0x41, 0x2C, 0x41},
    {0x41, 0x2C, 0x3C},

    {0x41, 0x2C, 0x34},
    {0x41, 0x2C, 0x30},
    {0x41, 0x2C, 0x2C},
    {0x41, 0x30, 0x2C},
    {0x41, 0x34, 0x2C},

    {0x41, 0x3C, 0x2C},
    {0x41, 0x41, 0x2C},
    {0x3C, 0x41, 0x2C},
    {0x34, 0x41, 0x2C},
    {0x30, 0x41, 0x2C},

    {0x2C, 0x41, 0x2C},
    {0x2C, 0x41, 0x30},
    {0x2C, 0x41, 0x34},
    {0x2C, 0x41, 0x3C},
    {0x2C, 0x41, 0x41},

    {0x2C, 0x3C, 0x41},
    {0x2C, 0x34, 0x41},
    {0x2C, 0x30, 0x41},

    /* all black */
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},

    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},

};

uint8_t rgb2vga(int r, int g, int b) {

	int closest = 32000;
	int ndx = 0;
	for (int i = 0; i < 248; i++)
	{
		uint8_t *sample = vgapal[i];
		int rs = (sample[0] > r)? sample[0] - r : r - sample[0];
		int gs = (sample[1] > g)? sample[1] - g : g - sample[1];
		int bs = (sample[2] > b)? sample[2] - b : b - sample[2];
		int dst = rs + gs + bs;

		// printf("dist %d\n", dst);

		if (closest > dst)
		{
			closest = dst;
			ndx = i;
		}
		else if (dst < 30)
		{
			ndx = i;
			break;
		}

	}

	return (uint8_t)ndx;
}
#endif
