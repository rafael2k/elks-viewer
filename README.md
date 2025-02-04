# ELKS viewer


An optimized image viewer which runs on ELKS on Intel 8088 CPU or better.

ELKS Viewer is a set of tools composed by standalone viewers:
- bmpview: Supports 1, 4, 8 and 24 bits BMP (run-length encoding also supported)
- ppmview: Supports PPM and PGM formats
- jpgview: A JPEG viewer which supports coloured and grayscale images

JPEG decoding uses PicoJPEG. Source "graphics.c" contains the graphics operation routines. 


Open Watcom v2 compiler is supported (C86 support WIP), and the software is linked to ELKS libc. 


```
 Usage: {jpg,bmp,ppm}view [source_file]
    source_file: Image file to decode.
```

Folder "3rdparty" contains CPIG software (Color Palette Inference Generator) to create optimized palettes for a given image. 

# Build

In order to build ELKS viewer tools, you need Open Watcom v2 installed, and ELKS libc compiled for OpenWatcom. For it, you need ELKS source code, available at
https://github.com/ghaerr/elks/ and follow the steps to build the ELKS libc as explained here: https://github.com/ghaerr/elks/wiki/Using-OpenWatcom-C-with-ELKS

On Linux, for example, do the following (ELKS source at ~/elks, and ELKS Viewer at ~/elks-viewer, Open Watcom installed in default location):
```
cd ~/elks
. env.sh
. /usr/bin/watcom/owsetenv.sh
cd libc
make -f watcom.mk
cd ~/elks-viewer
```

Then, with the setup ready (in the same shell, with environment variables properly set), type:
```
make -f Makefile.owc
```

# Author

ELKS Viewer is developed by Rafael Diniz.
