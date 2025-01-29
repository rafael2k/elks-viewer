#include "graphics.h"


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// Reference: https://www.chibialiens.com/8086/platform.php?noui=1
uint8_t __far *CGA = (void __far *)0xB8000000L;        /* this points to video CGA memory. */
uint8_t __far *VGA = (void __far *)0xA0000000L;        /* this points to video VGA memory. */


uint8_t vgapal[256][3] = {
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

uint8_t __far *get_video_pointer()
{
	return VGA;
}

uint8_t rgb2vga(int r, int g, int b) {

	int closest = 32000;
	int ndx = 0;
	for (int i = 0; i < 248; i++) {
		uint8_t *sample = vgapal[i];
		int rs = (sample[0] > r)? sample[0] - r : r - sample[0];
		int gs = (sample[1] > g)? sample[1] - g : g - sample[1];
		int bs = (sample[2] > b)? sample[2] - b : b - sample[2];
		int dst = rs + gs + bs;

		// printf("dist %d\n", dst);

		if (closest > dst) {
			closest = dst;
			ndx = i;
		}
		else if (dst < 30) {
			ndx = i;
			break;
		}

	}

	return (uint8_t)ndx;
}

void plot_pixel(int x,int y, uint8_t color)
{
	// x + (y*320)
     /*  y*320 = y*256 + y*64 = y*2^8 + y*2^6   */
    int offset = (y<<8)+(y<<6)+x;
	VGA[offset] = color;
}



//    on return:
//  AH = number of screen columns
//	AL = mode currently set (see VIDEO MODES)
//	BH = current display page
uint16_t get_mode_a();
#pragma aux get_mode_a value [ax] =								\
"mov ax,0F00h", \
"int 10h", \
modify [ ax bx ];

void set_mode_a(uint16_t mode);
#pragma aux set_mode_a parm [ax] =								\
"int 10h", \
modify [ ax ];

void set_palette_a(uint16_t ax, uint16_t bx);
#pragma aux set_palette_a parm [ ax bx ] =		\
"mov dx,bx", \
"mov bh,0", \
"mov dl,0", \
"mov cx,ax", \
"mov ax, 1010h", \
"int 10h", \
modify [ ax bx cx dx ];

uint16_t pal_cx, pal_dx;

 // in [ax] and [ax+1] returns CH = green value CL = blue value DH = red value
void get_palette_a(uint16_t bx);
#pragma aux get_palette_a parm [ bx ] =		\
"mov bh,0", \
"mov ax,1015h",									\
"int 10h",															\
"mov pal_cx,cx", \
"mov pal_dx,dx", \
modify [ ax bx cx dx ];

void set_mode(uint8_t mode)
{
	set_mode_a((uint16_t)mode);
}

uint16_t get_mode()
{
	return get_mode_a();
}

void set_palette(uint8_t red, uint8_t green, uint8_t blue, uint16_t index)
{
	uint16_t arg1 = (((uint16_t) green >> 2) << 8) | ((uint16_t) blue >> 2);
	uint16_t arg2 = (((uint16_t) red >> 2) << 8) | index;
	set_palette_a(arg1, arg2);
}

// returns the palette of index in colors[0,1,2]
void get_palette(uint8_t *red, uint8_t *green, uint8_t *blue, uint16_t index)
{
	get_palette_a(index);

	*red = (pal_dx >> 8) << 2;
	*green = (pal_cx >> 8) << 2;
	*blue = (pal_cx & 0xff) << 2;
}
