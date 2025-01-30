# ELKS viewer


An optimized image viewer which runs on ELKS on Intel 8088 CPU or better.

ELKS Viewer is composed by standalone viewers:
- bmpview: Supports 1, 4, 8 and 24 bits BMP (run-length encoding also supported)
- ppmview: Supports PPM and PGM formats
- jpgview: A JPEG viewer which supports coloured and grayscale images

JPEG decoding uses PicoJPEG.

At this point, only the Open Watcom v2 compiler is supported, and the software is linked to ELKS libc. For now, only mode 0x13 (320x200 256 colors) is supported.


```
 Usage: {jpg,bmp,ppm}view [source_file]
    source_file: Image file to decode.
```

# Author

ELKS Viewer is developed by Rafael Diniz.
