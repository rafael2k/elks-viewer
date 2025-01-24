# ELKS viewer

Based on picojpeg for jpeg decoding.

Compiled with the OW compiler.

```
 Usage: eview [source_file] [dest_file] <reduce>
    source_file: JPEG file to decode. Note: Progressive files are not supported.
    dest_file: Output .raw file.
    reduce: Optional, if 1 the JPEG file is quickly decoded to ~1/8th resolution.
    
    Outputs 8-bit grayscale or truecolor 24-bit raw files.

```
