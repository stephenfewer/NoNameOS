MAKEFILE=Makefile

INCLUDE_DIR=../../../include/

CC=gcc
RM=del

CCFLAGS=-Wall -O -fstrength-reduce -fomit-frame-pointer -ffreestanding -nostdlib -fno-builtin -finline-functions -nostdinc -I $(INCLUDE_DIR)

OBJ=scheduler.o process.o elf.o
	
all: $(OBJ)

%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<

clean:
	$(RM) *.o