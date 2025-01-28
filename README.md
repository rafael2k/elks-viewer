# ELKS viewer

An optimized image viewer which runs on ELKS on Intel 8088 CPU or better.

Based on picojpeg for jpeg decoding. Also supports BMP and PPM formats.

At this point, only the Open Watcom v2 compiler is supported, together with the ELKS libc. A DOS port though would be very easy.


```
 Usage: {jpg,bmp,ppm}view [source_file]
    source_file: Image file to decode.
```
