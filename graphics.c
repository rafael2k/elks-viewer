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

void set_palette_b(uint16_t ax);
#pragma aux set_palette_b parm [ ax ] =			\
	"mov bx,ax",								\
	"mov ax, 1000h",							\
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

#if 1
void ow_vga_init();
#pragma aux ow_vga_init =							\
	"mov dx,0x03ce",								\
	"mov ax,0xff01",								\
	"out dx,ax",									\
	"mov ax,0x0003",								\
	"out dx,ax",									\
	"mov ax,0x0005",								\
	"out dx,ax",									\
	modify [ ax bx cx dx ];
#endif

#if 0
void ow_vga_init();
#pragma aux ow_vga_init =							\
	"mov dx,0x03ce",								\
	"mov ax,0x0205",							\
	"out dx,ax",								\
	"mov ax,0x0F02",								\
	modify [ ax bx cx dx ];
#endif

void ow_setcolorreg(uint16_t ax);
#pragma aux ow_setcolorreg parm [ax] =							\
	"mov dx,03ceh",								\
	"out dx,ax",								\
	modify [ ax bx cx dx ];

void ow_setcolor(uint16_t color);
#pragma aux ow_setcolor parm [ax] =				\
	"mov dx,03ceh",								\
	"mov ah, al",								\
	"xor al,al",								\
	"out dx,ax",								\
	modify [ ax bx cx dx ];

void ow_setpix(uint16_t offset);
#pragma aux ow_setpix parm [ax] =							\
	"mov cx,ds",								\
	"mov bx,ax",								\
	"mov ax,0xa000",								\
	"mov ds,ax",								\
	"mov al,0h",								\
	"or [bx],al",							\
	"mov ds,cx",						\
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
void writevid(uint16_t offset, uint8_t c)
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
        "mov    dx,[bp+8]\n"    /* dx */
		"mov    ax,#0x1010\n"
		"int    0x10\n"
        "pop    dx\n"
        "pop    cx\n"
        "pop    bx\n"
		);

}

static void set_palette_b(uint16_t bx)
{
    asm(
        "push   bx\n"
        "push   cx\n"
		"push   dx\n"
        "mov    bx,[bp+4]\n"
		"mov    ax,#0x1000\n"
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
//static unsigned char mask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
static uint16_t mask[8] ={ 0x8008, 0x4008, 0x2008, 0x1008, 0x0808, 0x0408, 0x0208, 0x0108 };
//static uint16_t mask[8] =  { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

void drawpixel(int x,int y, uint8_t color)
{
	uint16_t offset;

	switch (mode_internal)
	{
	case VIDEO_MODE_10:
	case VIDEO_MODE_12:
#if defined(__C86__)
		vga_drawpixel(x, y, color);
#elif defined(__WATCOMC__)
		ow_setcolor((uint16_t)color);
		ow_setcolorreg(mask[x & 0x07]);
		offset = (y * 80) + (x >> 3);
		ow_setpix(offset);
		//VGA[offset] = color;
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
#if  defined(__C86__)
		vga_init();
#elif defined(__WATCOMC__)
		ow_vga_init();
#endif
		break;

	case VIDEO_MODE_12:
		width_internal = 640;
		heigth_internal = 480;
		set_mode_a(mode_set);
#if  defined(__C86__)
		vga_init();
#elif defined(__WATCOMC__)
		ow_vga_init();
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

// format for light conversion from RGB 24bit (palette1) is RRRGGGBB
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

// palette for VGA 16 colors to better match RGB
void load_palette1_4bit(uint8_t mode_set)
{
	int num_colors = 16;

	uint16_t bx;

	for(int c = 0; c < num_colors; c++)
	{

        if (c == 8)
        {
            bx = 0x38; // bh
        }
        else
        {
            bx  = c & 1 ? (c & 8 ? 0x09 : 0x01) : 0;
            bx |= c & 2 ? (c & 8 ? 0x12 : 0x02) : 0;
            bx |= c & 4 ? (c & 8 ? 0x24 : 0x04) : 0;
        }
        bx  = (bx << 8) + c; // bl = c
		set_palette_b(bx);
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

    /* grayscale 16-31 (non gamma corrected) */
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

uint8_t vgapal4b[16][3] =
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
};

uint8_t rgb2vga4b(int r, int g, int b)
{

	int closest = 32000;
	int ndx = 0;
	for (int i = 0; i < 16; i++)
	{
		uint8_t *sample = vgapal4b[i];
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

//#define BRI     3
//#define RED     2
//#define GRN     1
//#define BLU     0
#define BRI     0
#define RED     1
#define GRN     2
#define BLU     3

/*
 * 8x4 dither matrix (4x4 replicated twice horizontally to fill byte).
 */
static unsigned long ddithmask[16] = // Color dither
{
    0x00000000L,
    0x88000000L,
    0x88002200L,
    0x8800AA00L,
    0xAA00AA00L,
    0xAA44AA00L,
    0xAA44AA11L,
    0xAA44AA55L,
    0xAA55AA55L,
    0xAADDAA55L,
    0xAADDAA77L,
    0xAADDAAFFL,
    0xAAFFAAFFL,
    0xEEFFAAFFL,
    0xEEFFBBFFL,
    0xEEFFFFFFL,
};
static unsigned long bdithmask[16] = // Color dither
{
    0x00000000L,
    0x88000000L,
    0x88002200L,
    0x8800AA00L,
    0xAA00AA00L,
    0xAA44AA00L,
    0xAA44AA11L,
    0xAA44AA55L,
    0xAA55AA55L,
    0xAADDAA55L,
    0xAADDAA77L,
    0xAADDAAFFL,
    0xAAFFAAFFL,
    0xEEFFAAFFL,
    0xEEFFBBFFL,
    0xFFFFFFFFL
};
static unsigned int pixmask[] =
{
    0x8008, 0x4008, 0x2008, 0x1008, 0x0808, 0x0408, 0x0208, 0x0108
};

/*
 * Build a dithered brush and return the closest solid color match to the RGB.
 */
unsigned char buildbrush(uint8_t red, uint8_t grn, uint8_t blu, uint32_t *brush)
{
    unsigned char clr, l;

    /*
     * Find MAX(R,G,B)
     */
    if (red >= grn && red >= blu)
        l = red;
    else if (grn >= red && grn >= blu)
        l = grn;
    else // if (blue >= grn && blu >= red)
        l = blu;
    if (l > 127) // 50%-100% brightness
    {
        /*
         * Fill brush based on scaled RGB values (brightest -> 100% -> 0x0F).
         */
        brush[BRI] = bdithmask[(l >> 3) & 0x0F];
        brush[RED] = bdithmask[(red << 4) / (l + 8)];
        brush[GRN] = bdithmask[(grn << 4) / (l + 8)];
        brush[BLU] = bdithmask[(blu << 4) / (l + 8)];
        clr        = 0x08
                   | ((red & 0x80) >> 5)
                   | ((grn & 0x80) >> 6)
                   | ((blu & 0x80) >> 7);
    }
    else // 0%-50% brightness
    {
        /*
         * Fill brush based on dim RGB values.
         */
        if ((l - red) + (l - grn) + (l - blu) < 8)
        {
            /*
             * RGB close to grey.
             */
            if (l > 63) // 25%-50% grey
            {
                /*
                 * Mix light grey and dark grey.
                 */
                brush[BRI] = ~ddithmask[((l - 64) >> 2)];
                brush[RED] =
                brush[GRN] =
                brush[BLU] =  ddithmask[((l - 64) >> 2)];
                clr        =  0x07;
            }
            else // 0%-25% grey
            {
                /*
                 * Simple dark grey dither.
                 */
                brush[BRI] = ddithmask[(l >> 2)];
                brush[RED] = 0;
                brush[GRN] = 0;
                brush[BLU] = 0;
                clr        = (l > 31) ? 0x08 : 0x00;
            }
        }
        else
        {
            /*
             * Simple 8 color RGB dither.
             */
            brush[BRI] = 0;
            brush[RED] = ddithmask[red >> 3];
            brush[GRN] = ddithmask[grn >> 3];
            brush[BLU] = ddithmask[blu >> 3];
            clr        = ((red & 0x40) >> 4)
                       | ((grn & 0x40) >> 5)
                       | ((blu & 0x40) >> 6);
        }
    }
    return clr;
}


void drawpixel4rgb(int x, int y, unsigned char red, unsigned char grn, unsigned char blu)
{
    uint8_t pixbrush[4][4];
	uint8_t pix, idx;

    idx = buildbrush(red, grn, blu, (uint32_t *) pixbrush);
  /*
     * Extract pixel value from IRGB planes.
     */
    pix  = ((pixbrush[BRI][y & 3] >> (x & 3)) & 0x01) << BRI;
    pix |= ((pixbrush[RED][y & 3] >> (x & 3)) & 0x01) << RED;
    pix |= ((pixbrush[GRN][y & 3] >> (x & 3)) & 0x01) << GRN;
    pix |= ((pixbrush[BLU][y & 3] >> (x & 3)) & 0x01) << BLU;

	int ofs = y * 80 + (x >> 3);

    //outpw(0x3CE, pixmask[x & 0x07]);
#if defined(__C86__)
		writevid(ofs, pix);
#elif defined(__WATCOMC__)
		VGA[ofs] = pix;
#endif
}

#include <stdlib.h>

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} Color;

// Standard VGA 16-color palette
static const Color vga_palette[16] = {
    {0x00, 0x00, 0x00},  // 0  Black
    {0x00, 0x00, 0xAA},  // 1  Blue
    {0x00, 0xAA, 0x00},  // 2  Green
    {0x00, 0xAA, 0xAA},  // 3  Cyan
    {0xAA, 0x00, 0x00},  // 4  Red
    {0xAA, 0x00, 0xAA},  // 5  Magenta
    {0xAA, 0x55, 0x00},  // 6  Brown
    {0xAA, 0xAA, 0xAA},  // 7  Light Gray
    {0x55, 0x55, 0x55},  // 8  Dark Gray
    {0x55, 0x55, 0xFF},  // 9  Light Blue
    {0x55, 0xFF, 0x55},  // 10 Light Green
    {0x55, 0xFF, 0xFF},  // 11 Light Cyan
    {0xFF, 0x55, 0x55},  // 12 Light Red
    {0xFF, 0x55, 0xFF},  // 13 Light Magenta
    {0xFF, 0xFF, 0x55},  // 14 Yellow
    {0xFF, 0xFF, 0xFF}   // 15 White
};

// Gray levels in VGA palette for quick reference
static const unsigned char vga_grays[] = {
    0x00,  // Black    (index 0)
    0x55,  // Dark Gray (index 8)
    0xAA,  // Light Gray (index 7)
    0xFF   // White     (index 15)
};

static const unsigned char vga_gray_indices[] = {0, 8, 7, 15};

// Check if a color is grayscale within tolerance
static inline int is_gray(unsigned char r, unsigned char g, unsigned char b, unsigned char tolerance) {
    unsigned char max = r > g ? (r > b ? r : b) : (g > b ? g : b);
    unsigned char min = r < g ? (r < b ? r : b) : (g < b ? g : b);
    return (max - min) <= tolerance;
}

// Find closest VGA gray level
static unsigned char find_closest_gray(unsigned char level) {
    unsigned char best_index = 0;
    unsigned long min_diff = 0xFF;

    for (int i = 0; i < 4; i++) {
        unsigned long diff = abs(level - vga_grays[i]);
        if (diff < min_diff) {
            min_diff = diff;
            best_index = i;
        }
    }

    return vga_gray_indices[best_index];
}

// Convert 24-bit RGB to VGA 16 colors with grayscale handling
unsigned char rgb_to_vga16(unsigned char r, unsigned char g, unsigned char b) {
    // Check for grayscale first (using weighted luminance)
    if (is_gray(r, g, b, 10)) {
        unsigned char gray = (r * 30 + g * 59 + b * 11) / 100;
        return find_closest_gray(gray);
    }

    unsigned char best_index = 0;
    unsigned long min_distance = 0xFFFFFFFF;

    // Skip gray indices (0, 7, 8, 15) if not grayscale
    for (int i = 0; i < 16; i++) {
        // Skip gray colors as they were handled above
        if (i == 7 || i == 8 || i == 15) continue;

        int dr = (int)r - (int)vga_palette[i].r;
        int dg = (int)g - (int)vga_palette[i].g;
        int db = (int)b - (int)vga_palette[i].b;

        unsigned long distance =
            ((unsigned long)dr * dr * 30 +
             (unsigned long)dg * dg * 59 +
             (unsigned long)db * db * 11) / 100;

        if (distance < min_distance) {
            min_distance = distance;
            best_index = i;
        }
    }

    return best_index;
}

// Fast conversion with grayscale handling
unsigned char rgb_to_vga16_fast(unsigned char r, unsigned char g, unsigned char b) {
    // Check for grayscale using simpler method
    if (is_gray(r, g, b, 20)) {
        unsigned char gray = (r/3 + g/3 + b/3);  // Simple average for speed
        if (gray < 0x40) return 0;      // Black
        if (gray < 0x80) return 8;      // Dark Gray
        if (gray < 0xC0) return 7;      // Light Gray
        return 15;                       // White
    }

    unsigned char indexx = 0;

    // Set high intensity bit
    if ((r > 0xC0) || (g > 0xC0) || (b > 0xC0)) {
        indexx |= 0x08;
    }

    // Set RGB bits
    if (r > 0x80) indexx |= 0x04;
    if (g > 0x80) indexx |= 0x02;
    if (b > 0x80) indexx |= 0x01;

    return indexx;
}

// Convert RGB565 to VGA 16 colors
unsigned char rgb565_to_vga16(unsigned short rgb565) {
    unsigned char r = ((rgb565 >> 11) & 0x1F) << 3;
    unsigned char g = ((rgb565 >> 5) & 0x3F) << 2;
    unsigned char b = (rgb565 & 0x1F) << 3;

    return rgb_to_vga16(r, g, b);
}

// Convert VGA 16 color index back to 24-bit RGB
void vga16_to_rgb(unsigned char indexx, unsigned char* r, unsigned char* g, unsigned char* b) {
    *r = vga_palette[indexx & 0x0F].r;
    *g = vga_palette[indexx & 0x0F].g;
    *b = vga_palette[indexx & 0x0F].b;
}
