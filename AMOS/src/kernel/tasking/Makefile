MAKEFILE=Makefile

INCLUDE_DIR=../../../include/

CC=gcc
RM=del

CCFLAGS=-Wall -O -fstrength-reduce -fomit-frame-pointer -ffreestanding -fno-strict-aliasing -nostdlib -fno-builtin -finline-functions -nostdinc -I $(INCLUDE_DIR)

OBJ=tasking.o
	
all: $(OBJ)

%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<

clean:
	$(RM) *.o