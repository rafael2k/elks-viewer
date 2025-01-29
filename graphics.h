#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include <stdint.h>

#define TEXT_MODE_3         0x03      /* 80x25 4-bit text mode. */
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


uint8_t __far *get_video_pointer();
void plot_pixel(int x,int y, uint8_t color);
void set_mode(uint8_t mode);
uint16_t get_mode();
void set_palette(uint8_t red, uint8_t green, uint8_t blue, uint16_t index);
void get_palette(uint8_t *red, uint8_t *green, uint8_t *blue, uint16_t index);

// this is very slow - don't use it
uint8_t rgb2vga(int r, int g, int b);
#endif // GRAPHICS_H_
