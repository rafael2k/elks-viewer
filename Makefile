# ELKS Viewer - C86 Makefile for native ELKS compilation

INCLUDES=-I/usr/include -I/usr/include/c86
C86LIB=/usr/lib
#BIN=/usr/bin/

DEFINES=

CPP=$(BIN)cpp
CC=$(BIN)c86
AS=$(BIN)as
LD=$(BIN)ld

CPPFLAGS=-0 $(INCLUDES) $(DEFINES)
CFLAGS=-g -O -bas86 -separate=yes -warn=4 -lang=c99 \
    -align=yes -separate=yes -stackopt=minimum -peep=all -stackcheck=no
#ASFLAGS=-0 -j -O -w- -V
ASFLAGS=-0 -j
LDFLAGS=-0 -i -L$(C86LIB)
LDLIBS=-lc86

# Automated rules for C86 toolchain
.c.o:
	$(CPP) $(CPPFLAGS) $*.c -o $*.i
	$(CC) $(CFLAGS) $*.i $*.as
	$(AS) $(ASFLAGS) $*.as -o $*.o

.s.o:
	$(AS) $(ASFLAGS) $*.s -o $*.o

##### End of standardized section #####

#PROGS=ppmview bmpview jpgview - jpeview is not building yet
PROGS=ppmview bmpview

all: $(PROGS)

jpgview: picojpeg.o jpgview.o graphics.o vga-4bp.o utils.o
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

ppmview: ppmview.o graphics.o vga-4bp.o utils.o
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

bmpview: bmpview.o bmputils.o graphics.o vga-4bp.o utils.o
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f *.i *.o *.as $(PROGS)
