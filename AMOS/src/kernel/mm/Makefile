MAKEFILE=Makefile

INCLUDE_DIR=../../../include/

CC=gcc
RM=del

CCFLAGS=-Wall -O -fstrength-reduce -fomit-frame-pointer -ffreestanding -nostdlib -fno-builtin -finline-functions -nostdinc -I $(INCLUDE_DIR)

OBJ=paging.o segmentation.o physical.o mm.o dma.o
	
all: $(OBJ)

%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<

clean:
	$(RM) *.o