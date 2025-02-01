# Sample commands to convert to BMP or JPG with ImageMagick 7

# BMP 8-bit example
magick racecar.ppm -resize 40% -define bmp:format=bmp4 -type palette -depth 8 -compress none racecar-8b.bmp

# BBP 8-bit with RLE example
magick bars2.ppm -resize 40% -define bmp:format=bmp4 -type palette -depth 8 -compress RLE bars2-8b-rle.bmp

# JPG encoding example
magick racecar.ppm -resize 40% racecar.jpg

# Grayscale JPEG
magick racecar.ppm -resize 40% -colorspace Gray racecar.jpg
