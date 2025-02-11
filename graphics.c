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

void set_palette_c(uint8_t color, uint8_t red, uint8_t green, uint8_t blue);
#pragma aux set_palette_c parm [ al ] [ bl ] [ cl ] [ dl ] =	\
	"push dx",													\
	"mov dx,03c8h",												\
	"out dx,al",												\
	"mov dx,03c9h",												\
	"mov al,bl",												\
	"out dx,al",												\
	"mov al,cl",												\
	"out dx,al",												\
	"pop ax"													\
	"out dx,al",												\
	modify [ ax bx cx dx ];

void set_palette_register(uint8_t color, uint8_t register);
#pragma aux set_palette_register parm [ al ] [ bl ] =	\
	"push ax",													\
	"mov cx,03dah",												\
	"mov al, 0h",												\
	"out dx,al",												\
	"pop ax",													\
	"mov dx,03c0h",												\
	"out dx,al",												\
	"mov dx,03dah",												\
	"mov al,20h",												\
	modify [ ax bx cx dx ];


void get_palette_a(uint16_t bx);
#pragma aux get_palette_a parm [ bx ] =			\
	"mov bh,0",									\
	"mov ax,1015h",								\
	"int 10h",									\
	"mov pal_cx,cx",							\
	"mov pal_dx,dx",							\
	modify [ ax bx cx dx ];

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

static void set_palette_c(uint16_t color, uint16_t red, uint16_t green, uint16_t blue)
{

    asm(
        "push   bx\n"
        "push   cx\n"
		"push   dx\n"
		"mov    dx,#0x03c8\n"
        "mov    al,[bp+4]\n"
		"out    dx,al\n"
		"mov    dx,#0x03c9\n"
		"mov    al,[bp+6]\n"
		"out    dx,al\n"
		"mov    al,[bp+8]\n"
		"out    dx,al\n"
		"mov    al,[bp+10]\n"
		"out    dx,al\n"
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

void set_palette_register(uint8_t palette_register, uint8_t dac_index)
{
    asm(
        "push   ax\n"
		"push   bx\n"
        "push   cx\n"
		"push   dx\n"
		"mov    ax,#0x1000\n"
		"mov    bh,[bp+5]\n"
		"mov    bl,[bp+4]\n"
		"int    0x10\n"
        "pop    dx\n"
        "pop    cx\n"
        "pop    bx\n"
		"pop    ax\n"
		);
}

static void set_pport(uint8_t idx, uint8_t color)
{
	asm(
		"push   ax\n"
		"push   dx\n"
		"mov    dx,#0x3DA\n"
		"mov    al,#0x0\n"
		"out    dx,al\n"
		"mov    dx,#0x3C0\n"
		"mov    al,[bp+4]\n"
		"out    dx,al\n"
		"mov    al,[bx+5]\n"
		"out    dx,al\n"
		"mov    al,#0x20\n"
		"out    dx,al\n"
        "pop    dx\n"
		"pop    ax\n"
		);

}

void set_dac_color(uint8_t dac_index, uint8_t red, uint8_t green, uint8_t blue)
{
    asm(
        "push   ax\n"
		"push   bx\n"
        "push   cx\n"
		"push   dx\n"
		"mov    dx,#0x3C8\n"
		"mov    al,[bp+4]\n"
		"out    dx,al\n"
		"mov    dx,#0x3C9\n"
		"mov    al,[bp+5]\n"
		"out    dx,al\n"
		"mov    al,[bp+6]\n"
		"out    dx,al\n"
		"mov    al,[bp+7]\n"
		"out    dx,al\n"
        "pop    dx\n"
        "pop    cx\n"
        "pop    bx\n"
		"pop    ax\n"
		);
}


#endif

uint16_t width_internal, heigth_internal, mode_internal;
//static unsigned char mask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
static uint16_t mask[8] ={ 0x8008, 0x4008, 0x2008, 0x1008, 0x0808, 0x0408, 0x0208, 0x0108 };
//static uint16_t mask[8] =  { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

uint8_t color_register_map[16] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
};

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
	if (mode_internal == VIDEO_MODE_13)
	{
#if defined(__WATCOMC__)
		pal_cx = ((green >> 2) << 8) | (blue >> 2);
		pal_dx = (red >> 2) << 8;

		set_palette_a(idx);
#elif defined(__C86__)
		set_palette_a(idx, ((green >> 2) << 8) | (blue >> 2), (red >> 2) << 8);
#endif
	}
	if (mode_internal == VIDEO_MODE_12 || mode_internal == VIDEO_MODE_10)
	{
		set_palette_c(color_register_map[idx],red >> 2, green >> 2, blue >> 2);
	}
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

void load_palette1g_4bit(uint8_t mode_set)
{
	if (mode_set == VIDEO_MODE_12 || mode_set == VIDEO_MODE_10)
	{
		for (int i = 0; i < 16; i++)
		{
			uint8_t lum = i << 2;
			set_palette_c(color_register_map[i], lum, lum, lum);
		}
	}
}

// palette for VGA 16 colors to better match RGB
void load_palette1_4bit(uint8_t mode_set)
{
	if (mode_set == VIDEO_MODE_12 || mode_set == VIDEO_MODE_10)
	{
		for (int i = 0; i < 16; i++)
		{
			set_palette_c(color_register_map[i], vga_palette[i].r >> 2, vga_palette[i].g >> 2, vga_palette[i].b >> 2);
		}
	}
}


// simple and works
uint8_t rgb2palette1(uint8_t r, uint8_t g, uint8_t b)
{
	return ((r >> 5) << 5) | ((g >> 5) << 2) | (b >> 6);
}

// Check if a color is grayscale within tolerance
static inline int is_gray(unsigned char r, unsigned char g, unsigned char b, unsigned char tolerance) {
    unsigned char max = r > g ? (r > b ? r : b) : (g > b ? g : b);
    unsigned char min = r < g ? (r < b ? r : b) : (g < b ? g : b);
    return (max - min) <= tolerance;
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
